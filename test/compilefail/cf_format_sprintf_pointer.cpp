// cf_format_sprintf_pointer.cpp — array-safe sprintf に配列への参照ではなく
// ポインタ変数を渡すとコンパイルエラーになる(TC-IO-07、ILIBC-IO-001 §7)。
// 容量が分からないポインタでは不変性の保証ができないため受け付けない。
// EXPECT-GCC:   no matching function
// EXPECT-CLANG: no matching function|no viable
#include <ilibc/format.hpp>

#ifdef CF_FAIL
// 失敗用: char* は char (&)[N] にマッチしない
int main() {
    char buf[16];
    char *p = buf;
    return ilibc::sprintf<"a">(p);
}
#else
// 対照(-DCF_CONTROL): 配列そのものを渡せば成立する
int main() {
    char buf[16];
    return ilibc::sprintf<"a">(buf);
}
#endif
