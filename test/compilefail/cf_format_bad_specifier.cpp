// cf_format_bad_specifier.cpp — 書式に %x や修飾子付き %.2d が含まれると
// コンパイルエラーになる(TC-IO-03、ILIBC-IO-001 §3)。
// 対応は %d/%s/%% のみで、フラグ・幅・精度の修飾子は受け付けない。
// EXPECT-GCC:   ilibc: invalid format token
// EXPECT-CLANG: ilibc: invalid format token
#include <ilibc/format.hpp>

#ifdef CF_FAIL
// 失敗用: %x と %.2d はどちらも未知トークン
int main() {
    char buf[16];
    return ilibc::snprint_f<"a%x">(buf, sizeof buf, 1);
}
#else
// 対照(-DCF_CONTROL): %d は受理される
int main() {
    char buf[16];
    return ilibc::snprint_f<"a%d">(buf, sizeof buf, 1);
}
#endif
