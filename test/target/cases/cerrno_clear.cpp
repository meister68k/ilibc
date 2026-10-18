// cerrno_clear.cpp — <cerrno> の clear_errno 単体試験(R: CE-01〜03)。
// spec/ilibc_test_spec.md §14 と ILIBC-ERR-001 §12 を検証する。
// CE-01 型・アドレス一致と constexpr 性、CE-02 グローバル版とホスト
// errno の分離、CE-03 入力 API との結合。入力 API を使うため
// POSIX バックエンドの <cstdio> を取り込む(§14.5 の errno 取得手段)。
#define ILIBC_IO_BACKEND_POSIX
#include "../../common/harness/th_output.hpp"
#include <unistd.h>  // pipe/write/close(E1 のホスト syscall)
#include <cstdio>
#include <cerrno>
#include <cstddef>

// プラットフォーム層(§5.6.2)。[[noreturn]] の位置は e1_stdio.cpp の注記のとおり
extern "C" [[noreturn]] void ilibc_test_exit(int status);

// ---------------------------------------------------------------- CE-01

// constexpr 性: 定数評価の中で参照版がクリアとアドレス取得を行う
// (ILIBC-ERR-001 §12 のconstexpr規定)
static constexpr int ce01_probe(void) {
    int e = 99;
    int *p = ilibc::clear_errno(e);
    return (e == 0 && p == &e) ? 1 : 0;
}
static_assert(ce01_probe() == 1);

static int ce01_sink = 0;

// 両オーバーロードとも noexcept で、結果型は int*(代入でも検査できる)
static_assert(noexcept(ilibc::clear_errno(ce01_sink)));
static_assert(noexcept(ilibc::clear_errno()));

// 型・アドレス一致と constexpr 性(成功0、失敗は検査番号)
static int ce01(void) {
    ce01_sink = 5;
    int *p = ilibc::clear_errno(ce01_sink);  // int* でなければ成立しない
    if (ce01_sink != 0) {
        return 1;
    }
    if (p != &ce01_sink) {
        return 2;
    }
    // ランタイムでも同じ性質が保たれる
    int e = 99;
    int *q = ilibc::clear_errno(e);
    if (e != 0 || q != &e) {
        return 3;
    }
    return 0;
}

// ---------------------------------------------------------------- CE-02

// グローバル版とホスト errno 分離(成功0、失敗は検査番号)
static int ce02(void) {
    int *host = __errno_location();  // ホストの errno セル(io_backend §14.5)
    *host = 111;
    ILIBC_ERRNO = 77;
    int *p = ilibc::clear_errno();
    if (p != &ILIBC_ERRNO) {  // 返るアドレスは g_errno そのもの
        return 1;
    }
    if (ILIBC_ERRNO != 0) {  // クリア済み
        return 2;
    }
    if (*host != 111) {  // グローバル版の書込みがホストに漏れない
        return 3;
    }
    ILIBC_ERRNO = 5;
    if (*host != 111) {  // 逆方向にも漏れない: 完全に別実体
        return 4;
    }
    // ホスト側の書込みが g_errno に漏れないこと
    *host = 42;
    if (ILIBC_ERRNO != 5) {
        return 5;
    }
    *host = 0;  // 後片付け
    ILIBC_ERRNO = 0;
    return 0;
}

// ---------------------------------------------------------------- CE-03

// 入力 API との結合(成功0、失敗は検査番号)
static int ce03(void) {
    char line[16];
    int p[2];
    // (1) clear_errno(int&) で渡した err に失敗が書き込まれる
    if (::pipe(p) != 0) {
        return 1;
    }
    ::close(p[0]);
    ::close(p[1]);
    int local = 99;
    if (ilibc::fgets(line, 16, p[0], ilibc::clear_errno(local)) != nullptr) {
        return 2;
    }
    if (local == 0 || local == 99) {  // ホストエラー番号が入った
        return 3;
    }
    // (2) 成功パス: クリア済みのまま 0 のまま
    int q[2];
    if (::pipe(q) != 0) {
        return 4;
    }
    if (::write(q[1], "e\n", 2) != 2) {
        return 5;
    }
    ::close(q[1]);
    local = 77;
    if (ilibc::fgets(line, 16, q[0], ilibc::clear_errno(local)) == nullptr) {
        return 6;
    }
    if (local != 0) {
        return 7;
    }
    ::close(q[0]);
    // (3) 省略版は clear_errno() と同じ実体 g_errno を書込先にする
    ILIBC_ERRNO = 0;
    if (ilibc::fgetc(p[0]) != -1) {
        return 8;
    }
    if (ILIBC_ERRNO == 0) {
        return 9;
    }
    // (4) nullptr 破棄は g_errno も触らない
    ILIBC_ERRNO = 0;
    if (ilibc::fgetc(p[0], nullptr) != -1) {
        return 10;
    }
    if (ILIBC_ERRNO != 0) {
        return 11;
    }
    return 0;
}

int main(void) {
    int nfail = 0;

    // 各CEの最初の失敗箇所を番号で報告する
    const int c1 = ce01();
    if (c1 == 0) {
        th_puts("CE-01 PASS\n");
    } else {
        th_puts("CE-01 FAIL case=");
        th_put_int(c1);
        th_putc('\n');
        ++nfail;
    }
    const int c2 = ce02();
    if (c2 == 0) {
        th_puts("CE-02 PASS\n");
    } else {
        th_puts("CE-02 FAIL case=");
        th_put_int(c2);
        th_putc('\n');
        ++nfail;
    }
    const int c3 = ce03();
    if (c3 == 0) {
        th_puts("CE-03 PASS\n");
    } else {
        th_puts("CE-03 FAIL case=");
        th_put_int(c3);
        th_putc('\n');
        ++nfail;
    }
    th_puts("SUMMARY pass=");
    th_put_int(3 - nfail);
    th_puts(" fail=");
    th_put_int(nfail);
    th_putc('\n');
    ilibc_test_exit((nfail == 0) ? 0 : 1);
}
