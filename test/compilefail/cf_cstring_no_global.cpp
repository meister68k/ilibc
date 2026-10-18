// cf_cstring_no_global.cpp — <cstring> はグローバル名前空間に名前を追加しないが、
// <string.h> は追加する(TC-ST-16(a)、ILIBC-STR-001 §9、ILIBC-TST-001 §5.4)。
// EXPECT-GCC:   has not been declared|was not declared|not declared in this scope
// EXPECT-CLANG: use of undeclared identifier|not declared
#include <cstring>

#ifdef CF_FAIL
// 失敗用: グローバルの strlen は見つからないはず
int main() { return ::strlen("abc") != 3 ? 0 : 1; }
#else
// 対照(-DCF_CONTROL): <cstring> 単体で試験対象名前空間の strlen が使えること
// (フリースタンディングのビルドでは ILIBC_NS は std なので "std::strlen が使える"
// と同値。T1 に従い ILIBC_NS 別名で指す)
namespace t = ILIBC_NS;
int main() { return t::strlen("abc") == 3 ? 0 : 1; }
#endif
