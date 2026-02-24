#include "common.h"

/*
 * 05_fork_exec
 *
 * Purpose:
 *   Show that exec() replaces the process image *without changing PID*.
 *
 * Demo steps:
 *   1) Run: ./05_fork_exec
 *   2) In another terminal, run:
 *        ps -eo pid,ppid,state,comm --forest | head -n 40
 *   3) Optionally trace:
 *        strace -f -e trace=process,signal ./05_fork_exec
 */
int main(int argc, char **argv) {
    (void)argc; (void)argv;

    print_ids("parent: start");
    pid_t pid = fork();
    if (pid < 0) die_errno("fork");

    if (pid == 0) {
        print_ids("child: pre-exec");
        info("[child] About to exec 'ls -l'. PID will remain %d", (int)getpid());
        char *const args[] = { (char*)"ls", (char*)"-l", NULL };
        execvp(args[0], args);
        die_errno("execvp"); // only runs on failure
    } else {
        print_ids("parent");
        info("[parent] spawned child pid=%d; waiting...", (int)pid);
        int status = 0;
        if (waitpid(pid, &status, 0) < 0) die_errno("waitpid");

        if (WIFEXITED(status)) info("[parent] child exit=%d", WEXITSTATUS(status));
        else if (WIFSIGNALED(status)) info("[parent] child signal=%d", WTERMSIG(status));
        else info("[parent] child ended (status=0x%x)", status);

        info("[parent] done");
    }
    return 0;
}
