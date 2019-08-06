#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include "23.h"
#include "checker.h"

#define MAX 6

static pid_t _pid = -1;

static int numbers[] = {
    4034940,
    6859398,
    3074240,
    3237559,
    3045419,
    7464639,
};

static int reportPool[MAX] = {};

static enum {
    SHOULD_GET_N = 0,
    SHOULD_GET_A_NUMBER,
    SHOULD_REPORT,
    SHOULD_EXIT,
} status = 0;
static int pos = 0;

static int gcd(int a, int b) {
    return a == 0 ? b : gcd(b % a, a);
}

static int fatal() {
    return kill(_pid, SIGKILL);
}

void GetN(int* x) {
    if (status != SHOULD_GET_N) {
        fatal();
    }
    *x = MAX;
    status = SHOULD_GET_A_NUMBER;
}

void Get(int* x) {
    if (status != SHOULD_GET_A_NUMBER) {
        fatal();
    }
    printf("GET(int)!! [%d]: %d\n", pos, numbers[pos]);
    *x = numbers[pos];
    if (pos == 0) {
        pos++;
    } else {
        status = SHOULD_REPORT;
    }
}

void Report(int x) {
    if (status != SHOULD_REPORT) {
        fatal();
    }
    printf("Report pos=%d, n=%d\n", pos, x);
    reportPool[pos] = x;
    pos++;
    status = (pos < MAX ? SHOULD_GET_A_NUMBER : SHOULD_EXIT);
}

void Bye() {
    // do the total check here (?)
    if (status != SHOULD_EXIT) {
        fatal();
    }
    printf("Final checking...\n");
    int ans = numbers[0];
    int fail = 0;
    for (int i = 1; i < MAX; i++) {
        // compute with next number...
        ans = gcd(ans, numbers[i]);
        // ... and compare against the reported one
        if (reportPool[i] != ans) {
            printf("WA at case %d: reportPool[i]=%d, ans=%d\n", i, reportPool[i], ans);
            fail = 1;
        }
    }
    if (fail) {
        fatal();
    } else {
        printf("AC!\n");
        // AC!
    }
}


void __callback(pid_t pid, struct user_regs_struct regs) {
    _pid = pid;
    int tmp;
    printf("--- rdi=%lld, rsi=%llx\n", regs.rdi, regs.rsi);
    switch (regs.rdi) {
    case 1:
        GetN(&tmp);
        ptrace(PTRACE_POKEDATA, pid, regs.rsi, (size_t) tmp);
        break;
    case 2:
        Get(&tmp);
        ptrace(PTRACE_POKEDATA, pid, regs.rsi, (size_t) tmp);
        break;
    case 3:
        Report(regs.rsi);
        break;
    case 4:
        Bye();
        break;
    default:
        regs.orig_rax = -1;
    }
}
