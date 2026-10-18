// cf_cstdlib_strtol_range.cpp — 定数式で、std::strtol / strtoul / atoi /
// atol に範囲外の入力を与えると、errno(g_errno)への書き込みによりコンパイル
// エラーになる(TC-SL-05、ILIBC-STL-001 §3.4、§9)。
// EXPECT-GCC:   not a constant expression|non-constant condition
// EXPECT-CLANG: not a constant expression|is not a constant expression
#include <cstdlib>
#include <climits>

#ifdef CF_FAIL
// 失敗用: 範囲外の経路は errno に書くため、定数式にならない
static_assert(std::strtol("2147483648", nullptr, 10) == LONG_MAX);
static_assert(std::strtoul("4294967296", nullptr, 10) == 4294967295UL);
static_assert(std::atoi("2147483648") == INT_MAX);
static_assert(std::atol("-2147483649") == LONG_MIN);
#else
// 対照(-DCF_CONTROL): 範囲内の入力は定数式として評価できる
static_assert(std::strtol("2147483647", nullptr, 10) == LONG_MAX);
static_assert(std::strtoul("4294967295", nullptr, 10) == 4294967295UL);
static_assert(std::atoi("2147483647") == INT_MAX);
static_assert(std::atol("-2147483648") == LONG_MIN);
#endif
