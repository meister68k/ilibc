// cf_gets_tiny_array.cpp — gets は要素数 N>=2 の配列を受け付ける。
// N=1 の配列は fgets の size>=2 契約を満たさないため static_assert で
// 拒否される(TC-IO-20、ILIBC-IO-001 §14.1)。
// EXPECT-GCC:   static assertion failed
// EXPECT-CLANG: static assertion failed|static_assert
#define ILIBC_IO_BACKEND_POSIX
#include <cstdio>

#ifdef CF_FAIL
// 失敗用: N=1 は1バイト+NUL の最低要件を満たさない
int main() {
    char buf[1];
    return ilibc::gets(buf) == nullptr;
}
#else
// 対照(-DCF_CONTROL): N=2 なら成立する
int main() {
    char buf[2];
    return ilibc::gets(buf) == nullptr;
}
#endif
