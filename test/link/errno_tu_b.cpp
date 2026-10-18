// errno_tu_b.cpp — <cerrno> の実体と初期値の検査(副側)。
// spec/ilibc_header_spec.md §10 TC-HD-09(d)、spec/ilibc_cerrno_spec.md §4.2。
// 別の翻訳単位から同じ g_errno 実体を読み書きする。errno_tu_a.cpp とリンクする。
#include <cerrno>

// A が設定した値を返し、代わりに別の値を書き込む(実体共有の往復確認)
int tu_b_readback() {
    const int v = errno;
    errno = 0x1234;
    return v;
}

// 同一実体であることをアドレスの一致で確認する
int *tu_b_address() { return &errno; }
