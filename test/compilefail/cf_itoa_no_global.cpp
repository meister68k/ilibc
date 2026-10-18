// cf_itoa_no_global.cpp — <cstdlib> と <stdlib.h> をインクルードしても、
// ::_itoa_s と std::_itoa_s は存在しない(拡張は ilibc:: にのみ置く。
// TC-SL-21(d)、ILIBC-STL-001 §2.5、§9、ILIBC-HDR-001 §6 W6)。
// EXPECT-GCC:   has not been declared|is not a member of
// EXPECT-CLANG: has not been declared|no member named
#include <cstdlib>
#include <stdlib.h>

#ifdef CF_FAIL
// 失敗用: グローバルにも std にも導入しない
int main() {
    char buf[8];
    return ::_itoa_s(1, buf, 10) + std::_itoa_s(1, buf, 10);
}
#else
// 対照(-DCF_CONTROL): ilibc:: に置かれている
int main() {
    char buf[8];
    return ilibc::_itoa_s(1, buf, 10);
}
#endif
