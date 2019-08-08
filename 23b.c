#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>

static int gcd(int a, int b) {
    return a == 0 ? b : gcd(b % a, a);
}

#define MAX 100010
#define ABORT_IF_FAILED(x) { \
    int ret = (x); \
    if (!ret) { \
        printf("Assert \"%s\" failed! Abort...\n", #x); \
        abort(); \
    } \
}

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


static int fatal() {
    return raise(SIGKILL);
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
    printf("GET_N\n");
    *x = cnt;
    status = SHOULD_GET_A_NUMBER;
}

void Get(int* x) {
    if (status != SHOULD_GET_A_NUMBER) {
        fatal();
    }
    printf("GET [%d]: %d\n", pos, numbers[pos]);
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
    printf("REPORT pos=%d, n=%d\n", pos, x);
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

int main() {
    // no longer need these?
    // printf("My pid is %d.\n", getpid());
    // puts("Waiting for ptrace attach...");
    // pause();
    // puts("Attached!");

    /* real answers start here */
    int len, nowGCD, nextNumber, firstNumber;
    GetN(&len);
    Get(&firstNumber);
    // printf("first=%d\n", firstNumber);
    nowGCD = firstNumber;
    for(int i = 1; i < len; i++) {
        Get(&nextNumber);
        // printf("next(%d)=%d\n", i, nextNumber);
        nowGCD = gcd(nowGCD, nextNumber);
        // printf("now gcd=%d\n", nowGCD);
        Report(nowGCD);
    }
    Bye();
}
