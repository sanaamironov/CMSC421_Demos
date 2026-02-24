#include "common.h"

/*
 * 07_signals
 *
 * Purpose:
 *   Demonstrate:
 *     - catching SIGINT (Ctrl+C) and SIGTERM
 *     - SIGKILL cannot be caught
 *
 * Demo steps:
 *   1) Run: ./07_signals
 *   2) Press Ctrl+C (SIGINT) and see handler output.
 *   3) In another terminal:
 *        kill -TERM <pid>   (graceful)
 *        kill -KILL <pid>   (immediate, cannot be caught)
 */
static volatile sig_atomic_t g_got_term = 0;

static void on_sigint(int sig) {
    (void)sig;
    info("[handler] caught SIGINT (Ctrl+C). Not exiting; try again or send SIGTERM.");
}

static void on_sigterm(int sig) {
    (void)sig;
    g_got_term = 1;
}

static void install_handler(int signo, void (*fn)(int)) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = fn;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(signo, &sa, NULL) < 0) die_errno("sigaction");
}

int main(void) {
    print_ids("start");
    info("Send signals from another terminal:");
    info("  kill -TERM %d", (int)getpid());
    info("  kill -KILL %d   (cannot be caught)", (int)getpid());
    info("Or press Ctrl+C here (SIGINT).");

    install_handler(SIGINT, on_sigint);
    install_handler(SIGTERM, on_sigterm);

    int ticks = 0;
    while (!g_got_term) {
        info("[loop] alive (tick=%d) pid=%d", ticks++, (int)getpid());
        msleep(1000);
    }

    info("[main] received SIGTERM; cleaning up and exiting gracefully");
    return 0;
}
