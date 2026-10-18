#define ILIBC_IO_BACKEND_POSIX
#define ILIBC_HOST_IO_TEST
#include <cstdio>
int main(){return ilibc::putstr("")?1:0;}
