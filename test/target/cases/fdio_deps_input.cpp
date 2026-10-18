// fdio_deps_input.cpp — 入力側 <cstdio> 利用コードの実行時依存を nm で
// 検査する翻訳単位(TC-IO-13、ILIBC-IO-001 §14.5)。テスト自身のホスト
// syscall を混ぜないよう、ilibc の入力関数だけを参照する。main を持たず、
// リンクせず -c でコンパイルして未解決参照を検査する(読み取り専用の
// 参照関数)。期待される未解決シンボルは read と __errno_location だけ
// (GOT は許可集合に含む)。ヒープ・va_arg・printf 等が混ざらないこと、
// および §14.5 のホストerrno取得参照が実際に現れることを検査する。
#define ILIBC_IO_BACKEND_POSIX
#include <cstdio>
#include <cstddef>

// 全入力関数を1回ずつ参照し、リンク時に不要な依存が混ざらないこと
extern "C" int fdio_deps_input_use(void) {
    char line[8];
    char buf[16];
    int e = 0;
    int n = 0;
    if (ilibc::fgets(line, 8, 0, &e) != nullptr) {
        n += 1;
    }
    if (ilibc::gets(line, 0, &e) != nullptr) {
        n += 1;
    }
    n += ilibc::fgetc(0, &e);
    n += ilibc::getchar(&e);
    n += static_cast<int>(ilibc::fread(buf, 4, 4, 0, &e));
    n += static_cast<int>(ilibc::fread(buf, 0, 4, 0, nullptr));
    return n + e;
}
