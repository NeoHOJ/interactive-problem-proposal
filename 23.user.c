#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include "23.h"

void GetN(int* x) {
    size_t tmp;
    errno = 0;
    syscall(999, 1, &tmp);
    *x = (int) tmp;
}

void Get(int* x) {
    size_t tmp;
    syscall(999, 2, &tmp);
    *x = (int) tmp;
}

void Report(int x) {
    syscall(999, 3, x);
}

void Bye() {
    syscall(999, 4);
}
