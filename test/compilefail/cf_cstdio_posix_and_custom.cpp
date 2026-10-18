// cf_cstdio_posix_and_custom.cpp — ILIBC_IO_BACKEND_POSIX と
// ILIBC_IO_BACKEND_CUSTOM の同時指定は矛盾するためコンパイルエラーになる
// (TC-IO-16、ILIBC-IO-001 §5.1)。両方指定しても POSIX が勝つことはない。
// EXPECT-GCC:   POSIX and ILIBC_IO_BACKEND_CUSTOM are mutually exclusive
// EXPECT-CLANG: POSIX and ILIBC_IO_BACKEND_CUSTOM are mutually exclusive
#define ILIBC_IO_BACKEND_POSIX
#ifdef CF_FAIL
#  define ILIBC_IO_BACKEND_CUSTOM
#endif
#include <cstdio>
int main() { return ilibc::putstr("") ? 1 : 0; }
