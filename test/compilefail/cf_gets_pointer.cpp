// cf_gets_pointer.cpp — gets は配列参照 char(&)[N] のみを受け付け、
// char* ポインタ版は意図的に無い。バッファ長がコンパイル時に分からない
// ためコンパイルエラーになる(TC-IO-20、ILIBC-IO-001 §14.1)。
// EXPECT-GCC:   no matching function
// EXPECT-CLANG: no matching function|no viable
#define ILIBC_IO_BACKEND_POSIX
#include <cstdio>

#ifdef CF_FAIL
// 失敗用: char* は配列参照にマッチせず、ポインタ版オーバーロードも無い
int main() {
    char buf[8];
    char *p = buf;
    return ilibc::gets(p) == nullptr;
}
#else
// 対照(-DCF_CONTROL): 配列そのものなら容量が推定され成立する
int main() {
    char buf[8];
    return ilibc::gets(buf) == nullptr;
}
#endif
