// sl_cases.hpp — <cstdlib> の実行時試験本体(err版規則・errno・基数・空白・
// 範囲境界・_itoa_s の参照/往復/カナリア)。ホスト差分(H)と E1 実行(R)で
// 共有する(TST §5.2 単一ソース)。
// spec/ilibc_cstdlib_spec.md §9 TC-SL-06〜11、22〜24、spec/ilibc_test_spec.md §5.2/§5.7。
// 出力は含めない(T5)。各関数は最初の失敗の事例番号を返し、-1 は合格。
// ライブラリの関数には volatile 経由の値を渡し、最適化による畳み込みを避ける(T3)。
#pragma once

#include <cstdlib>
#include <climits>
#include <ilibc/stdlib.hpp>

namespace sl_cases {

// 試験対象の名前空間(std、またはホスト差分の ilibc_under_test)(T1)
namespace t = ILIBC_NS;
// 拡張名前空間(err版と _itoa_s。ホスト差分でも名前は変わらない)
namespace e = ilibc;

// 実行時に値を隠し、最適化による畳み込みを防ぐ(T3)
inline int vol(int x) {
    volatile int v = x;
    return v;
}

// --------------------------------------------------------------- 入力生成

// xorshift32乱数(TST §5.7.1。既定シードは 0x12345678、reseed で上書き可)
inline unsigned g_rng = 0x12345678u;
inline void reseed(unsigned s) { g_rng = (s != 0u) ? s : 1u; }
inline unsigned rnd() {
    unsigned x = g_rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    g_rng = x;
    return x;
}

// 0以上 n 未満(n > 0)
inline int rnd_below(int n) {
    return static_cast<int>(rnd() % static_cast<unsigned>(n));
}

// 64ビット整数 v を radix 進数の文字列 out にする(実装とは独立な参照:
// 桁を下位から取り出して並べ替える。TC-SL-11/23)。長さを返す(終端を除く)
inline int ref_ulltoa(unsigned long long v, int radix, char *out) {
    char tmp[72];  // 64bit を基数2で展開しても64桁
    int n = 0;
    if (v == 0ull) {
        tmp[n] = '0';
        ++n;
    }
    while (v != 0ull) {
        const int d = static_cast<int>(v % static_cast<unsigned long long>(radix));
        tmp[n] = static_cast<char>(d < 10 ? '0' + d : 'a' + (d - 10));
        ++n;
        v /= static_cast<unsigned long long>(radix);
    }
    for (int i = 0; i < n; ++i) {
        out[i] = tmp[n - 1 - i];
    }
    out[n] = '\0';
    return n;
}

// ------------------------------------------------- TC-SL-06: err版の規則

// (a)成功時に *err を書かない (b)範囲外で ERANGE (c)err が nullptr でも動作
// (d)errno(ILIBC_ERRNO)を変更しない
inline int tc_sl06_first_failure() {
    // (a): 番兵値が成功後も不変(strtol/strtoul/atoi/atol すべて)
    {
        const int sentinel = 0x12345678;
        int err = sentinel;
        char *end = nullptr;
        char s[4] = { '4', '2', '\0', '\0' };
        const int base = vol(10);
        const long v = e::strtol(s, &end, base, &err);
        if (v != 42L || err != sentinel || end != s + 2) {
            return 1;
        }
        err = sentinel;
        const unsigned long u = e::strtoul(s, &end, base, &err);
        if (u != 42ul || err != sentinel) {
            return 2;
        }
        err = sentinel;
        if (e::atoi(s, &err) != 42 || err != sentinel) {
            return 3;
        }
        err = sentinel;
        if (e::atol(s, &err) != 42L || err != sentinel) {
            return 4;
        }
    }
    // (b): 範囲外で *err == ERANGE(結果は LONG_MAX / ULONG_MAX)
    {
        int err = 0;
        char s[12] = { '2', '1', '4', '7', '4', '8', '3', '6', '4', '8', '\0', '\0' };  // LONG_MAX+1
        char *end = nullptr;
        const int base = vol(10);
        if (e::strtol(s, &end, base, &err) != LONG_MAX || err != ILIBC_ERANGE || end != s + 10) {
            return 5;
        }
        err = 0;
        char u[12] = { '4', '2', '9', '4', '9', '6', '7', '2', '9', '6', '\0', '\0' };  // ULONG_MAX+1
        if (e::strtoul(u, &end, base, &err) != ULONG_MAX || err != ILIBC_ERANGE) {
            return 6;
        }
        err = 0;
        if (e::atoi(s, &err) != INT_MAX || err != ILIBC_ERANGE) {
            return 7;
        }
        err = 0;
        char n[12] = { '-', '2', '1', '4', '7', '4', '8', '3', '6', '4', '9', '\0' };  // LONG_MIN-1
        if (e::atol(n, &err) != LONG_MIN || err != ILIBC_ERANGE) {
            return 8;
        }
    }
    // (c): err が nullptr でも結果は得られ、範囲外の報告は捨てる
    {
        char s[12] = { '2', '1', '4', '7', '4', '8', '3', '6', '4', '8', '\0', '\0' };
        char *end = nullptr;
        const int base = vol(10);
        if (e::strtol(s, &end, base, nullptr) != LONG_MAX) {
            return 9;
        }
        if (e::strtoul(s, nullptr, base, nullptr) != 2147483648ul) {  // 値としては範囲内
            return 10;
        }
        if (e::atoi(s, nullptr) != INT_MAX) {
            return 11;
        }
        if (e::atol(s, nullptr) != LONG_MAX) {
            return 12;
        }
    }
    // (d): errno は err版では変更しない(番兵値を設定して全経路を通過)
    {
        ILIBC_ERRNO = 0x0BADF00D;
        int err = 0;
        char ok[4] = { '4', '2', '\0', '\0' };
        char ovf[12] = { '2', '1', '4', '7', '4', '8', '3', '6', '4', '8', '\0', '\0' };
        const int base = vol(10);
        (void)e::strtol(ok, nullptr, base, &err);
        (void)e::strtol(ovf, nullptr, base, &err);
        (void)e::strtoul(ok, nullptr, base, &err);
        (void)e::strtoul(ovf, nullptr, base, nullptr);
        (void)e::atoi(ok, &err);
        (void)e::atoi(ovf, nullptr);
        (void)e::atol(ok, &err);
        (void)e::atol(ovf, nullptr);
        if (ILIBC_ERRNO != 0x0BADF00D) {
            return 13;
        }
    }
    return -1;
}

// ------------------------------------------------- TC-SL-07: std::版のerrno

// (a)成功時に errno 不変 (b)範囲外で ERANGE (c)atoi と strtol の等価
inline int tc_sl07_first_failure() {
    // (a): 成功経路で errno が番兵のまま
    {
        ILIBC_ERRNO = 0x0BADF00D;
        char ok[4] = { '4', '2', '\0', '\0' };
        const int base = vol(10);
        char *end = nullptr;
        if (t::strtol(ok, &end, base) != 42L || ILIBC_ERRNO != 0x0BADF00D || end != ok + 2) {
            return 1;
        }
        if (t::strtoul(ok, nullptr, base) != 42ul || ILIBC_ERRNO != 0x0BADF00D) {
            return 2;
        }
        if (t::atoi(ok) != 42 || ILIBC_ERRNO != 0x0BADF00D) {
            return 3;
        }
        if (t::atol(ok) != 42L || ILIBC_ERRNO != 0x0BADF00D) {
            return 4;
        }
    }
    // (b): 範囲外の経路で ERANGE
    {
        ILIBC_ERRNO = 0;
        char ovf[12] = { '2', '1', '4', '7', '4', '8', '3', '6', '4', '8', '\0', '\0' };
        const int base = vol(10);
        if (t::strtol(ovf, nullptr, base) != LONG_MAX || ILIBC_ERRNO != ILIBC_ERANGE) {
            return 5;
        }
        ILIBC_ERRNO = 0;
        char ovfu[12] = { '4', '2', '9', '4', '9', '6', '7', '2', '9', '6', '\0', '\0' };
        if (t::strtoul(ovfu, nullptr, base) != ULONG_MAX || ILIBC_ERRNO != ILIBC_ERANGE) {
            return 6;
        }
        ILIBC_ERRNO = 0;
        if (t::atoi(ovf) != INT_MAX || ILIBC_ERRNO != ILIBC_ERANGE) {
            return 7;
        }
        ILIBC_ERRNO = 0;
        char ovfn[12] = { '-', '2', '1', '4', '7', '4', '8', '3', '6', '4', '9', '\0' };
        if (t::atol(ovfn) != LONG_MIN || ILIBC_ERRNO != ILIBC_ERANGE) {
            return 8;
        }
    }
    // (c): atoi(s) と (int)strtol(s, nullptr, 10) が戻り値と errno の両方で一致
    // (仕様の例と、乱数入力)
    {
        static const char *const examples[] = {
            "123", "  -42xyz", "+5", "0x1F", "017", "019", "z", "", "abc", "-",
            "- 1", "2147483647", "2147483648", "-2147483648", "-2147483649",
            "99999999999999", "+0", "-0", "0000000000000000000009",
        };
        const int nex = static_cast<int>(sizeof examples / sizeof examples[0]);
        for (int i = 0; i < nex; ++i) {
            char buf[40];
            int n = 0;
            while (examples[i][n] != '\0') {
                buf[n] = examples[i][n];
                ++n;
            }
            buf[n] = '\0';
            ILIBC_ERRNO = 0;
            const int va = t::atoi(buf);
            const int ea = ILIBC_ERRNO;
            ILIBC_ERRNO = 0;
            const long vs = t::strtol(buf, nullptr, vol(10));
            const int es = ILIBC_ERRNO;
            if (va != static_cast<int>(vs) || ea != es) {
                return 9;
            }
        }
        // 乱数入力: 数字・英字・符号・空白・x・X からなる文字列
        static const char charset[] = "0123456789+- abcxyzXYZxX";
        for (int iter = 0; iter < 500; ++iter) {
            char buf[32];
            const int n = rnd_below(25);
            for (int i = 0; i < n; ++i) {
                buf[i] = charset[rnd_below(23)];
            }
            buf[n] = '\0';
            ILIBC_ERRNO = 0;
            const int va = t::atoi(buf);
            const int ea = ILIBC_ERRNO;
            ILIBC_ERRNO = 0;
            const long vs = t::strtol(buf, nullptr, vol(10));
            const int es = ILIBC_ERRNO;
            if (va != static_cast<int>(vs) || ea != es) {
                return 10;
            }
        }
    }
    return -1;
}

// ------------------------------------------------- TC-SL-08: 基数(実行時)

// base=2〜36 の各基数で基数以上の数字で停止、大小文字非区別、接頭辞の扱い
inline int tc_sl08_first_failure() {
    // (a): 各基数で "1"+基数 の数字 の2文字目で停止する
    for (int base = 2; base <= 36; ++base) {
        char s[3];
        s[0] = '1';
        s[1] = static_cast<char>(base < 10 ? '0' + base : 'a' + (base - 10));
        s[2] = '\0';
        int err = 0;
        char *end = nullptr;
        const int b = vol(base);
        if (e::strtol(s, &end, b, &err) != 1L || end != s + 1 || err != 0) {
            return 1;
        }
    }
    // (b): 英字の大文字・小文字を区別しない(base=16: "AbC"、base=36: "Z")
    {
        char s1[4] = { 'A', 'b', 'C', '\0' };
        char s2[2] = { 'Z', '\0' };
        int err = 0;
        char *end = nullptr;
        if (e::strtol(s1, &end, vol(16), &err) != 0xABCL || end != s1 + 3 || err != 0) {
            return 2;
        }
        if (e::strtol(s2, &end, vol(36), &err) != 35L || end != s2 + 1 || err != 0) {
            return 3;
        }
    }
    // (c)/(d): 接頭辞 0x/0X(base 0/16)、"0x"単独/"0xg" は "0" のみ、
    // base=8/10 では "0x" は接頭辞にならない
    {
        char p1[5] = { '0', 'x', '1', 'F', '\0' };
        char p2[5] = { '0', 'X', '1', 'F', '\0' };
        char p3[3] = { '0', 'x', '\0' };
        char p4[4] = { '0', 'x', 'g', '\0' };
        char p5[5] = { '0', 'x', '1', 'g', '\0' };
        char p6[4] = { '0', '1', '7', '\0' };
        int err = 0;
        char *end = nullptr;
        if (e::strtol(p1, &end, vol(0), &err) != 31L || end != p1 + 4) {
            return 4;
        }
        if (e::strtol(p2, &end, vol(16), &err) != 31L || end != p2 + 4) {
            return 5;
        }
        if (e::strtol(p3, &end, vol(16), &err) != 0L || end != p3 + 1) {
            return 6;
        }
        if (e::strtol(p4, &end, vol(0), &err) != 0L || end != p4 + 1) {
            return 7;
        }
        if (e::strtol(p5, &end, vol(0), &err) != 1L || end != p5 + 3) {
            return 8;
        }
        if (e::strtol(p6, &end, vol(0), &err) != 15L || end != p6 + 3) {
            return 9;
        }
        if (e::strtol(p1, &end, vol(8), &err) != 0L || end != p1 + 1) {
            return 10;
        }
        if (e::strtol(p1, &end, vol(10), &err) != 0L || end != p1 + 1) {
            return 11;
        }
    }
    return -1;
}

// ------------------------------------------------- TC-SL-09: 無効な基数(実行時)

// 無効な基数で 0・*endptr==nptr・エラーなし(D-L1)
inline int tc_sl09_first_failure() {
    const int bases[5] = { -1, 1, 37, INT_MIN, INT_MAX };
    char s[3] = { '4', '2', '\0' };
    for (int i = 0; i < 5; ++i) {
        int err = 0;
        char *end = nullptr;
        const int b = vol(bases[i]);
        if (e::strtol(s, &end, b, &err) != 0L || end != s || err != 0) {
            return i + 1;
        }
        err = 0;
        end = nullptr;
        if (e::strtoul(s, &end, b, &err) != 0ul || end != s || err != 0) {
            return i + 6;
        }
    }
    return -1;
}

// ------------------------------------------------- TC-SL-10: 空白・符号(実行時)

inline int tc_sl10_first_failure() {
    // 空白類6文字(0x20、0x09〜0x0D)が読み飛ばされる
    const char ws[6] = { ' ', '\t', '\n', '\v', '\f', '\r' };
    for (int i = 0; i < 6; ++i) {
        char s[4];
        s[0] = ws[i];
        s[1] = '4';
        s[2] = '2';
        s[3] = '\0';
        int err = 0;
        char *end = nullptr;
        if (e::strtol(s, &end, vol(10), &err) != 42L || end != s + 3 || err != 0) {
            return i + 1;
        }
    }
    // 0x1C〜0x1F は空白ではない(変換なし)
    const char ctrl[2] = { 0x1C, '\0' };
    {
        char s[4] = { ctrl[0], '4', '2', '\0' };
        int err = 0;
        char *end = nullptr;
        if (e::strtol(s, &end, vol(10), &err) != 0L || end != s || err != 0) {
            return 7;
        }
        s[0] = 0x1F;
        if (e::strtol(s, &end, vol(10), &err) != 0L || end != s || err != 0) {
            return 8;
        }
    }
    // 変換なし: "+", "-", "+-1", "- 1", 符号の後の空白
    static const char *const noconv[] = { "+", "-", "+-1", "- 1", "+\t1", "-  1" };
    for (int i = 0; i < 6; ++i) {
        char buf[8];
        int n = 0;
        while (noconv[i][n] != '\0') {
            buf[n] = noconv[i][n];
            ++n;
        }
        buf[n] = '\0';
        int err = 0;
        char *end = nullptr;
        if (e::strtol(buf, &end, vol(10), &err) != 0L || end != buf || err != 0) {
            return 9 + i;
        }
    }
    return -1;
}

// ------------------------------------------------- TC-SL-11: 範囲の境界

// 各基数(0、2〜36)で LONG_MAX、LONG_MAX+1、LONG_MIN、LONG_MIN-1、
// ULONG_MAX、ULONG_MAX+1 の文字列を64ビット参照で生成して検証する。
// 範囲外の後も endptr が有効な数字の終端を指す。
inline int tc_sl11_first_failure() {
    for (int bi = 0; bi < 36; ++bi) {
        const int base = (bi == 0) ? 0 : bi + 1;  // 0, 2..36
        // base=0 は10進の数字列(先頭0なし)で解釈されるため、展開は10進で行う
        const int gen_base = (base == 0) ? 10 : base;
        const unsigned long long long_max = 2147483647ull;
        const unsigned long long ulong_max = 4294967295ull;

        // 正の境界: LONG_MAX(範囲内)、LONG_MAX+1(範囲外)
        {
            char s[72];
            ref_ulltoa(long_max, gen_base, s);
            int err = 0;
            char *end = nullptr;
            const int bl = vol(base);
            if (e::strtol(s, &end, bl, &err) != static_cast<long>(long_max) || err != 0) {
                return bi + 1;
            }
            const int nmax = ref_ulltoa(long_max + 1ull, gen_base, s);
            err = 0;
            const long r = e::strtol(s, &end, bl, &err);
            if (r != LONG_MAX || err != ILIBC_ERANGE || end != s + nmax) {
                return bi + 2;
            }
            // strtoul: ULONG_MAX まで範囲内、ULONG_MAX+1 で範囲外
            const int num = ref_ulltoa(ulong_max, gen_base, s);
            err = 0;
            if (e::strtoul(s, &end, bl, &err) != ULONG_MAX || err != 0 || end != s + num) {
                return bi + 3;
            }
            const int nover = ref_ulltoa(ulong_max + 1ull, gen_base, s);
            err = 0;
            if (e::strtoul(s, &end, bl, &err) != ULONG_MAX || err != ILIBC_ERANGE
                || end != s + nover) {
                return bi + 4;
            }
        }
        // 負の境界: LONG_MIN(範囲内)、LONG_MIN-1(範囲外)
        {
            char s[72];
            s[0] = '-';
            const int n1 = ref_ulltoa(2147483648ull, gen_base, s + 1);  // |LONG_MIN|
            int err = 0;
            char *end = nullptr;
            const int bl = vol(base);
            if (e::strtol(s, &end, bl, &err) != LONG_MIN || err != 0 || end != s + 1 + n1) {
                return bi + 5;
            }
            const int n2 = ref_ulltoa(2147483649ull, gen_base, s + 1);  // |LONG_MIN|+1
            err = 0;
            if (e::strtol(s, &end, bl, &err) != LONG_MIN || err != ILIBC_ERANGE
                || end != s + 1 + n2) {
                return bi + 6;
            }
            // strtoul の負号は2の補数の否定: "-1" は ULONG_MAX で範囲外ではない
            char m1[3] = { '-', '1', '\0' };
            err = 0;
            if (e::strtoul(m1, &end, bl, &err) != ULONG_MAX || err != 0 || end != m1 + 2) {
                return bi + 7;
            }
        }
    }
    return -1;
}

// ------------------------------------------------- TC-SL-22: errno 不変

// 成功とすべての失敗(nullptr、size0、基数の誤り、バッファ不足)で errno が
// 変化しない。戻り値は 0 / ILIBC_EINVAL / ILIBC_ERANGE のいずれか。
inline int tc_sl22_first_failure() {
    char buf[12];
    int i;
    for (i = 0; i < 12; ++i) {
        buf[i] = 'Q';
    }
    const int sentinel = 0x0BADF00D;

    // 成功
    ILIBC_ERRNO = sentinel;
    int r = e::_itoa_s(vol(-123), buf, 12, vol(10));
    if (r != 0 || ILIBC_ERRNO != sentinel || buf[0] != '-') {
        return 1;
    }
    // nullptr
    ILIBC_ERRNO = sentinel;
    r = e::_itoa_s(10, static_cast<char *>(nullptr), 8, vol(10));
    if (r != ILIBC_EINVAL || ILIBC_ERRNO != sentinel) {
        return 2;
    }
    // size が 0
    ILIBC_ERRNO = sentinel;
    r = e::_itoa_s(10, buf, 0, vol(10));
    if (r != ILIBC_EINVAL || ILIBC_ERRNO != sentinel) {
        return 3;
    }
    // 基数の誤り
    ILIBC_ERRNO = sentinel;
    r = e::_itoa_s(10, buf, 12, vol(1));
    if (r != ILIBC_EINVAL || ILIBC_ERRNO != sentinel || buf[0] != '\0') {
        return 4;
    }
    // バッファ不足
    ILIBC_ERRNO = sentinel;
    r = e::_itoa_s(-123, buf, 4, vol(10));  // "-123" は size 5 必要
    if (r != ILIBC_ERANGE || ILIBC_ERRNO != sentinel || buf[0] != '\0') {
        return 5;
    }
    return -1;
}

// ------------------------------------------------- TC-SL-23: 参照と往復

// got と want が同一文字列か(終端を含む)
inline bool str_same(const char *a, const char *b) {
    for (int k = 0; a[k] != '\0' || b[k] != '\0'; ++k) {
        if (a[k] != b[k]) {
            return false;
        }
    }
    return true;
}

// 1値分の変換・参照比較・往復・文字数を検査する(radix は 2〜36、value は
// 32bit ビット列)。失敗種別を返し、-1 は合格。
inline int sl23_check(int value, int radix) {
    const unsigned long long bit = static_cast<unsigned long long>(
        static_cast<unsigned long>(value));  // C++20: 負値は2の補数のビット列
    char got[40];
    char want[48];
    int woff = 0;
    unsigned long absu;
    // 基数10の負値だけ '-' + 絶対値。それ以外はビット列のまま展開
    if (radix == 10 && value < 0) {
        want[0] = '-';
        woff = 1;
        absu = (value == INT_MIN) ? 2147483648ul
                                  : static_cast<unsigned long>(-static_cast<long>(value));
    } else {
        absu = static_cast<unsigned long>(value);
    }
    ref_ulltoa(absu, radix, want + woff);
    const int r = e::_itoa_s(value, got, 40, vol(radix));
    if (r != 0) {
        return 1;
    }
    if (!str_same(got, want)) {
        return 2;
    }
    // 往復: 基数10 は strtol、それ以外は strtoul で元のビット列に戻る
    if (radix == 10) {
        const long back = e::strtol(got, nullptr, vol(10), nullptr);
        if (static_cast<unsigned long>(back) != static_cast<unsigned long>(bit)) {
            return 3;
        }
    } else {
        const unsigned long back = e::strtoul(got, nullptr, vol(radix), nullptr);
        if (back != static_cast<unsigned long>(bit)) {
            return 4;
        }
    }
    // (c) 文字数が33以下(基数2の32桁+終端が最大)
    int len = 0;
    while (got[len] != '\0') {
        ++len;
    }
    if (len > 33) {
        return 5;
    }
    return -1;
}

// 基数2〜36の各基数で、境界値・-2000〜2000・乱数10万個を検証する
inline int tc_sl23_first_failure() {
    for (int radix = 2; radix <= 36; ++radix) {
        // (a) 境界値: 0、±1、基数^n の前後、32bit のビット限界の前後
        unsigned long bounds[24];
        int nb = 0;
        bounds[nb++] = 0ul;
        bounds[nb++] = 1ul;
        bounds[nb++] = static_cast<unsigned long>(radix) - 1ul;
        bounds[nb++] = static_cast<unsigned long>(radix);
        bounds[nb++] = 2147483647ul;  // LONG_MAX / 正の最大
        bounds[nb++] = 2147483648ul;  // |LONG_MIN| のビット列(INT_MIN)
        bounds[nb++] = 4294967294ul;  // ULONG_MAX-1
        bounds[nb++] = 4294967295ul;  // ULONG_MAX
        {
            unsigned long p = 1ul;
            while (p < 2147483648ul / static_cast<unsigned long>(radix) && nb < 22) {
                p *= static_cast<unsigned long>(radix);
                bounds[nb++] = p;
                if (nb < 22) {
                    bounds[nb++] = p - 1ul;
                }
            }
        }
        for (int i = 0; i < nb; ++i) {
            const int c = sl23_check(static_cast<int>(bounds[i]), radix);
            if (c >= 0) {
                return radix * 10 + c;
            }
        }
        // (a) -2000〜2000 の全値
        for (int v = -2000; v <= 2000; ++v) {
            const int c = sl23_check(v, radix);
            if (c >= 0) {
                return 1000 + radix * 10 + c;
            }
        }
        // (a) 乱数 10万個(基数ごと。シードは共有生成器)
        for (int iter = 0; iter < 100000; ++iter) {
            const int value = static_cast<int>(rnd());
            const int c = sl23_check(value, radix);
            if (c >= 0) {
                return 2000 + radix * 10 + c;
            }
        }
    }
    return -1;
}


// ------------------------------------------------- TC-SL-24: 書き込み範囲

// size が必要な大きさちょうど、+1、+8 のとき、前後のカナリア(0xA5)が壊れない。
// 失敗時に書き込まれるのは buffer[0] のみ(nullptr と size0 では何も書かれない)。
inline int tc_sl24_first_failure() {
    // 必要な大きさ: "-2147483648" は12(11文字+終端)
    static const int sizes[3] = { 12, 13, 20 };  // ちょうど、+1、+8
    for (int si = 0; si < 3; ++si) {
        const int size = sizes[si];
        unsigned char buf[32];
        int pre = 4;
        int i;
        for (i = 0; i < 32; ++i) {
            buf[i] = 0xA5;
        }
        char *p = reinterpret_cast<char *>(buf + pre);
        // 成功: 全体(前後カナリア含む)が不変、結果は "-2147483648"
        const int r = e::_itoa_s(INT_MIN, p, static_cast<std::size_t>(size), vol(10));
        if (r != 0) {
            return 1;
        }
        for (i = 0; i < pre; ++i) {
            if (buf[i] != 0xA5) {
                return 2;
            }
        }
        static const char want[12] = { '-', '2', '1', '4', '7', '4', '8', '3',
                                       '6', '4', '8', '\0' };
        for (i = 0; i < 12; ++i) {
            if (p[i] != want[i]) {
                return 3;
            }
        }
        for (i = pre + size; i < 32; ++i) {
            if (buf[i] != 0xA5) {
                return 4;
            }
        }
    }
    // 失敗(size 不足): buffer[0] のみ '\0'、他とカナリアは不変
    {
        unsigned char buf[32];
        int i;
        for (i = 0; i < 32; ++i) {
            buf[i] = 0xA5;
        }
        char *p = reinterpret_cast<char *>(buf + 4);
        const int r = e::_itoa_s(INT_MIN, p, 11, vol(10));  // 12必要 → ERANGE
        if (r != ILIBC_ERANGE || p[0] != '\0') {
            return 5;
        }
        for (i = 0; i < 4; ++i) {
            if (buf[i] != 0xA5) {
                return 6;
            }
        }
        for (i = 5; i < 32; ++i) {
            if (buf[i] != 0xA5) {
                return 7;
            }
        }
    }
    // nullptr / size0: 何も書かれない(バッファは触れない)
    {
        unsigned char buf[8];
        int i;
        for (i = 0; i < 8; ++i) {
            buf[i] = 0xA5;
        }
        if (e::_itoa_s(10, static_cast<char *>(nullptr), 8, vol(10)) != ILIBC_EINVAL) {
            return 8;
        }
        if (e::_itoa_s(10, reinterpret_cast<char *>(buf), 0, vol(10)) != ILIBC_EINVAL) {
            return 9;
        }
        for (i = 0; i < 8; ++i) {
            if (buf[i] != 0xA5) {
                return 10;
            }
        }
    }
    return -1;
}

}  // namespace sl_cases
