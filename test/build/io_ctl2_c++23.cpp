#define ILIBC_ENABLE_PRINTF_MACRO
#define ILIBC_IO_BACKEND_POSIX
#include <cstdlib>
#include <cstdio>
int main(){return printf("")?1:0;}
