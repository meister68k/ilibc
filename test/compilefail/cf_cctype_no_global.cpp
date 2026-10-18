// cf_cctype_no_global.cpp — <cctype> はグローバル名前空間に名前を追加しないが、
// <ctype.h> は追加する(TC-CT-10、ILIBC-CTP-001 §8、ILIBC-TST-001 §5.4)。
// EXPECT-GCC:   has not been declared|was not declared|not declared in this scope
// EXPECT-CLANG: use of undeclared identifier|not declared
#include <cctype>

#ifdef CF_FAIL
// 失敗用: グローバルの isalpha は見つからないはず
int main() { return ::isalpha('A') != 0 ? 0 : 1; }
#else
// 対照(-DCF_CONTROL): <cctype> 単体で試験対象名前空間の isalpha が使えること
// (フリースタンディングのビルドでは ILIBC_NS は std なので TC-CT-10(a)の
// "std::isalpha が使える" と同値。T1 に従い ILIBC_NS 別名で指す)
namespace t = ILIBC_NS;
int main() { return t::isalpha('A') ? 0 : 1; }
#endif
