// cstring_runtime.cpp — <cstring>のE1実行時試験(R: TC-ST-11)。
// 先頭でブートストラップ(spec/ilibc_test_spec.md §5.8)を行い、mem* 5関数を
// 短い固定データで最小検査してから、TC-ST-07〜09を共有本体 str_cases.hpp で
// 実行する(spec/ilibc_cstring_spec.md §9)。
// データ設定・照合は volatile のループで行い、ライブラリに依存しない(R1/R4)。
#include "../../common/harness/th_output.hpp"
#include "../../common/str_cases.hpp"

// プラットフォーム層(§5.6.2)。putc は th_output.hpp が宣言する。
// [[noreturn]] の位置は e1_stdio.cpp の注記のとおり(GCCが受理する順)
extern "C" [[noreturn]] void ilibc_test_exit(int status);

// 試験対象の名前空間(T1)。E1 では std(=ilibc本体)。
namespace t = ILIBC_NS;

// volatile 配列を mem* の void* 引数にする(cast away volatile)
static void *as_void(volatile unsigned char *p) {
    return const_cast<unsigned char *>(p);
}
static const void *as_cvoid(const volatile unsigned char *p) {
    return const_cast<const unsigned char *>(p);
}

// volatile 配列2本の先頭16バイトを照合する(§5.8: volatile スカラー変数のループ)
static int vol_match(const volatile unsigned char *x, const volatile unsigned char *y) {
    int same = 1;
    for (int i = 0; i < 16; ++i) {
        volatile const unsigned char vx = x[i];
        volatile const unsigned char vy = y[i];
        if (vx != vy) {
            same = 0;
        }
    }
    return same;
}

// ブートストラップ(§5.8): mem* 5関数を、短い固定データ(16バイト)で検査する。
// 集成体初期化を使わず、データは1バイトずつ設定する(R4)。
// 成功で1、失敗で0を返す(呼び出し側が BOOTSTRAP FAIL・終了コード2で止める)。
static int bootstrap(void) {
    volatile unsigned char a[16];
    volatile unsigned char b[16];
    volatile unsigned char c[16];
    volatile unsigned char w[16];
    for (int i = 0; i < 16; ++i) {
        a[i] = static_cast<unsigned char>(i * 7 + 1);
        b[i] = static_cast<unsigned char>(0x80 + i);
        c[i] = 0xA5;
        w[i] = 0xA5;
    }

    // (1) memset: a[0,12) = 0x3C、カナリア a[12,16) は初期パターンのまま
    t::memset(as_void(a), 0x3C, 12);
    for (int i = 0; i < 16; ++i) {
        w[i] = static_cast<unsigned char>((i < 12) ? 0x3C : (i * 7 + 1));
    }
    if (!vol_match(a, w)) {
        return 0;
    }

    // (2) memcpy: b[0,12) を a に複製、カナリアは維持
    for (int i = 12; i < 16; ++i) {
        a[i] = 0xA5;
    }
    t::memcpy(as_void(a), as_cvoid(b), 12);
    for (int i = 0; i < 16; ++i) {
        w[i] = static_cast<unsigned char>((i < 12) ? (0x80 + i) : 0xA5);
    }
    if (!vol_match(a, w)) {
        return 0;
    }

    // (3) memmove 前方重複: a[0,10) を a[2,12) へ。結果は退避 c から作る
    for (int i = 0; i < 16; ++i) {
        c[i] = a[i];
    }
    t::memmove(as_void(a + 2), as_cvoid(a), 10);
    for (int i = 0; i < 16; ++i) {
        if (i < 2) {
            w[i] = c[i];
        } else if (i < 12) {
            w[i] = c[i - 2];
        } else {
            w[i] = 0xA5;
        }
    }
    if (!vol_match(a, w)) {
        return 0;
    }

    // (4) memmove 後方重複: a[2,12) を a[0,10) へ
    for (int i = 0; i < 16; ++i) {
        c[i] = a[i];
    }
    t::memmove(as_void(a), as_cvoid(a + 2), 10);
    for (int i = 0; i < 16; ++i) {
        if (i < 10) {
            w[i] = c[i + 2];
        } else if (i < 12) {
            w[i] = c[i];
        } else {
            w[i] = 0xA5;
        }
    }
    if (!vol_match(a, w)) {
        return 0;
    }

    // (5) memcmp: 同一なら0、退避前の中身(c)とは1バイトでも異なる
    const volatile int r_eq = t::memcmp(as_cvoid(a), as_cvoid(a), 12);
    const volatile int r_ne = t::memcmp(as_cvoid(a), as_cvoid(c), 12);
    if (r_eq != 0) {
        return 0;
    }
    if (r_ne == 0) {
        return 0;
    }

    // (6) memchr: 既知の値を設定して、位置11の発見・不発見・n=0 を検査する
    for (int i = 0; i < 16; ++i) {
        a[i] = static_cast<unsigned char>((i < 12) ? (i * 3 + 5) : 0xA5);
    }
    const void *p = t::memchr(as_cvoid(a), 0x26, 12);  // a[11] = 11*3+5 = 0x26
    if (p == nullptr) {
        return 0;
    }
    if ((static_cast<const volatile unsigned char *>(p) - a) != 11) {
        return 0;
    }
    if (t::memchr(as_cvoid(a), 0x19, 12) != nullptr) {  // 0x19 は範囲に無い
        return 0;
    }
    if (t::memchr(as_cvoid(a), 0x26, 0) != nullptr) {  // n=0 は必ず不発見
        return 0;
    }
    return 1;
}

int main(void) {
    // まずブートストラップ(§5.8)。失敗したら以降の実行時試験は行わない
    if (!bootstrap()) {
        th_puts("BOOTSTRAP FAIL\n");
        ilibc_test_exit(2);
    }

    // TC-ST-07〜09: ホスト差分と同じ共有本体(H: TST §5.2 単一ソース)
    const int c7 = str_cases::tc_st07_first_failure();
    const int c8 = str_cases::tc_st08_first_failure();
    const int c9 = str_cases::tc_st09_first_failure();
    int nfail = 0;
    if (c7 < 0) {
        th_puts("TC-ST-07 PASS\n");
    } else {
        th_puts("TC-ST-07 FAIL str_cases.hpp case=");
        th_put_int(c7);
        th_putc('\n');
        ++nfail;
    }
    if (c8 < 0) {
        th_puts("TC-ST-08 PASS\n");
    } else {
        th_puts("TC-ST-08 FAIL str_cases.hpp case=");
        th_put_int(c8);
        th_putc('\n');
        ++nfail;
    }
    if (c9 < 0) {
        th_puts("TC-ST-09 PASS\n");
    } else {
        th_puts("TC-ST-09 FAIL str_cases.hpp case=");
        th_put_int(c9);
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
