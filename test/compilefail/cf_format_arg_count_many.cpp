// cf_format_arg_count_many.cpp — 変換指定より引数が多いとコンパイルエラーに
// なる(TC-IO-03、ILIBC-IO-001 §3)。
// EXPECT-GCC:   ilibc: format conversion count does not match arguments
// EXPECT-CLANG: ilibc: format conversion count does not match arguments
#include <ilibc/format.hpp>

#ifdef CF_FAIL
// 失敗用: %d が1つしかないのに引数が2つある
int main() {
    char buf[16];
    return ilibc::snprint_f<"a%d">(buf, sizeof buf, 1, 2);
}
#else
// 対照(-DCF_CONTROL): 個数が一致すれば成立する
int main() {
    char buf[16];
    return ilibc::snprint_f<"a%d">(buf, sizeof buf, 1);
}
#endif
