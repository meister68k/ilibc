// cf_cstdlib_no_rand.cpp — std::rand が存在しない(提供しない。ILIBC-STL-001
// §1、§7 差異6、ILIBC-OVR-001 第7章。TC-SL-16(d))。
// EXPECT-GCC:   is not a member of
// EXPECT-CLANG: no member named
#include <cstdlib>

#ifdef CF_FAIL
// 失敗用: 乱数は提供しない
int main() { return std::rand(); }
#else
// 対照(-DCF_CONTROL): 提供する名前(std::abs)なら使える
int main() { return std::abs(-1); }
#endif
