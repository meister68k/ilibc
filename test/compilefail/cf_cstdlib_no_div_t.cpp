// cf_cstdlib_no_div_t.cpp — std::div_t が存在しない(提供しない。ILIBC-STL-001
// §2.4、§7 差異6。TC-SL-16(d))。
// EXPECT-GCC:   is not a member of
// EXPECT-CLANG: no member named
#include <cstdlib>

#ifdef CF_FAIL
// 失敗用: div/ldiv とその型は提供しない
int main() { std::div_t d; (void)d; return 0; }
#else
// 対照(-DCF_CONTROL): 提供する名前(std::labs)なら使える
int main() { return static_cast<int>(std::labs(-1L)); }
#endif
