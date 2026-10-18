// fdio_input.cpp — <cstdio> 入力拡張のE1実行時試験(R: TC-IO-19/20/21/
// 22/23/25)。spec/ilibc_cstdio_spec.md §14 を検証する。入力は pipe に
// 決定的な内容を書き切って書込端を閉じ、読込端 fd を直接渡す。入力系は
// fd=1 を書き換えないので、報告はすべての試験の後にまとめて出せる(§5.6.2
// の出力ハーネスと干渉しない)。照合はこのファイル内の手書き比較のみ
// (R1: ライブラリ・ホスト文字列関数に依存しない)。
#define ILIBC_IO_BACKEND_POSIX
#include "../../common/harness/th_output.hpp"
#include <unistd.h>  // pipe/dup2/dup/read/write/close(E1 のホスト syscall)
#include <cstdio>
#include <cerrno>     // ILIBC_ERRNO / clear_errno
#include <cstddef>
#include <cstdint>    // SIZE_MAX

// プラットフォーム層(§5.6.2)。[[noreturn]] の位置は e1_stdio.cpp の注記のとおり
extern "C" [[noreturn]] void ilibc_test_exit(int status);

// ---------------------------------------------------------------- 補助

// 2つのNUL終端文字列が一致するか(nullptr は不一致)
static int streq(const char *a, const char *b) {
    if (a == nullptr) {
        return 0;
    }
    while (*a != 0 && *a == *b) {
        ++a;
        ++b;
    }
    return *a == 0 && *b == 0;
}

// 決定的な入力源を作る: data の n バイトを書き切って書込端を閉じ、
// そのとき以降の読み出しは全バイト受領後に EOF になる読込端 fd を返す
static int make_src(const char *data, unsigned int n, int *rfd) {
    int p[2];
    if (::pipe(p) != 0) {
        return -1;
    }
    if (::write(p[1], data, n) != static_cast<ssize_t>(n)) {
        ::close(p[0]);
        ::close(p[1]);
        return -1;
    }
    ::close(p[1]);
    *rfd = p[0];
    return 0;
}

// fgets が want と一致するか( nullptr は失敗 )
static int line_is(char *buf, int size, int fd, const char *want) {
    if (ilibc::fgets(buf, size, fd) == nullptr) {
        return 0;
    }
    return streq(buf, want);
}

// buf の先頭 n バイトが pat(長さ plen)と一致するか(fread は NUL 終端
// しないため文字列比較に置き換えられない)
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

// 両端を閉じた pipe を作り読込 fd を返す(閉じた fd への読み出しで
// ホストが本物のエラー番号を返す装置)
static int make_dead(int *rfd) {
    int p[2];
    if (::pipe(p) != 0) {
        return -1;
    }
    ::close(p[0]);
    ::close(p[1]);
    *rfd = p[0];
    return 0;
}

// ---------------------------------------------------------------- TC-IO-19

// fgets の改行保持・NUL終端・容量境界・行の継続・EOF(成功0、失敗は検査番号)
static int tc_io19(void) {
    char buf[32];
    int r;
    // (1) 改行までを1行として保持し、次行は次回に続けない
    if (make_src("abc\ndef\n", 8, &r) != 0) {
        return 1;
    }
    char *q = ilibc::fgets(buf, 32, r);
    if (q != buf || !streq(buf, "abc\n")) {
        return 2;
    }
    if (!line_is(buf, 32, r, "def\n")) {
        return 3;
    }
    // (2) 先頭バイト前の EOF は nullptr で、バッファには触れない
    buf[0] = 'Z';
    if (ilibc::fgets(buf, 32, r) != nullptr) {
        return 4;
    }
    if (buf[0] != 'Z') {
        return 5;
    }
    ::close(r);
    // (3) 改行のない部分行(EOF)は NUL 終端して返す
    if (make_src("xy", 2, &r) != 0) {
        return 6;
    }
    if (!line_is(buf, 32, r, "xy")) {
        return 7;
    }
    ::close(r);
    // (4) 容量境界と行の継続: size=4 で "abc" → "\n" → "def" → EOF
    if (make_src("abc\ndef", 7, &r) != 0) {
        return 8;
    }
    if (!line_is(buf, 4, r, "abc")) {
        return 9;
    }
    if (!line_is(buf, 4, r, "\n")) {
        return 10;
    }
    if (!line_is(buf, 4, r, "def")) {
        return 11;
    }
    buf[0] = 'Z';
    if (ilibc::fgets(buf, 4, r) != nullptr || buf[0] != 'Z') {
        return 12;
    }
    ::close(r);
    // (5) size=2 は1バイト+のちに NUL(バッファは2バイト分)
    if (make_src("z\n", 2, &r) != 0) {
        return 13;
    }
    if (!line_is(buf, 2, r, "z")) {
        return 14;
    }
    if (!line_is(buf, 2, r, "\n")) {
        return 15;
    }
    ::close(r);
    return 0;
}

