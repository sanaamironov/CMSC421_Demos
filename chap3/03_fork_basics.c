#include "common.h"

/*
 * 03_fork_basics
 *
 * Purpose:
 *   Show the two return paths of fork(): parent receives child's PID, child receives 0.
 *
 * Demo steps:
 *   1) Run: ./03_fork_basics
 *   2) Observe output interleaving (nondeterministic ordering).
 *   3) Use: ps -eo pid,ppid,state,comm --forest | head -n 40
 */
int main(void) {
    print_ids("before fork");

    pid_t pid = fork();
    if (pid < 0) die_errno("fork");

    if (pid == 0) {
        // Child
        print_ids("child");
        info("[child] fork() returned %d (expected 0)", (int)pid);
        info("[child] sleeping 2 seconds...");
        msleep(2000);
        info("[child] exiting");
        _exit(0);
    } else {
        // Parent
        print_ids("parent");
        info("[parent] fork() returned %d (child pid)", (int)pid);
        int status = 0;
        pid_t w = waitpid(pid, &status, 0);
        if (w < 0) die_errno("waitpid");
        if (WIFEXITED(status)) {
            info("[parent] child exited with status=%d", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            info("[parent] child killed by signal=%d", WTERMSIG(status));
        } else {
            info("[parent] child ended (status=0x%x)", status);
        }
        info("[parent] done");
    }
    return 0;
}
