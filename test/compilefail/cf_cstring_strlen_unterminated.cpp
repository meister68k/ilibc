// cf_cstring_strlen_unterminated.cpp — 定数式で、終端のない配列に strlen を
// 呼ぶとコンパイルエラーになる(TC-ST-04(a)、ILIBC-STR-001 §9、ILIBC-TST-001 §5.4)。
// EXPECT-GCC:   outside the bounds of array|outside array bounds
// EXPECT-CLANG: index .* out of bounds|out of bounds|not evaluated
#include <cstring>

namespace t = ILIBC_NS;

#ifdef CF_FAIL
// 失敗用: 終端 '\0' のない char[3] を読む(定数式の未定義動作は診断される)
constexpr char kNoTerm[3] = {'a', 'b', 'c'};
static_assert(t::strlen(kNoTerm) == 3);
#else
// 対照(-DCF_CONTROL): 正しく終端された配列なら成立すること
constexpr char kTerm[4] = {'a', 'b', 'c', '\0'};
static_assert(t::strlen(kTerm) == 3);
#endif
