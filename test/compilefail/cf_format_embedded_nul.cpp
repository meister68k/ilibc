// cf_format_embedded_nul.cpp — 書式リテラルの途中に NUL バイトが含まれると
// コンパイルエラーになる(TC-IO-03、ILIBC-IO-001 §3)。
// EXPECT-GCC:   ilibc: format string contains an embedded NUL
// EXPECT-CLANG: ilibc: format string contains an embedded NUL
#include <ilibc/format.hpp>

#ifdef CF_FAIL
// 失敗用: "a\0b" は長さ4の配列になり、終端より先に NUL が現れる
int main() {
    char buf[16];
    return ilibc::snprint_f<"a\0b">(buf, sizeof buf);
}
#else
// 対照(-DCF_CONTROL): 埋め込み NUL がなければ成立する
int main() {
    char buf[16];
    return ilibc::snprint_f<"ab">(buf, sizeof buf);
}
#endif
