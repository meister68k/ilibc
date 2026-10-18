// cstdlib_host.cpp — <cstdlib>のホストlibc差分試験(H: TC-SL-12)と、
// _itoa_s のホスト往復/参照・書き込み範囲(H: TC-SL-23/24 のホスト分)。
// spec/ilibc_cstdlib_spec.md §9、spec/ilibc_test_spec.md §5.7(§5.7.2 比較規則)。
// ビルド: -m32 -static -DILIBC_HOST_TEST -U_FORTIFY_SOURCE。
// 本体は ilibc_under_test 名前空間、オラクルはホストlibcの実関数。
// <cstdio> はスタブされるため、素通りしてコンパイラ付属を使う
// (ヘッダ仕様 §7.3 S3。ホスト施設を使う試験コードだけの設定)。
#define ILIBC_STUB_PASSTHROUGH
#include <cstdio>  // 失敗の報告と snprintf(ホスト専用)
#include <cstdlib>  // 本ライブラリ(ILIBC_HOST_TEST で ilibc_under_test)

#include "../common/sl_cases.hpp"  // TC-SL-23/24 の共有本体(TST §5.2)

// 試験対象の名前空間(T1)
namespace t = ILIBC_NS;

// ホストlibcの4関数。
// ホストの <stdlib.h> を含めると本ライブラリの宣言と衝突するため、含めずに
// C の実関数を宣言して直接呼ぶ(オラクルはホストlibc、TST §5.7.1)。
extern "C" {
long strtol(const char *, char **, int);
unsigned long strtoul(const char *, char **, int);
int atoi(const char *);
long atol(const char *);
}

// ホストの errno。本ライブラリの <cerrno> は ILIBC_HOST_TEST で公開マクロを
// 定義しないため、glibc の実装(__errno_location)を直接参照して取得する。
extern "C" int *__errno_location(void);
#define HOST_ERRNO (*__errno_location())

// 失敗の記録(TST P8)
static int g_fail12 = 0;
static int g_printed = 0;
static unsigned g_seed_log = 0x12345678u;

static void report12(const char *what, int iter) {
    ++g_fail12;
    if (g_printed < 20) {
        ++g_printed;
        std::printf("TC-SL-12 FAIL what=%s iter=%d seed=0x%08x\n", what, iter, g_seed_log);
    }
}

// ポインタを先頭からのオフセットに変換する(nullptrは-1)(TST §5.7.2 #2)
template <class B, class P>
static long off(const B *base, const P *p) {
    return (p == nullptr) ? -1L : static_cast<long>(p - base);
}

// シードの解析(10進、または0x始まりの16進。範囲外は拒否)
static bool parse_seed(const char *s, unsigned *out) {
    unsigned v = 0;
    int base = 10;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        base = 16;
        s += 2;
    }
    if (*s == '\0') {
        return false;
    }
    for (; *s != '\0'; ++s) {
        int d;
        if (*s >= '0' && *s <= '9') {
            d = *s - '0';
        } else if (base == 16 && *s >= 'a' && *s <= 'f') {
            d = *s - 'a' + 10;
        } else if (base == 16 && *s >= 'A' && *s <= 'F') {
            d = *s - 'A' + 10;
        } else {
            return false;
        }
        if (v > (0xFFFFFFFFu - static_cast<unsigned>(d)) / static_cast<unsigned>(base)) {
            return false;  // オーバーフロー
        }
        v = v * static_cast<unsigned>(base) + static_cast<unsigned>(d);
    }
    *out = v;
    return true;
}

// --------------------------------------------------------- TC-SL-12 本体

