// cf_cstdlib_no_global_atoi.cpp — <cstdlib> のみを含む翻訳単位で、グローバル
// の ::atoi が見つからない(TC-SL-16(a)、ILIBC-STL-001 §9、ILIBC-HDR-001 §6 W4)。
// EXPECT-GCC:   has not been declared
// EXPECT-CLANG: has not been declared|use of undeclared identifier
#include <cstdlib>

#ifdef CF_FAIL
// 失敗用: <cstdlib> はグローバル名前空間に何も追加しない
int main() { return ::atoi("1"); }
#else
// 対照(-DCF_CONTROL): std:: 形なら使える
int main() { return std::atoi("1"); }
#endif
