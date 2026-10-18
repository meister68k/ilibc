// cf_format_arg_type_s.cpp — %s に文字列ポインタへ変換できない型(int 等)を
// 渡すとコンパイルエラーになる(TC-IO-03、ILIBC-IO-001 §3)。
// EXPECT-GCC:   %s needs const char
// EXPECT-CLANG: %s needs const char
#include <ilibc/format.hpp>

#ifdef CF_FAIL
// 失敗用: int は const char* に変換できない
int main() {
    char buf[16];
    return ilibc::snprint_f<"a%s">(buf, sizeof buf, 1);
}
#else
// 対照(-DCF_CONTROL): 文字列リテラルなら成立する
int main() {
    char buf[16];
    return ilibc::snprint_f<"a%s">(buf, sizeof buf, "s");
}
#endif