// 1反復の差分試験: 乱数文字列 × 基数0、2〜36(無効基数はホストが EINVAL を
// 設けるため除外。TC-SL-12)。比較するのは戻り値、endptr のオフセット、
// ERANGE の有無(ホストは errno の設定の有無で判定)。
static void diff_iter(int iter) {
    // --- 入力: 数字・英字・符号・空白・x・X からなる乱数文字列 ---
    static const char charset[] = "0123456789abcdefABCDEF+- \txX";
    constexpr int kCharsetLen = static_cast<int>(sizeof(charset) - 1);  // NUL を除く
    char s[48];
    const int len = sl_cases::rnd_below(33);
    for (int i = 0; i < len; ++i) {
        s[i] = charset[sl_cases::rnd_below(kCharsetLen)];
    }
    s[len] = '\0';

    // --- 基数: 0、2〜36 ---
    int base = sl_cases::rnd_below(36);
    if (base == 1) {
        base = 2 + sl_cases::rnd_below(35);  // 1 を避ける(2..36)
    }

    // --- strtol: 値・endptr・ERANGE(err版でエラー出力先を明示) ---
    // 併せて、別名前空間の std:: ラッパ(t::)も同じ値/endptr を返す(TC-SL-12)
    {
        int our_err = 0;
        char *our_end = nullptr;
        const long our_v = ilibc::strtol(s, &our_end, base, &our_err);
        char *wrap_end = nullptr;
        ILIBC_ERRNO = 0;
        const long wrap_v = t::strtol(s, &wrap_end, base);
        if (wrap_v != our_v || off(s, wrap_end) != off(s, our_end)) {
            report12("strtol wrapper", iter);
        }
        HOST_ERRNO = 0;
        char *host_end = nullptr;
        const long host_v = ::strtol(s, &host_end, base);
        const bool host_set = (HOST_ERRNO != 0);
        const bool our_set = (our_err != 0);
        if (our_v != host_v) {
            report12("strtol value", iter);
        }
        if (off(s, our_end) != off(s, host_end)) {
            report12("strtol endptr", iter);
        }
        if (our_set != host_set) {
            report12("strtol erange", iter);
        }
    }
    // --- strtoul ---
    {
        int our_err = 0;
        char *our_end = nullptr;
        const unsigned long our_v = ilibc::strtoul(s, &our_end, base, &our_err);
        HOST_ERRNO = 0;
        char *host_end = nullptr;
        const unsigned long host_v = ::strtoul(s, &host_end, base);
        const bool host_set = (HOST_ERRNO != 0);
        const bool our_set = (our_err != 0);
        if (our_v != host_v) {
            report12("strtoul value", iter);
        }
        if (off(s, our_end) != off(s, host_end)) {
            report12("strtoul endptr", iter);
        }
        if (our_set != host_set) {
            report12("strtoul erange", iter);
        }
    }
    // --- atoi / atol(基数十進)。値と errno を比較 ---
    {
        int our_err = 0;
        const int our_v = ilibc::atoi(s, &our_err);
        HOST_ERRNO = 0;
        const int host_v = ::atoi(s);
        const bool host_set = (HOST_ERRNO != 0);
        if (our_v != host_v || (our_err != 0) != host_set) {
            report12("atoi", iter);
        }
        int our_err2 = 0;
        const long our_v2 = ilibc::atol(s, &our_err2);
        HOST_ERRNO = 0;
        const long host_v2 = ::atol(s);
        const bool host_set2 = (HOST_ERRNO != 0);
        if (our_v2 != host_v2 || (our_err2 != 0) != host_set2) {
            report12("atol", iter);
        }
    }
}

// TC-SL-12: 1000反復(シードはログに出す)
static int tc_sl12() {
    const int kIter = 1000;
    for (int i = 0; i < kIter; ++i) {
        diff_iter(i);
    }
    std::printf("seed=0x%08x iter=%d\n", g_seed_log, kIter);
    return g_fail12;
}

// ------------------------------------------------- TC-SL-23 ホスト分: snprintf

// 基数8、10、16 で、ホストの snprintf(%o、%d、%x)とも比較する
static int tc_sl23_snprintf() {
    static const unsigned long long bits[10] = {
        0ull, 1ull, 9ull, 10ull, 35ull, 36ull, 255ull, 256ull, 2147483647ull, 4294967295ull,
    };
    for (int i = 0; i < 10; ++i) {
        const unsigned long long bit = bits[i];
        const int value = static_cast<int>(bit);
        char got[40];
        char want[40];
        // %d(10進、符号付き)
        const int r10 = ilibc::_itoa_s(value, got, 40, 10);
        std::snprintf(want, 40, "%d", value);
        if (r10 != 0 || !sl_cases::str_same(got, want)) {
            return 1;
        }
        // %o / %x(8進・16進はビット列を符号なしで表示)
        const unsigned u = static_cast<unsigned>(bit);
        const int r8 = ilibc::_itoa_s(value, got, 40, 8);
        std::snprintf(want, 40, "%o", u);
        if (r8 != 0 || !sl_cases::str_same(got, want)) {
            return 2;
        }
        const int r16 = ilibc::_itoa_s(value, got, 40, 16);
        std::snprintf(want, 40, "%x", u);
        if (r16 != 0 || !sl_cases::str_same(got, want)) {
            return 3;
        }
    }
    return -1;
}

int main(int argc, char **argv) {
    // シード: 既定 0x12345678、argv[1] で上書き(ログには既定または指定値を出す)
    g_seed_log = 0x12345678u;
    if (argc > 1) {
        unsigned s = 0;
        if (!parse_seed(argv[1], &s)) {
            std::printf("bad seed: %s\n", argv[1]);
            return 2;
        }
        sl_cases::reseed(s);
        g_seed_log = s;
    }

    int nfail = 0;

    // TC-SL-12: ホストlibcとの差分
    const int f12 = tc_sl12();
    if (f12 == 0) {
        std::printf("TC-SL-12 PASS\n");
    } else {
        std::printf("TC-SL-12 FAIL count=%d\n", f12);
        ++nfail;
    }

    // TC-SL-23: 共有本体(参照・往復)+ snprintf ホスト分
    const int c23 = sl_cases::tc_sl23_first_failure();
    const int c23s = tc_sl23_snprintf();
    if (c23 < 0 && c23s < 0) {
        std::printf("TC-SL-23 PASS\n");
    } else {
        std::printf("TC-SL-23 FAIL case=%d snprintf=%d\n", c23, c23s);
        ++nfail;
    }

    // TC-SL-24: 書き込み範囲(共有本体)
    const int c24 = sl_cases::tc_sl24_first_failure();
    if (c24 < 0) {
        std::printf("TC-SL-24 PASS\n");
    } else {
        std::printf("TC-SL-24 FAIL case=%d\n", c24);
        ++nfail;
    }

    std::printf("SUMMARY pass=%d fail=%d\n", 3 - nfail, nfail);
    return (nfail == 0) ? 0 : 1;
}
