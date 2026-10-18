// fdio_read_inject.cpp — <cstdio> 入力の故障注入試験(R: TC-IO-24)。
// spec/ilibc_cstdio_spec.md §14.5 を検証する。この翻訳単位でのみ read
// トークンを fdio_mock_read に置換してから <cstdio> を取り込み、
// バックエンドの ::read 呼び出しだけを差し替える(§11 TC-IO-24)。
// 他の翻訳単位・ホスト libc の read には影響しない(§11 の許容範囲)。
// 通常運転のモックは実際の SYS_read に回し、注入モードでのみ結果を差し替える。
// 検証内容: POSIX ::read 経路(pipe)、1バイト/回の先読み防止、短いreadの
// 再送ループ、強制EOF、注入エラーのホストエラー番号転送、err省略/nullptr。
#define ILIBC_IO_BACKEND_POSIX
#include "../../common/harness/th_output.hpp"
#include <unistd.h>  // 実 read の宣言・pipe/close(E1 のホスト syscall)
#include <sys/syscall.h>
#include <cstddef>
#include <cerrno>     // ILIBC_ERRNO

// モックの宣言(glibc の read と同じ C リンケージ・noexcept)
extern "C" ssize_t fdio_mock_read(int fd, void *buf, std::size_t n) noexcept;

#define read fdio_mock_read  // ::read のみマクロ化(<cstdio> の中だけで有効)
#include <cstdio>
#undef read

// プラットフォーム層(§5.6.2)。[[noreturn]] の位置は e1_stdio.cpp の注記のとおり
extern "C" [[noreturn]] void ilibc_test_exit(int status);

// ---------------------------------------------------------------- 注入装置

// 注入状態: 0=通常、1=短いread(最大 arg バイト)、2=強制0(EOF)、
// 3=-1(ホストerrnoを5=EIOにして返す)
static int g_mode = 0;
static int g_arg = 0;
static int g_calls = 0;

// ::read の代わりに呼ばれる。通常時と短いread時は実際に読み、
// 0/-1 は何も読まずにスクリプトどおりの結果を返す
extern "C" ssize_t fdio_mock_read(int fd, void *buf, std::size_t n) noexcept {
    ++g_calls;
    switch (g_mode) {
    case 1: {
        const std::size_t arg = static_cast<std::size_t>(g_arg);
        const std::size_t k = (n < arg) ? n : arg;
        return ::syscall(SYS_read, fd, buf, k);
    }
    case 2:
        return 0;
    case 3:
        *__errno_location() = 5;  // 注入するホストエラー番号(EIO)
        return -1;
    default:
        return ::syscall(SYS_read, fd, buf, n);
    }
}

// モックを mode に設定し、呼び出し回数をリセットする
static void arm(int mode, int arg) {
    g_mode = mode;
    g_arg = arg;
    g_calls = 0;
}

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

// pipe を作り data を書き切って書込端を閉じ、読込端 fd を返す
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

// ---------------------------------------------------------------- TC-IO-24

// POSIX ::read・pipe・故障注入・エラー番号転送(成功0、失敗は検査番号)
static int tc_io24(void) {
    char buf[32];
    int r;
    int errv;

    // (1) POSIX ::read 経路(通常運転): 改行ちょうどで止まり、次行を
    //     先読みしない(1回の read_some は1バイトのみ)
    if (make_src("a\nb\n", 4, &r) != 0) {
        return 1;
    }
    arm(0, 0);
    if (ilibc::fgets(buf, 32, r) == nullptr) {
        return 2;
    }
    const int c1 = g_calls;
    if (!streq(buf, "a\n")) {
        return 3;
    }
    if (c1 != 2) {  // 'a' と '\n' の2回のみ: 'b' は未読
        return 4;
    }
    if (ilibc::fgets(buf, 32, r) == nullptr || !streq(buf, "b\n")) {
        return 5;
    }
    if (g_calls != 4) {
        return 6;
    }
    ::close(r);

    // (2) 短いread(毎回最大3バイト): fread はループで読み切る
    if (make_src("0123456789", 10, &r) != 0) {
        return 7;
    }
    arm(1, 3);
    errv = 0;
    const std::size_t g1 = ilibc::fread(buf, 4, 2, r, &errv);  // 8バイト要求
    const int c2 = g_calls;
    if (g1 != 2) {
        return 8;
    }
    if (!eq_n(buf, 8, "01234567", 8)) {
        return 9;
    }
    if (c2 != 3) {  // 3+3+2 バイトの3回
        return 10;
    }
    if (errv != 0) {  // 成功で *err 不変
        return 11;
    }
    ::close(r);

    // (2b) 短いread(毎回1バイト)でも fgets は行をまたいで成立する
    if (make_src("xy\n", 3, &r) != 0) {
        return 12;
    }
    arm(1, 1);
    if (ilibc::fgets(buf, 32, r) == nullptr) {
        return 13;
    }
    if (!streq(buf, "xy\n")) {
        return 14;
    }
    if (g_calls != 3) {
        return 15;
    }
    ::close(r);

    // (3) 強制EOF: データが残っていても nullptr/-1、*err は不変
    if (make_src("zz", 2, &r) != 0) {
        return 16;
    }
    arm(2, 0);
    errv = 77;
    if (ilibc::fgets(buf, 32, r) != nullptr) {
        return 17;
    }
    if (errv != 77) {
        return 18;
    }
    if (ilibc::fgetc(r, &errv) != -1 || errv != 77) {
        return 19;
    }
    ::close(r);

    // (4) 注入エラー: モックが設定したホストエラー番号5が err に転送される
    if (make_src("q", 1, &r) != 0) {
        return 20;
    }
    arm(3, 0);
    errv = 999;
    if (ilibc::fgetc(r, &errv) != -1) {
        return 21;
    }
    if (errv != 5) {
        return 22;
    }
    errv = 999;
    if (ilibc::fgets(buf, 32, r, &errv) != nullptr || errv != 5) {
        return 23;
    }
    if (ilibc::fgetc(r, nullptr) != -1) {  // nullptr は破棄
        return 24;
    }
    ILIBC_ERRNO = 0;
    if (ilibc::fgetc(r) != -1) {  // 省略版は g_errno へ
        return 25;
    }
    if (ILIBC_ERRNO != 5) {
        return 26;
    }
    errv = 999;
    if (ilibc::fread(buf, 1, 4, r, &errv) != 0 || errv != 5) {
        return 27;
    }
    ::close(r);

    // (5) 正常経路で *err 不変(成功・EOF)は注入なしでも確認する
    if (make_src("k", 1, &r) != 0) {
        return 28;
    }
    arm(0, 0);
    errv = 77;
    if (ilibc::fgets(buf, 32, r, &errv) == nullptr || errv != 77) {
        return 29;
    }
    errv = 77;
    if (ilibc::fgetc(r, &errv) != -1 || errv != 77) {
        return 30;
    }
    ::close(r);
    arm(0, 0);
    return 0;
}

int main(void) {
    int nfail = 0;
    const int c24 = tc_io24();
    if (c24 == 0) {
        th_puts("TC-IO-24 PASS\n");
    } else {
        th_puts("TC-IO-24 FAIL case=");
        th_put_int(c24);
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
