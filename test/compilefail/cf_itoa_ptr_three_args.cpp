// cf_itoa_ptr_three_args.cpp — char* のポインタに size を省略した3引数形
// _itoa_s(v, ptr, r) は、バッファ長が分からないためコンパイルエラーになる
// (TC-SL-21(a)、ILIBC-STL-001 §2.5、§9)。
// EXPECT-GCC:   no matching function
// EXPECT-CLANG: no matching function|no viable
#include <cstdlib>

#ifdef CF_FAIL
// 失敗用: char* は配列版にマッチせず、3引数ではポインタ版も選べない
int main() {
    char buf[8];
    char *p = buf;
    return ilibc::_itoa_s(1, p, 10);
}
#else
// 対照(-DCF_CONTROL): size を明示した4引数形なら成立する
int main() {
    char buf[8];
    char *p = buf;
    return ilibc::_itoa_s(1, p, 8, 10);
}
#endif
