#include "common.h"

/*
 * 01_proc_identity
 *
 * Purpose:
 *   Show that a process has kernel-exposed metadata via /proc/<pid>/status.
 *
 * Demo steps:
 *   1) Run: ./01_proc_identity
 *   2) In another terminal:
 *        cat /proc/<pid>/status | head -n 40
 *        ls -l /proc/<pid>/fd
 *   3) Observe: PID, PPID, State, Uid/Gid, Threads, open fds, etc.
 */
int main(void) {
    print_ids("start");
    info("Open another terminal and run:");
    info("  cat /proc/%d/status | head -n 40", (int)getpid());
    info("  ls -l /proc/%d/fd", (int)getpid());
    info("Keeping process alive for 30 seconds...");
    for (int i = 30; i >= 1; --i) {
        info("  exiting in %d...", i);
        msleep(1000);
    }
    info("done");
    return 0;
}
