// th_output.hpp — 試験用の最小出力ヘルパ(ilibc の関数を使わない。ILIBC-TST-001 §5.6.1 R1)。
// 出力先はプラットフォーム層の ilibc_test_putc(§5.6.2。E1 では stdio)。
// printf や <string> は使わない(R1)。
#pragma once

extern "C" void ilibc_test_putc(char c);

// 1文字 c を出力する
inline void th_putc(char c) {
    ilibc_test_putc(c);
}

// 文字列 s をそのまま出力する(NUL終端)
inline void th_puts(const char *s) {
    while (*s != '\0') {
        ilibc_test_putc(*s);
        ++s;
    }
}

// int 値を10進で出力する(INT_MIN でも壊れないよう、負値は unsigned の絶対値で処理する)
inline void th_put_int(int v) {
    unsigned u = (v < 0) ? (0u - static_cast<unsigned>(v)) : static_cast<unsigned>(v);
    char buf[10];  // 32bit unsigned の最大10桁
    int n = 0;
    do {
        buf[n] = static_cast<char>('0' + static_cast<int>(u % 10u));
        ++n;
        u /= 10u;
    } while (u != 0u);
    if (v < 0) {
        ilibc_test_putc('-');
    }
    while (n > 0) {
        --n;
        ilibc_test_putc(buf[n]);
    }
}
