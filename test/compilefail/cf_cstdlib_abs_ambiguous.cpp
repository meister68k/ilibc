// cf_cstdlib_abs_ambiguous.cpp — abs(1LL)、abs(1U)、abs(1.0) が int 版と
// long 版の間で曖昧になり、コンパイルエラーになる(TC-SL-02(b)、ILIBC-STL-001 §9)。
// EXPECT-GCC:   ambiguous
// EXPECT-CLANG: ambiguous
#include <cstdlib>

#ifdef CF_FAIL
// 失敗用: long long / unsigned / double は int にも long にも変換でき、曖昧
static_assert(std::abs(1LL) == 1);
static_assert(std::abs(1U) == 1U);
static_assert(std::abs(1.0) == 1.0);
#else
// 対照(-DCF_CONTROL): int と long のリテラルは明確に選ばれる
static_assert(std::abs(1) == 1);
static_assert(std::abs(1L) == 1L);
#endif
