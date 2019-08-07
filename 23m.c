#include <stdio.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "23.h"
#include "checker.h"


int main() {
    pid_t pid;
    printf("pid=");
    scanf("%d", &pid);

    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0) {
        printf("Failed to attach ptrace\n");
        return -1;
    }

    if (waitpid(pid, NULL, WUNTRACED) != pid) {
        perror("waitpid(WUNTRACED)");
        return -1;
    }

    printf("Attached to pid %d\n", pid);

    int ptopts = (
        // our main purpose
        PTRACE_O_TRACESECCOMP |
        PTRACE_O_TRACESYSGOOD |
        // also notify if the tracee exits early
        PTRACE_O_TRACEEXIT |
        // don't detach if the tracer dies
        PTRACE_O_EXITKILL);

    if (ptrace(PTRACE_SETOPTIONS, pid, NULL, ptopts) < 0) {
        perror("set ptrace options");
    }

    puts("Ready");
    // PTRACE_ATTACH delivers a SIGSTOP to stop the tracee
    // so continue it when we are ready
    // if using PTRACE_SEIZE, this step is omitted
    // kill(pid, SIGCONT);

    int ws;
    // the first PTRACE_CONT -> waitpid must be (ws>>8 == SIGCONT)
    int first = 1;
    int signo = 0;

    while (1) {
        puts(">>> Enter trace loop");
        int res;
        if ((res = ptrace(PTRACE_CONT, pid, 0, signo)) < 0) {
            perror("ptrace CONT");
            return -1;
        }

        if (waitpid(pid, &ws, 0) != pid) {
            perror("waitpid");
        }

        printf("wait return ws=%d\n", ws);

        // handle signals
        if (WIFSTOPPED(ws)) {
            signo = WSTOPSIG(ws);
            // if it's SIGTRAP, then cancel it
            if (signo == SIGTRAP) {
                signo = 0;
            } else {
                printf("Get signal %d\n", signo);
            }
        }

        if (ws>>8 == (SIGTRAP | (PTRACE_EVENT_SECCOMP<<8))) {
            // it's a SECCOMP trace
            // printf("TRACED!\n");

            // Refer to "PTRACE_EVENT_SECCOMP stops (since Linux 4.8)" section @ man ptrace(2)
            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) < 0) {
                printf("Fail to get registers.\n");
                ptrace(PTRACE_DETACH, pid, NULL, NULL);
                return -1;
            }
            printf("Syscall=%lld\n", regs.orig_rax);

            unsigned long msg;
            if (ptrace(PTRACE_GETEVENTMSG, pid, NULL, &msg) < 0) {
                printf("Failed to get event msg\n");
                ptrace(PTRACE_DETACH, pid, NULL, NULL);
                return -1;
            }
            printf("seccomp data=%zd\n", msg);


            int new_syscall = -1;
            // printf("Change syscall %lld -> ? ", regs.orig_rax);
            // scanf("%d", &new_syscall);
            __callback(pid, regs);
            regs.orig_rax = new_syscall;
            regs.rax = 0;
            ptrace(PTRACE_SETREGS, pid, NULL, &regs);
        } else if (ws>>8 == (SIGTRAP | (PTRACE_EVENT_EXIT<<8))) {
            int msg;
            ptrace(PTRACE_GETEVENTMSG, pid, NULL, &msg);
            printf("Child exits with status %d\n", msg);
            // if we break without detaching, the child is still counted
            // as exit early
            ptrace(PTRACE_DETACH, pid, NULL, NULL);
            break;
        } else if (WIFSIGNALED(ws) && WTERMSIG(ws) == SIGKILL) {
            // FIXME: should I check the signal like this?
            printf("Child is dead QAQ!\n");
            ptrace(PTRACE_DETACH, pid, NULL, NULL);
            break;
        } else if (first) {
            first = 0;
            // This is because we use pause() to stop at a certain point

            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) < 0) {
                return -1;
            }

            if (regs.orig_rax != 34) {
                printf("The first syscall is %lld instead of pause()\n", regs.orig_rax);
                continue;
                // return -1;
            }

            printf("Skip pause()\n");

            regs.orig_rax = -1; // set to invalid syscall
            // To understand why setting rax here:
            // https://stackoverflow.com/questions/37167141/linux-syscalls-and-errno
            regs.rax = -EINTR;
            ptrace(PTRACE_SETREGS, pid, NULL, &regs);
        } else {
            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) < 0) {
                return -1;
            }

            printf("???: syscall id=%lld\n", regs.orig_rax);
            // return -1;
        }
    }
}
