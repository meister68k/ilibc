// e1_stdio.cpp — E1(ホスト -m32)のプラットフォーム層(ILIBC-TST-001 §5.6.2)。
// 出力は stdio、終了は exit。ホスト専用(E2〜E4 はセミホスティング実装を使う)。
// <cstdlib> は本ライブラリのヘッダで遮蔽され、exit を持たないため、ホストの
// exit は直接宣言して使う(プラットフォーム層はホスト施設を使ってよい)。
// <cstdio> もスタブされるため、素通りしてコンパイラ付属のものを取り込む
// (ヘッダ仕様 §7.3 S3。ホスト施設を使う試験コードだけの設定)。
#define ILIBC_STUB_PASSTHROUGH
#include <cstdio>

extern "C" [[noreturn]] void exit(int status) noexcept;

extern "C" void ilibc_test_putc(char c) {
    std::fputc(static_cast<unsigned char>(c), stdout);
}

// [[noreturn]] は extern "C" の後に置く(GCCは属性-リンケージ指定の順を受理しない。
// ILIBC-TST-001 §5.6.2 の記載順 [[noreturn]] extern "C" は GCC でコンパイル不可のため)
extern "C" [[noreturn]] void ilibc_test_exit(int status) {
    exit(status);
}
