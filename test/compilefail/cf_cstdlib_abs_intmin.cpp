// cf_cstdlib_abs_intmin.cpp — 定数式で abs(INT_MIN) と labs(LONG_MIN) を
// 評価するとコンパイルエラーになる(TC-SL-02(a)、ILIBC-STL-001 §9、ILIBC-TST-001 §5.4)。
// EXPECT-GCC:   overflow in constant expression
// EXPECT-CLANG: overflow|is not a constant expression
#include <cstdlib>
#include <climits>

#ifdef CF_FAIL
// 失敗用: 結果が表現できない(定数式の符号付きオーバーフローは診断される)
static_assert(std::abs(INT_MIN) == 0);
static_assert(std::labs(LONG_MIN) == 0L);
#else
// 対照(-DCF_CONTROL): 範囲内の入力なら成立すること
static_assert(std::abs(-INT_MAX) == INT_MAX);
static_assert(std::labs(-LONG_MAX) == LONG_MAX);
#endif