// ---------------------------------------------------------------- TC-IO-20

// gets の配列限定・容量自動取得・デフォルトfd(成功0、失敗は検査番号)
static int tc_io20(void) {
    char arr[32];
    int r;
    int errv = 0;
    // (1) 配列サイズ分のバッファで2行読み、EOF は nullptr。
    //     gets は fgets と同じ動作(§14.1): 改行も格納して返す
    if (make_src("hello\nworld\n", 12, &r) != 0) {
        return 1;
    }
    if (ilibc::gets(arr, r) != arr || !streq(arr, "hello\n")) {
        return 2;
    }
    if (ilibc::gets(arr, r, ilibc::clear_errno(errv)) != arr
        || !streq(arr, "world\n")) {
        return 3;
    }
    if (ilibc::gets(arr, r) != nullptr) {
        return 4;
    }
    ::close(r);
    // (2) デフォルト fd=0: pipe の読込端を stdin に差し替えて使う
    if (make_src("via0\n", 5, &r) != 0) {
        return 5;
    }
    const int saved = ::dup(0);
    if (saved < 0) {
        ::close(r);
        return 6;
    }
    if (::dup2(r, 0) < 0) {
        ::close(saved);
        ::close(r);
        return 7;
    }
    const char *gp = ilibc::gets(arr);  // fd=0・err省略
    const int restored = ::dup2(saved, 0);
    ::close(saved);
    ::close(r);
    if (restored < 0) {
        return 8;
    }
    if (gp != arr || !streq(arr, "via0\n")) {  // 改行は格納される
        return 9;
    }
    // (3) 最小の配列 N=2 でも成立(1バイト+のちに NUL)
    char tiny[2];
    if (make_src("q\n", 2, &r) != 0) {
        return 10;
    }
    if (ilibc::gets(tiny, r) != tiny || !streq(tiny, "q")) {
        return 11;
    }
    ::close(r);
    return 0;
}

// ---------------------------------------------------------------- TC-IO-21

// fgetc/getchar の0〜255・EOF・エラー(成功0、失敗は検査番号)
static int tc_io21(void) {
    unsigned char all[256];
    for (int i = 0; i < 256; ++i) {
        all[i] = static_cast<unsigned char>(i);
    }
    int p[2];
    if (::pipe(p) != 0) {
        return 1;
    }
    if (::write(p[1], all, 256) != 256) {
        ::close(p[0]);
        ::close(p[1]);
        return 2;
    }
    ::close(p[1]);
    // (1) 全256値が unsigned char 由来の 0〜255 として返り、成功で *err 不変
    int errv = 0;
    for (int i = 0; i < 256; ++i) {
        if (ilibc::fgetc(p[0], ilibc::clear_errno(errv)) != i) {
            return 3;
        }
        if (errv != 0) {
            return 4;
        }
    }
    // (2) EOF は -1 で、*err は不変
    errv = 77;
    if (ilibc::fgetc(p[0], &errv) != -1) {
        return 5;
    }
    if (errv != 77) {
        return 6;
    }
    ::close(p[0]);
    // (3) 閉じた fd への読み出しは -1 と *err 設定、nullptr でも破棄できる
    int dead;
    if (make_dead(&dead) != 0) {
        return 7;
    }
    errv = 77;
    if (ilibc::fgetc(dead, &errv) != -1) {
        return 8;
    }
    if (errv == 77 || errv == 0) {
        return 9;
    }
    if (ilibc::fgetc(dead, nullptr) != -1) {
        return 10;
    }
    // (4) getchar は fd=0: 差し替えた pipe から1バイト、次で EOF
    int r;
    if (make_src("A", 1, &r) != 0) {
        return 11;
    }
    const int saved = ::dup(0);
    if (saved < 0 || ::dup2(r, 0) < 0) {
        ::close(saved);
        ::close(r);
        return 12;
    }
    errv = 0;
    const int g1 = ilibc::getchar(ilibc::clear_errno(errv));
    const int g2 = ilibc::getchar(ilibc::clear_errno(errv));
    const int ok = (::dup2(saved, 0) >= 0);
    ::close(saved);
    ::close(r);
    if (!ok) {
        return 13;
    }
    if (g1 != 'A') {
        return 14;
    }
    if (g2 != -1 || errv != 0) {  // EOF で *err 不変
        return 15;
    }
    return 0;
}

