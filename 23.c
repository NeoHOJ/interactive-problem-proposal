#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>
#include "23.h"

static int gcd(int a, int b) {
    return a == 0 ? b : gcd(b % a, a);
}

int main() {
    printf("My pid is %d.\n", getpid());
    puts("Waiting for ptrace attach...");

    pause();
    errno = 0;

    puts("Attached!");

    /* real answers start here */
    int len, nowGCD, nextNumber, firstNumber;
    GetN(&len);
    Get(&firstNumber);
    printf("first=%d\n", firstNumber);
    nowGCD = firstNumber;
    for(int i = 1; i < len; i++) {
        Get(&nextNumber);
        printf("next(%d)=%d\n", i, nextNumber);
        nowGCD = gcd(nowGCD, nextNumber);
        printf("now gcd=%d\n", nowGCD);
        Report(nowGCD);
    }
    Bye();
}
