// fdio_inject.cpp — <cstdio> の故障注入試験(R: TC-IO-09)。
// spec/ilibc_cstdio_spec.md §5.2・§6 を検証する。この翻訳単位でのみ
// write トークンを fdio_mock_write に置換してから <cstdio> を取り込み、
// バックエンドの ::write 呼び出しだけを差し替える(§11 TC-IO-09)。
// 他の翻訳単位・ホスト libc の write には影響しない(§11 の許容範囲)。
// 通常運転のモックは実際の SYS_write に回し、注入モードでのみ結果を差し替える。
#define ILIBC_IO_BACKEND_POSIX
#include "../../common/harness/th_output.hpp"
#include <unistd.h>  // 実 write の宣言・pipe/read/close(E1 のホスト syscall)
#include <sys/syscall.h>
#include <cstddef>

// モックの宣言(glibc の write と同じ C リンケージ・noexcept)
extern "C" ssize_t fdio_mock_write(int fd, const void *buf, std::size_t n) noexcept;

#define write fdio_mock_write  // ::write のみマクロ化(<cstdio> の中だけで有効)
#include <cstdio>
#undef write

// プラットフォーム層(§5.6.2)。[[noreturn]] の位置は e1_stdio.cpp の注記のとおり
extern "C" [[noreturn]] void ilibc_test_exit(int status);

// ---------------------------------------------------------------- 注入装置

// 注入状態: 0=通常、1=部分書込み(最大 arg バイト)、2=0、3=-1、
// 4=初回だけ部分(arg)あとは-1
static int g_mode = 0;
static int g_arg = 0;
static int g_calls = 0;

// ::write の代わりに呼ばれる。通常時と部分書込み時は実際に書き、
// 0/-1 は何も書かずにスクリプトどおりの結果を返す
extern "C" ssize_t fdio_mock_write(int fd, const void *buf, std::size_t n) noexcept {
    ++g_calls;
    switch (g_mode) {
    case 1:
    case 4: {
        if (g_mode == 4 && g_calls > 1) {
            return -1;
        }
        const std::size_t arg = static_cast<std::size_t>(g_arg);
        const std::size_t k = (n < arg) ? n : arg;
        return ::syscall(SYS_write, fd, buf, k);
    }
    case 2:
        return 0;
    case 3:
        return -1;
    default:
        return ::syscall(SYS_write, fd, buf, n);
    }
}

// モックを mode に設定し、呼び出し回数をリセットする
static void arm(int mode, int arg) {
    g_mode = mode;
    g_arg = arg;
    g_calls = 0;
}

// 書込端を閉じた pipe から EOF まで読み、読めたバイト数を返す
static std::size_t drain(int rfd, char *out, std::size_t cap) {
    std::size_t n = 0;
    while (n < cap) {
        const ssize_t r = ::read(rfd, out + n, cap - n);
        if (r <= 0) {
            break;
        }
        n += static_cast<std::size_t>(r);
    }
    return n;
}

// ---------------------------------------------------------------- TC-IO-09

// 部分書込み・0・-1・途中失敗が規定どおり振る舞うか(成功0、失敗は検査番号)
static int tc_io09(void) {
    int p[2];
    if (::pipe(p) != 0) {
        return 1;
    }
    // 部分書込み→再送で全量到達し、戻り値は完全な長さ、呼び出し3回
    arm(1, 3);
    const int r1 = ilibc::fprint_f<"abcdefgh">(p[1]);
    const int c1 = g_calls;
    // 0 は進捗なしとして即座に失敗(再送ループに入らない)
    arm(2, 0);
    const int r2 = ilibc::fprint_f<"abcdefgh">(p[1]);
    const int c2 = g_calls;
    // -1 も即座に失敗
    arm(3, 0);
    const int r3 = ilibc::putchar('Q');
    // 初回部分→次回失敗で-1(途中バイトは pipe に残る: 原子性は保証しない)
    arm(4, 5);
    const int r4 = ilibc::fprint_f<"abcdefgh">(p[1]);
    const int c4 = g_calls;
    // fwrite は書き切れたバイトから完全な要素数を数え、端数は不算入
    static const char data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    arm(4, 5);
    const std::size_t w1 = ilibc::fwrite(data, 4, 2, p[1]);
    // putchar/fputc の失敗(0/-1)は-1を返す
    arm(2, 0);
    const int r5 = ilibc::putchar('A');
    arm(3, 0);
    const int r6 = ilibc::fputc('B', p[1]);
    arm(0, 0);
    ::close(p[1]);
    char got[32];
    const std::size_t gn = drain(p[0], got, sizeof got);
    ::close(p[0]);
    // 期待: 8(3+3+2) + 0 + 0 + 5 + 5(w1) + 0 + 0 = 18 バイト
    if (r1 != 8) {
        return 2;
    }
    if (c1 != 3) {
        return 3;
    }
    if (r2 != -1) {
        return 4;
    }
    if (c2 != 1) {  // 0 で即停止: 再送が無限に続くことは無い
        return 5;
    }
    if (r3 != -1) {
        return 6;
    }
    if (r4 != -1 || c4 != 2) {
        return 7;
    }
    if (w1 != 1) {  // 5バイト書けたが端数1バイトは不算入 → 1要素のみ完了
        return 8;
    }
    if (r5 != -1 || r6 != -1) {
        return 9;
    }
    if (gn != 18) {
        return 10;
    }
    static const char want[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    for (std::size_t i = 0; i < 8; ++i) {
        if (got[i] != want[i]) {
            return 11;
        }
    }
    return 0;
}

int main(void) {
    int nfail = 0;
    const int c9 = tc_io09();
    if (c9 == 0) {
        th_puts("TC-IO-09 PASS\n");
    } else {
        th_puts("TC-IO-09 FAIL case=");
        th_put_int(c9);
        th_putc('\n');
        ++nfail;
    }
    th_puts("SUMMARY pass=");
    th_put_int(1 - nfail);
    th_puts(" fail=");
    th_put_int(nfail);
    th_putc('\n');
    ilibc_test_exit((nfail == 0) ? 0 : 1);
}