// ---------------------------------------------------------------- TC-IO-22

// fread の完全要素数・端数・EOF・ゼロ次元・溢れ(成功0、失敗は検査番号)
static int tc_io22(void) {
    char buf[32];
    int r;
    // (1) 10バイトを4バイト要素で読む: 完了2要素+端数2バイト、以降 EOF
    if (make_src("0123456789", 10, &r) != 0) {
        return 1;
    }
    int errv = 77;
    const std::size_t g1 = ilibc::fread(buf, 4, 3, r, &errv);  // 12バイト要求
    if (g1 != 2) {
        return 2;
    }
    if (errv != 77) {  // 部分読み後に EOF だが *err は不変
        return 3;
    }
    if (!eq_n(buf, 8, "01234567", 8)) {  // 完了要素のみが戻り値の根拠
        return 4;
    }
    if (buf[8] != '8' || buf[9] != '9') {  // 端数はバッファに残る
        return 5;
    }
    if (ilibc::fread(buf, 4, 1, r, &errv) != 0) {
        return 6;
    }
    if (errv != 77) {
        return 7;
    }
    ::close(r);
    // (2) ゼロ次元と size*count の溢れは読み出し前に検出(1バイトも消費しない)
    if (make_src("K", 1, &r) != 0) {
        return 8;
    }
    if (ilibc::fread(buf, 0, 5, r, nullptr) != 0) {
        return 9;
    }
    if (ilibc::fread(buf, 4, 0, r, nullptr) != 0) {
        return 10;
    }
    // ILP32 の size_t で 0x80000000*2 と 0xFFFFFFFF*2 は溢れる
    if (ilibc::fread(buf, static_cast<std::size_t>(0x80000000u), 2, r,
                     nullptr) != 0) {
        return 11;
    }
    if (ilibc::fread(buf, static_cast<std::size_t>(0xFFFFFFFFu), 2, r,
                     nullptr) != 0) {
        return 12;
    }
    if (ilibc::fgetc(r, nullptr) != 'K') {  // 4種とも1バイトも消費していない
        return 13;
    }
    ::close(r);
    return 0;
}

// ---------------------------------------------------------------- TC-IO-23

// err 明示・nullptr・省略の3経路と成功/EOF時の不変(成功0、失敗は検査番号)
static int tc_io23(void) {
    char buf[16];
    int r;
    int errv;
    // (1) 明示 &err: 成功でも EOF でも *err は不変
    if (make_src("ok", 2, &r) != 0) {
        return 1;
    }
    errv = 77;
    if (ilibc::fgets(buf, 16, r, &errv) == nullptr) {
        return 2;
    }
    if (errv != 77) {
        return 3;
    }
    if (ilibc::fgets(buf, 16, r, &errv) != nullptr) {
        return 4;
    }
    if (errv != 77) {
        return 5;
    }
    ::close(r);
    // (2) nullptr: エラー経路でも破棄されてクラッシュしない
    int dead;
    if (make_dead(&dead) != 0) {
        return 6;
    }
    if (ilibc::fgets(buf, 16, dead, nullptr) != nullptr) {
        return 7;
    }
    // (3) 省略版: エラーは g_errno に転送され、成功では不変
    if (make_src("x", 1, &r) != 0) {
        return 8;
    }
    ILIBC_ERRNO = 0;
    if (ilibc::fgets(buf, 16, r) == nullptr) {
        return 9;
    }
    if (ILIBC_ERRNO != 0) {
        return 10;
    }
    ::close(r);
    if (ilibc::fgetc(dead) != -1) {
        return 11;
    }
    if (ILIBC_ERRNO == 0) {
        return 12;
    }
    // (4) err==nullptr は転送先が無いので g_errno も触られない
    ILIBC_ERRNO = 0;
    if (ilibc::fgetc(dead, nullptr) != -1) {
        return 13;
    }
    if (ILIBC_ERRNO != 0) {
        return 14;
    }
    return 0;
}

