#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include "23.h"
#include "checker.h"

#define MAX 100010
#define ABORT_IF_FAILED(x) { \
    int ret = (x); \
    if (!ret) { \
        printf("Assert \"%s\" failed! Abort...\n", #x); \
        abort(); \
    } \
}

static pid_t _pid = -1;

static int cnt;
static int numbers[MAX];
static int reportPool[MAX];
static FILE* file;

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

__attribute__((constructor))
static void init_file() {
    file = fopen("./sample-in", "r");
    if (file == NULL) {
        perror("fopen");
        abort();
    }
    ABORT_IF_FAILED(fscanf(file, "%d", &cnt) > 0);
    for (int i = 0; i < cnt; i++) {
        ABORT_IF_FAILED(fscanf(file, "%d", &numbers[i]) > 0);
    }
}

void GetN(int* x) {
    if (status != SHOULD_GET_N) {
        fatal();
    }
    // printf("GET_N\n");
    *x = cnt;
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
    status = (pos < cnt ? SHOULD_GET_A_NUMBER : SHOULD_EXIT);
}

void Bye() {
    // do the total check here (?)
    if (status != SHOULD_EXIT) {
        fatal();
    }
    printf("Final checking...\n");
    int ans = numbers[0];
    int fail = 0;
    for (int i = 1; i < cnt; i++) {
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
    // printf("--- rdi=%lld, rsi=%llx\n", regs.rdi, regs.rsi);
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
