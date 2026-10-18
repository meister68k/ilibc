// cf_itoa_uchar_array.cpp — unsigned char / signed char の配列をバッファに
// 渡すとコンパイルエラーになる(要素型は char のみ。TC-SL-21(b)、
// ILIBC-STL-001 §2.5、§9)。
// EXPECT-GCC:   no matching function
// EXPECT-CLANG: no matching function|no viable
#include <cstdlib>

#ifdef CF_FAIL
// 失敗用: unsigned char[N] は char (&)[N] にマッチしない
int main() {
    unsigned char buf[8];
    return ilibc::_itoa_s(1, buf, 10);
}
#else
// 対照(-DCF_CONTROL): char の配列なら成立する
int main() {
    char buf[8];
    return ilibc::_itoa_s(1, buf, 10);
}
#endif