// ---------------------------------------------------------------- TC-IO-25

// clear_errno との結合とホストerrno分離(成功0、失敗は検査番号)
static int tc_io25(void) {
    char line[16];
    int r;
    // (1) clear_errno(int&) で渡した err に失敗が書き込まれる
    int local = 0;
    int dead;
    if (make_dead(&dead) != 0) {
        return 1;
    }
    if (ilibc::fgets(line, 16, dead, ilibc::clear_errno(local)) != nullptr) {
        return 2;
    }
    if (local == 0) {
        return 3;
    }
    // (2) 成功パス: クリア後の 0 のまま不変
    if (make_src("ok\n", 3, &r) != 0) {
        return 4;
    }
    local = 77;
    if (ilibc::fgets(line, 16, r, ilibc::clear_errno(local)) == nullptr) {
        return 5;
    }
    if (local != 0) {
        return 6;
    }
    ::close(r);
    // (3) clear_errno() は g_errno を返し、ホスト errno セルとは別実体
    int *host = __errno_location();
    *host = 111;
    ILIBC_ERRNO = 77;
    int *gp = ilibc::clear_errno();
    if (gp != &ILIBC_ERRNO) {
        return 7;
    }
    if (ILIBC_ERRNO != 0) {
        return 8;
    }
    if (*host != 111) {  // ホスト側は不変: 分離した2つのセル
        return 9;
    }
    ILIBC_ERRNO = 5;
    if (*host != 111) {  // 逆方向にも漏れない
        return 10;
    }
    *host = 0;  // 後片付け
    return 0;
}

int main(void) {
    int nfail = 0;

    // 各TCの最初の失敗箇所を番号で報告する
    const int c19 = tc_io19();
    if (c19 == 0) {
        th_puts("TC-IO-19 PASS\n");
    } else {
        th_puts("TC-IO-19 FAIL case=");
        th_put_int(c19);
        th_putc('\n');
        ++nfail;
    }
    const int c20 = tc_io20();
    if (c20 == 0) {
        th_puts("TC-IO-20 PASS\n");
    } else {
        th_puts("TC-IO-20 FAIL case=");
        th_put_int(c20);
        th_putc('\n');
        ++nfail;
    }
    const int c21 = tc_io21();
    if (c21 == 0) {
        th_puts("TC-IO-21 PASS\n");
    } else {
        th_puts("TC-IO-21 FAIL case=");
        th_put_int(c21);
        th_putc('\n');
        ++nfail;
    }
    const int c22 = tc_io22();
    if (c22 == 0) {
        th_puts("TC-IO-22 PASS\n");
    } else {
        th_puts("TC-IO-22 FAIL case=");
        th_put_int(c22);
        th_putc('\n');
        ++nfail;
    }
    const int c23 = tc_io23();
    if (c23 == 0) {
        th_puts("TC-IO-23 PASS\n");
    } else {
        th_puts("TC-IO-23 FAIL case=");
        th_put_int(c23);
        th_putc('\n');
        ++nfail;
    }
    const int c25 = tc_io25();
    if (c25 == 0) {
        th_puts("TC-IO-25 PASS\n");
    } else {
        th_puts("TC-IO-25 FAIL case=");
        th_put_int(c25);
        th_putc('\n');
        ++nfail;
    }
    th_puts("SUMMARY pass=");
    th_put_int(6 - nfail);
    th_puts(" fail=");
    th_put_int(nfail);
    th_putc('\n');
    ilibc_test_exit((nfail == 0) ? 0 : 1);
}
