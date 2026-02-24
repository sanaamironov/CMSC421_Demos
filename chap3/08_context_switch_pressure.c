
#include "common.h"
#include <fcntl.h>
#include <sys/resource.h>

static volatile uint64_t sink = 0;

static volatile sig_atomic_t stop_flag = 0;

static void on_sigint(int sig) {
    (void)sig;
    stop_flag = 1;
}

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [--cpu N] [--io N] [--seconds T] [--nice N]\n"
        "\n"
        "Creates CPU-bound and I/O-ish child processes so you can *observe* scheduling,\n"
        "time slicing, and context switching with Linux tools.\n"
        "\n"
        "Options:\n"
        "  --cpu N       number of CPU-bound workers (default: 2)\n"
        "  --io N        number of I/O-ish workers (default: 0)\n"
        "  --seconds T   run duration in seconds (default: 30)\n"
        "  --nice N      nice value applied to all children (default: 0)\n"
        "\n"
        "Suggested observation commands (run in another terminal):\n"
        "  top           (watch CPU share)\n"
        "  htop          (process list + per-core view)\n"
        "  uptime        (watch load averages)\n"
        "  vmstat 1      (watch context switches 'cs' and run queue 'r')\n"
        "  pidstat -w 1  (context switches per PID; may require sysstat)\n"
        "\n"
        "Example:\n"
        "  %s --cpu 6 --io 2 --seconds 40\n",
        prog, prog
    );
}

static int parse_int(const char *s, const char *what) {
    if (!s || !*s) die("Missing value for %s", what);
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (!end || *end != '\0') die("Invalid integer for %s: '%s'", what, s);
    if (v < 0 || v > 1000000) die("Out-of-range value for %s: %ld", what, v);
    return (int)v;
}

static void cpu_worker(void) {
    print_ids("cpu");
    info("[cpu] Tip: watch me in 'top' (%%CPU) and 'pidstat -w 1' (cswch/nvcswch).");
    // Busy loop with enough work to show on top without being optimized away.
    while (!stop_flag) {
        sink ^= (uint64_t)time(NULL);
        for (uint64_t i = 0; i < 2500000ULL; i++) {
            sink = (sink * 2862933555777941757ULL) + 3037000493ULL;
        }
        // Small pause so the terminal stays responsive if you oversubscribe.
        // (Still CPU-bound overall.)
        msleep(1);
    }
    _exit(0);
}

static void io_worker(void) {
    print_ids("io");
    info("[io] This is I/O-ish: periodic small writes + sleeps (not a real disk benchmark).");

    int fd = open("/dev/null", O_WRONLY);
    if (fd < 0) die_errno("open(/dev/null)");

    char buf[4096];
    memset(buf, 'A', sizeof(buf));

    while (!stop_flag) {
        // Small write to generate syscalls and keep the process active intermittently.
        ssize_t w = write(fd, buf, sizeof(buf));
        if (w < 0 && errno != EINTR) die_errno("write(/dev/null)");
        msleep(10);
    }

    close(fd);
    _exit(0);
}

int main(int argc, char **argv) {
    int cpu_n = 2;
    int io_n = 0;
    int seconds = 30;
    int nice_val = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--cpu") == 0 && i + 1 < argc) {
            cpu_n = parse_int(argv[++i], "--cpu");
        } else if (strcmp(argv[i], "--io") == 0 && i + 1 < argc) {
            io_n = parse_int(argv[++i], "--io");
        } else if (strcmp(argv[i], "--seconds") == 0 && i + 1 < argc) {
            seconds = parse_int(argv[++i], "--seconds");
        } else if (strcmp(argv[i], "--nice") == 0 && i + 1 < argc) {
            nice_val = (int)strtol(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        } else {
            usage(argv[0]);
            die("Unknown/invalid argument: %s", argv[i]);
        }
    }

    if (cpu_n == 0 && io_n == 0) die("Nothing to run: set --cpu and/or --io to > 0");

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0) die_errno("sigaction(SIGINT)");
    if (sigaction(SIGTERM, &sa, NULL) < 0) die_errno("sigaction(SIGTERM)");

    int total = cpu_n + io_n;
    pid_t *kids = calloc((size_t)total, sizeof(pid_t));
    if (!kids) die_errno("calloc");

    info("=== 08_context_switch_pressure ===");
    print_ids("parent");
    info("Config: cpu=%d io=%d seconds=%d nice=%d", cpu_n, io_n, seconds, nice_val);
    info("Run observation tools now: top | htop | uptime | vmstat 1 | pidstat -w 1");
    info("Press Ctrl+C to stop early.");

    int k = 0;

    for (int j = 0; j < cpu_n; j++) {
        pid_t pid = fork();
        if (pid < 0) die_errno("fork");
        if (pid == 0) {
            if (nice_val != 0) {
                errno = 0;
                int r = nice(nice_val);
                if (r == -1 && errno != 0) warn("[cpu] nice(%d) failed: %s", nice_val, strerror(errno));
            }
            cpu_worker();
        }
        kids[k++] = pid;
    }

    for (int j = 0; j < io_n; j++) {
        pid_t pid = fork();
        if (pid < 0) die_errno("fork");
        if (pid == 0) {
            if (nice_val != 0) {
                errno = 0;
                int r = nice(nice_val);
                if (r == -1 && errno != 0) warn("[io] nice(%d) failed: %s", nice_val, strerror(errno));
            }
            io_worker();
        }
        kids[k++] = pid;
    }

    info("Children started. PIDs:");
    for (int i = 0; i < total; i++) info("  %d", kids[i]);

    for (int t = 0; t < seconds && !stop_flag; t++) {
        // Print load averages (best-effort).
        double la[3] = {0.0, 0.0, 0.0};
        int n = getloadavg(la, 3);
        if (n == 3) {
            info("[t=%2ds] loadavg: %.2f %.2f %.2f", t + 1, la[0], la[1], la[2]);
        } else {
            info("[t=%2ds] running...", t + 1);
        }
        msleep(1000);
    }

    stop_flag = 1;
    info("Stopping children...");

    for (int i = 0; i < total; i++) {
        if (kids[i] > 0) kill(kids[i], SIGTERM);
    }

    // Reap all children.
    int status = 0;
    while (wait(&status) > 0) { }

    free(kids);
    info("Done.");
    return 0;
}
