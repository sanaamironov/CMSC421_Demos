#include "common.h"

/*
 * 04_fork_cow
 *
 * Purpose:
 *   Demonstrate process memory isolation and the idea of copy-on-write:
 *   parent and child start with the "same" memory, but writing causes divergence.
 *
 * Demo steps:
 *   1) Run: ./04_fork_cow
 *   2) Observe: parent and child see different values after child's write.
 */
static int g_counter = 100;

int main(void) {
    print_ids("start");
    info("Initial g_counter=%d @ %p", g_counter, (void*)&g_counter);

    pid_t pid = fork();
    if (pid < 0) die_errno("fork");

    if (pid == 0) {
        // Child modifies global
        print_ids("child");
        info("[child] before write: g_counter=%d", g_counter);
        g_counter = 999;
        info("[child] after  write: g_counter=%d", g_counter);
        info("[child] sleeping 3 seconds; inspect with ps if desired...");
        msleep(3000);
        _exit(0);
    } else {
        // Parent waits a moment, then prints its own view
        print_ids("parent");
        info("[parent] sleeping 1 second to let child write...");
        msleep(1000);
        info("[parent] after child's write (parent view): g_counter=%d", g_counter);

        int status = 0;
        if (waitpid(pid, &status, 0) < 0) die_errno("waitpid");
        info("[parent] child collected; done");
    }
    return 0;
}
