// cf_cassert_constexpr_false.cpp — 定数評価中に、偽の assert を含む関数を
// 呼ぶと、非 constexpr の失敗フック __ilibc_assert_fail の呼び出し経路が
// 入るためコンパイルエラーになる(TC-HD-06(d)、ILIBC-AST-001 §3.4、
// ILIBC-HDR-001 §4.1)。
// EXPECT-GCC:   not a constant expression|non-constant|constexpr
// EXPECT-CLANG: not a constant expression|constant expression|constexpr
#include <cassert>

// NDEBUG 未定義のときにのみ偽の経路が定数評価に入る(-DNDEBUG を指定しない)
#ifdef CF_FAIL
constexpr int cf_checked(int n) {
    assert(n >= 0);
    return n;
}
// 失敗用: 偽の assert はフックを呼ぶため、定数式として成立しない
static_assert(cf_checked(-1) == -1);
#else
constexpr int cf_checked(int n) {
    assert(n >= 0);
    return n;
}
// 対照(-DCF_CONTROL): 真の assert は定数式として評価できる
static_assert(cf_checked(1) == 1);
#endif
