// cf_cstring_strcpy_small.cpp — 定数式で、小さい配列に strcpy を
// 呼ぶとコンパイルエラーになる(TC-ST-04(b)、ILIBC-STR-001 §9、ILIBC-TST-001 §5.4)。
// EXPECT-GCC:   outside the bounds of array|outside array bounds|outside current evaluation
// EXPECT-CLANG: index .* out of bounds|out of bounds|not evaluated
#include <cstring>

namespace t = ILIBC_NS;

#ifdef CF_FAIL
// 失敗用: 書き込み先が char[2] しかない(定数式の未定義動作は診断される)
constexpr bool cf_probe() {
    char dst[2] = {};
    t::strcpy(dst, "abc");
    return dst[0] == 'a';
}
static_assert(cf_probe());
#else
// 対照(-DCF_CONTROL): 十分な大きさの配列なら成立すること
constexpr bool cf_probe() {
    char dst[4] = {};
    t::strcpy(dst, "abc");
    return dst[3] == '\0';
}
static_assert(cf_probe());
#endif
