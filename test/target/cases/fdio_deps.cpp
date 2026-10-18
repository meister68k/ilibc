// fdio_deps.cpp — <cstdio> 利用コードの実行時依存を nm で検査するための
// 翻訳単位(TC-IO-13、ILIBC-IO-001 §11)。テスト自身のホスト syscall を
// 混ぜないよう、ilibc の出力関数だけを参照する。main を持たず、リンクせず
// -c でコンパイルして未解決参照を検査する(読み取り専用の参照関数)。
// 期待される未解決シンボルは write だけ(GOT は許可集合に含む)。
#define ILIBC_IO_BACKEND_POSIX
#include <cstdio>
#include <cstddef>

// 全出力関数を1回ずつ参照し、リンク時に不要な依存が混ざらないこと
extern "C" int fdio_deps_use(void) {
    int n = ilibc::print_f<"x=%d %s">(1, "s");
    n += ilibc::fprint_f<"y">(2);
    n += ilibc::putstr("");
    n += ilibc::puts("z");
    n += ilibc::putchar('c');
    n += ilibc::fputc('d', 3);
    n += ilibc::fputs("e", 4);
    n += static_cast<int>(ilibc::fwrite("f", 1, 1, 5));
    char buf[8];
    n += ilibc::snprint_f<"%d">(buf, sizeof buf, 1);
    return n;
}
