#define ILIBC_STUB_PASSTHROUGH
#include <cstdio>
#include <cmath>
#include <ctime>
#include <csignal>
#include <clocale>
#include <csetjmp>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <locale.h>
#include <setjmp.h>
int main() {
    std::printf("stub passthrough\n");
    double d = std::sqrt(4.0);
    time_t t = std::time(nullptr);
    void (*prev)(int) = std::signal(SIGINT, SIG_DFL);
    char *lp = std::setlocale(LC_ALL, "");
    jmp_buf jb;
    (void)d; (void)t; (void)prev; (void)lp; (void)jb;
    return 0;
}
