#include "common.h"

/*
 * 06_zombie_demo
 *
 * Purpose:
 *   Demonstrate zombie processes when parent does not wait().
 *
 * Usage:
 *   ./06_zombie_demo --no-wait   (create zombie for ~25s)
 *   ./06_zombie_demo --wait      (properly reap child)
 *
 * Demo steps (no-wait):
 *   1) Run: ./06_zombie_demo --no-wait
 *   2) In another terminal during the parent's sleep:
 *        ps -eo pid,ppid,state,comm | egrep "PID|06_zombie_demo"
 *        ps aux | grep defunct
 *   3) Explain: child is dead, but parent hasn't collected exit status.
 */
static bool has_flag(int argc, char **argv, const char *flag) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], flag) == 0) return true;
    }
    return false;
}

int main(int argc, char **argv) {
    bool do_wait = has_flag(argc, argv, "--wait");
    bool no_wait = has_flag(argc, argv, "--no-wait");
    if (!do_wait && !no_wait) {
        info("Usage: %s --no-wait | --wait", argv[0]);
        return 2;
    }

    print_ids("parent: start");
    pid_t pid = fork();
    if (pid < 0) die_errno("fork");

    if (pid == 0) {
        print_ids("child");
        info("[child] exiting immediately");
        _exit(0);
    }

    // Parent
    print_ids("parent");
    info("[parent] child pid=%d", (int)pid);

    if (do_wait) {
        info("[parent] calling waitpid() (no zombie expected)...");
        int status = 0;
        if (waitpid(pid, &status, 0) < 0) die_errno("waitpid");
        info("[parent] reaped child; sleeping 5 seconds for observation...");
        msleep(5000);
    } else {
        info("[parent] NOT calling waitpid() (zombie expected)...");
        info("[parent] sleep 25 seconds; inspect with ps in another terminal:");
        info("  ps -eo pid,ppid,state,comm | egrep \"PID|06_zombie_demo\"");
        for (int i = 25; i >= 1; --i) {
            if (i % 5 == 0) info("[parent] %d seconds remaining...", i);
            msleep(1000);
        }
        info("[parent] exiting now; init/systemd will finally reap the zombie");
    }
    return 0;
}
