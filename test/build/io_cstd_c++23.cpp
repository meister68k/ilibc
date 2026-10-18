#define ILIBC_ENABLE_PRINTF_MACRO
#define ILIBC_IO_BACKEND_POSIX
#include <cstdio>
int main() {
    int a = printf("ok\n");
    int b = printf("v=%d", 7);
    int c = ilibc::putstr("");
    return (a == 3 && b == 3 && c == 0) ? 0 : 1;
}
