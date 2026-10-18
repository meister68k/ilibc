// cf_cstring_strcmp_null.cpp — 定数式で、nullptr に strcmp を
// 呼ぶとコンパイルエラーになる(TC-ST-04(c)、ILIBC-STR-001 §9、ILIBC-TST-001 §5.4)。
// EXPECT-GCC:   dereferencing a null pointer|null pointer
// EXPECT-CLANG: null pointer|read through
#include <cstring>

namespace t = ILIBC_NS;

#ifdef CF_FAIL
// 失敗用: ヌルポインタを読む(定数式の未定義動作は診断される)
static_assert(t::strcmp(nullptr, "a") == 0);
#else
// 対照(-DCF_CONTROL): 有効な文字列どうしなら成立すること
static_assert(t::strcmp("a", "a") == 0);
#endif
