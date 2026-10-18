// fdio_runtime.cpp — <cstdio> のE1実行時試験(R: TC-IO-08/10/11/12)。
// spec/ilibc_cstdio_spec.md §6・§8・§9 を検証する。fd=1 系の出力は pipe で
// 捕獲し、fd 指定系は pipe の書込端を直接渡す。捕獲は起動直後(glibc の
// stdout バッファが空のうち)に行い、報告は捕獲の復帰後にまとめて出す
// (§5.6.2 の出力ハーネスと干渉しないための順序)。照合はこのファイル内の
// 手書き比較のみ(R1: ライブラリ・ホスト文字列関数に依存しない)。
// printf マクロは §9 の規定形をこの翻訳単位で有効にして使う(ON 検証)。
#define ILIBC_ENABLE_PRINTF_MACRO
#define ILIBC_IO_BACKEND_POSIX
#include "../../common/harness/th_output.hpp"
#include <unistd.h>  // pipe/dup2/read/close(E1 のホスト syscall)
#include <cstdio>
#include <cstddef>

// プラットフォーム層(§5.6.2)。[[noreturn]] の位置は e1_stdio.cpp の注記のとおり
extern "C" [[noreturn]] void ilibc_test_exit(int status);

// ---------------------------------------------------------------- 捕獲補助

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

// buf の先頭 n バイトが pat(長さ plen)と一致するか
static int eq_n(const char *buf, std::size_t n, const char *pat, std::size_t plen) {
    if (n != plen) {
        return 0;
    }
    for (std::size_t i = 0; i < plen; ++i) {
        if (buf[i] != pat[i]) {
            return 0;
        }
    }
    return 1;
}

// fd=1 への出力を pipe で捕獲する。ops は出力と戻り値の産み手で、捕獲中は
// 報告出力を禁止し、rets とバイト列だけを持ち帰る。失敗は-1(中身は未定)。
static int capture_fd1(void (*ops)(int *), int *rets, char *out, std::size_t cap,
                       std::size_t *outlen) {
    int p[2];
    *outlen = 0;
    if (::pipe(p) != 0) {
        return -1;
    }
    const int saved = ::dup(1);
    if (saved < 0) {
        ::close(p[0]);
        ::close(p[1]);
        return -1;
    }
    if (::dup2(p[1], 1) < 0) {
        ::close(saved);
        ::close(p[0]);
        ::close(p[1]);
        return -1;
    }
    ::close(p[1]);
    ops(rets);  // 捕獲中: th_puts 等の報告禁止
    if (::dup2(saved, 1) < 0) {
        ::close(saved);
        ::close(p[0]);
        return -1;
    }
    ::close(saved);
    *outlen = drain(p[0], out, cap);
    ::close(p[0]);
    return 0;
}

// ------------------------------------------------------- 捕獲中の出力(ops)

// TC-IO-08: 書式化2種の出力と戻り値
static void ops_08(int *rets) {
    rets[0] = ilibc::print_f<"a=%d s=%s %%">(42, "hi");
    rets[1] = ilibc::print_f<"">();
}

// TC-IO-10: 改行の有無と戻り値(puts("") は "\n" の1バイト)
static void ops_10(int *rets) {
    rets[0] = ilibc::putstr("xy");
    rets[1] = ilibc::puts("ab");
    rets[2] = ilibc::puts("");
}

// TC-IO-11: unsigned char 変換(300→44、-1→255)
static void ops_11(int *rets) {
    rets[0] = ilibc::putchar(300);
    rets[1] = ilibc::putchar(-1);
}

// TC-IO-12: printf マクロ(規定形、ゼロ値引数も可)。出力後に対象外ヘッダが
// 続いても壊れないよう、使用後には #undef して後段のコードを守る
static void ops_12(int *rets) {
    rets[0] = printf("n=%d", 5);
    rets[1] = printf("z");
}

#undef printf  // §9: 必要なら解除できる

// ---------------------------------------------------------------- 各TC

// TC-IO-08: pipe 経由の print_f/fprint_f 出力と戻り値(成功0、失敗は検査番号)
static int tc_io08(void) {
    char buf[64];
    std::size_t n = 0;
    int rets[2] = {0, 0};
    if (capture_fd1(ops_08, rets, buf, sizeof buf, &n) != 0) {
        return 1;
    }
    if (rets[0] != 11) {
        return 2;
    }
    if (rets[1] != 0) {
        return 3;
    }
    if (!eq_n(buf, n, "a=42 s=hi %", 11)) {
        return 4;
    }
    // fprint_f は指定 fd へ(値9で "x=9!" の4バイト)
    int p[2];
    if (::pipe(p) != 0) {
        return 5;
    }
    const int rf = ilibc::fprint_f<"x=%d!">(p[1], 9);
    ::close(p[1]);
    char got[8];
    const std::size_t gn = drain(p[0], got, sizeof got);
    ::close(p[0]);
    if (rf != 4) {
        return 6;
    }
    if (!eq_n(got, gn, "x=9!", 4)) {
        return 7;
    }
    return 0;
}

