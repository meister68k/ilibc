// cf_itoa_nodiscard.cpp — 戻り値(エラーコード)を無視する _itoa_s の呼び出しは、
// [[nodiscard]] により -Wunused-result が出て、-Werror でコンパイルエラーになる
// (TC-SL-21(c)、ILIBC-STL-001 §2.5、§4.5)。
// EXPECT-GCC:   unused result|ignoring return value
// EXPECT-CLANG: unused result|ignoring return value
#include <cstdlib>

#ifdef CF_FAIL
// 失敗用: 戻り値を捨てている(検査の成否が伝わらない呼び出しは禁止)
int main() {
    char buf[8];
    ilibc::_itoa_s(1, buf, 10);
    return 0;
}
#else
// 対照(-DCF_CONTROL): 戻り値を受け取れば成立する
int main() {
    char buf[8];
    return ilibc::_itoa_s(1, buf, 10);
}
#endif
