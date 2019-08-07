#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include "23.h"

#define TO_TRACER(...) { \
    int ret = syscall(999, ##__VA_ARGS__); \
    if (ret < 0) { \
        fprintf(stderr, "Calling to tracer (" #__VA_ARGS__ ") fails with ret=%d: %s\n", ret, strerror(errno)); \
        abort(); \
    } \
}

void GetN(int* x) {
    size_t tmp;
    TO_TRACER(1, &tmp);
    *x = (int) tmp;
}

void Get(int* x) {
    size_t tmp;
    TO_TRACER(2, &tmp);
    *x = (int) tmp;
}

void Report(int x) {
    TO_TRACER(3, x);
}

void Bye() {
    TO_TRACER(4);
}