// TC-IO-10: putstr/puts/fputs の内容・改行・戻り値(成功0、失敗は検査番号)
static int tc_io10(void) {
    char buf[64];
    std::size_t n = 0;
    int rets[3] = {0, 0, 0};
    if (capture_fd1(ops_10, rets, buf, sizeof buf, &n) != 0) {
        return 1;
    }
    if (rets[0] != 2) {
        return 2;
    }
    if (rets[1] != 3) {  // "ab" + 改行
        return 3;
    }
    if (rets[2] != 1) {  // 空文字列でも改行は出る
        return 4;
    }
    if (!eq_n(buf, n, "xyab\n\n", 6)) {
        return 5;
    }
    // fputs は指定 fd へ・改行なし
    int p[2];
    if (::pipe(p) != 0) {
        return 6;
    }
    const int f1 = ilibc::fputs("q7", p[1]);
    const int f2 = ilibc::fputs("", p[1]);
    ::close(p[1]);
    char got[8];
    const std::size_t gn = drain(p[0], got, sizeof got);
    ::close(p[0]);
    if (f1 != 2 || f2 != 0) {
        return 7;
    }
    if (!eq_n(got, gn, "q7", 2)) {
        return 8;
    }
    return 0;
}

// TC-IO-11: putchar/fputc の戻り値と fwrite の完了要素数・0件・溢れ
// (成功0、失敗は検査番号)
static int tc_io11(void) {
    char buf[64];
    std::size_t n = 0;
    int rets[2] = {0, 0};
    if (capture_fd1(ops_11, rets, buf, sizeof buf, &n) != 0) {
        return 1;
    }
    if (rets[0] != 44) {
        return 2;
    }
    if (rets[1] != 255) {
        return 3;
    }
    // 送られるのは unsigned char に変換した1バイトずつ
    if (n != 2 || buf[0] != static_cast<char>(44) || buf[1] != static_cast<char>(0xFF)) {
        return 4;
    }
    // fputc は指定 fd へ同じ1バイト
    int p0[2];
    if (::pipe(p0) != 0) {
        return 5;
    }
    const int rc = ilibc::fputc('K', p0[1]);
    ::close(p0[1]);
    char one[4];
    const std::size_t on = drain(p0[0], one, sizeof one);
    ::close(p0[0]);
    if (rc != 75 || on != 1 || one[0] != 'K') {
        return 6;
    }
    // fwrite: 完了要素数・size/count 0・size*count の溢れ(出力前に検出)
    int p[2];
    if (::pipe(p) != 0) {
        return 7;
    }
    static const char data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    const std::size_t w1 = ilibc::fwrite(data, 4, 2, p[1]);
    const std::size_t w2 = ilibc::fwrite(data, 0, 5, p[1]);
    const std::size_t w3 = ilibc::fwrite(data, 5, 0, p[1]);
    // 0x40000001 * 4 は 32bit の size_t で溢れる(検出して0・書込みなし)
    const std::size_t w4 = ilibc::fwrite(data, static_cast<std::size_t>(0x40000001u), 4, p[1]);
    ::close(p[1]);
    char got[16];
    const std::size_t gn = drain(p[0], got, sizeof got);
    ::close(p[0]);
    if (w1 != 2) {
        return 8;
    }
    if (w2 != 0 || w3 != 0 || w4 != 0) {
        return 9;
    }
    if (gn != 8) {  // 溢れ4種は1バイトも書いていない
        return 10;
    }
    for (std::size_t i = 0; i < 8; ++i) {
        if (got[i] != data[i]) {
            return 11;
        }
    }
    return 0;
}

// TC-IO-12: printf マクロの ON と出力(成功0、失敗は検査番号)
static int tc_io12(void) {
    char buf[64];
    std::size_t n = 0;
    int rets[2] = {0, 0};
    if (capture_fd1(ops_12, rets, buf, sizeof buf, &n) != 0) {
        return 1;
    }
    if (rets[0] != 3) {
        return 2;
    }
    if (rets[1] != 1) {
        return 3;
    }
    if (!eq_n(buf, n, "n=5z", 4)) {
        return 4;
    }
    return 0;
}

int main(void) {
    int nfail = 0;

    // 各TCの最初の失敗箇所を番号で報告する
    const int c8 = tc_io08();
    if (c8 == 0) {
        th_puts("TC-IO-08 PASS\n");
    } else {
        th_puts("TC-IO-08 FAIL case=");
        th_put_int(c8);
        th_putc('\n');
        ++nfail;
    }
    const int c10 = tc_io10();
    if (c10 == 0) {
        th_puts("TC-IO-10 PASS\n");
    } else {
        th_puts("TC-IO-10 FAIL case=");
        th_put_int(c10);
        th_putc('\n');
        ++nfail;
    }
    const int c11 = tc_io11();
    if (c11 == 0) {
        th_puts("TC-IO-11 PASS\n");
    } else {
        th_puts("TC-IO-11 FAIL case=");
        th_put_int(c11);
        th_putc('\n');
        ++nfail;
    }
    const int c12 = tc_io12();
    if (c12 == 0) {
        th_puts("TC-IO-12 PASS\n");
    } else {
        th_puts("TC-IO-12 FAIL case=");
        th_put_int(c12);
        th_putc('\n');
        ++nfail;
    }
    th_puts("SUMMARY pass=");
    th_put_int(4 - nfail);
    th_puts(" fail=");
    th_put_int(nfail);
    th_putc('\n');
    ilibc_test_exit((nfail == 0) ? 0 : 1);
}
