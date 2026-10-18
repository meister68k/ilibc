// cf_format_arg_count.cpp — 変換指定(%d)より引数が少ないとコンパイルエラーに
// なる(TC-IO-03、ILIBC-IO-001 §3)。
// EXPECT-GCC:   ilibc: format conversion count does not match arguments
// EXPECT-CLANG: ilibc: format conversion count does not match arguments
#include <ilibc/format.hpp>

#ifdef CF_FAIL
// 失敗用: %d が1つあるのに引数がない
int main() {
    char buf[16];
    return ilibc::snprint_f<"a%d">(buf, sizeof buf);
}
#else
// 対照(-DCF_CONTROL): 個数が一致すれば成立する
int main() {
    char buf[16];
    return ilibc::snprint_f<"a%d">(buf, sizeof buf, 1);
}
#endif
