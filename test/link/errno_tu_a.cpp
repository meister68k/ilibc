// errno_tu_a.cpp — <cerrno> の実体と初期初期値の検査(主側)。
// spec/ilibc_header_spec.md §10 TC-HD-09(c)(d)、spec/ilibc_cerrno_spec.md §4.2。
// (c) 初期値が0であること、(d) 翻訳単位Bと g_errno の実体が1つであることを
// 実行で確認する。nm による弱シンボル1個の検査は test/Makefile の artifact が行う。
#include <cerrno>

int tu_b_readback();
int *tu_b_address();

int main() {
    // (c) 定数初期化のみで .bss に置かれるため、起動直後は 0 のはず
    if (errno != 0) {
        return 1;
    }
    errno = 0x5A5A;
    // (d) B が同一実体を読み取って書き換えるので、A からも変化が見える
    if (tu_b_readback() != 0x5A5A) {
        return 2;
    }
    if (errno != 0x1234) {
        return 3;
    }
    // アドレス取得も成立し、B から見たアドレスと一致する
    int *p = &errno;
    if (p != tu_b_address()) {
        return 4;
    }
    return 0;
}
