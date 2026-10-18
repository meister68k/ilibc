// cstdlib.cpp — <cstdlib>のコンパイル時試験(S)。
// spec/ilibc_cstdlib_spec.md §9 TC-SL-01、03、04、08〜10、18〜20 と
// spec/ilibc_test_spec.md §5.3。err版(ilibc::)と拡張関数(ilibc::_itoa_s)は
// <cstdlib>が取り込む ilibc/stdlib.hpp で可視になる。
// 期待値はすべて実装と独立に手書きした値である(TC-SL-18)。
// 関数ごとに別の static_assert にして、失敗箇所を絞る(§5.3)。
#include <cstdlib>
#include <type_traits>
#include <ilibc/config.hpp>

// 試験対象の名前空間(std、またはホスト差分の ilibc_under_test)(T1)
namespace t = ILIBC_NS;
// 拡張名前空間(err版と _itoa_s。ホスト差分でも名前は変わらない)
namespace e = ilibc;

// 文字列 s の長さ(終端を除く)
constexpr std::size_t sl_len(const char *s) {
    std::size_t n = 0;
    while (s[n] != '\0') { ++n; }
    return n;
}

// ------------------------------------------------- TC-SL-01: abs / labs

// abs の基本値(0、±1、INT_MAX、-INT_MAX)。INT_MIN は定数式で未定義のため含めない
static_assert(t::abs(0) == 0, "TC-SL-01 abs(0)");
static_assert(t::abs(1) == 1, "TC-SL-01 abs(1)");
static_assert(t::abs(-1) == 1, "TC-SL-01 abs(-1)");
static_assert(t::abs(INT_MAX) == INT_MAX, "TC-SL-01 abs(INT_MAX)");
static_assert(t::abs(-INT_MAX) == INT_MAX, "TC-SL-01 abs(-INT_MAX)");

// labs の基本値(0、±1、LONG_MAX、-LONG_MAX)
static_assert(t::labs(0L) == 0L, "TC-SL-01 labs(0)");
static_assert(t::labs(1L) == 1L, "TC-SL-01 labs(1)");
static_assert(t::labs(-1L) == 1L, "TC-SL-01 labs(-1)");
static_assert(t::labs(LONG_MAX) == LONG_MAX, "TC-SL-01 labs(LONG_MAX)");
static_assert(t::labs(-LONG_MAX) == LONG_MAX, "TC-SL-01 labs(-LONG_MAX)");

// オーバーロード選択: int リテラル→int 戻り、long リテラル→long 戻り(TC-SL-01)
static_assert(std::is_same_v<decltype(t::abs(1)), int>, "TC-SL-01 abs(int)");
static_assert(std::is_same_v<decltype(t::abs(1L)), long>, "TC-SL-01 abs(long)");
static_assert(std::is_same_v<decltype(t::labs(1L)), long>, "TC-SL-01 labs type");

// 全関数が noexcept(SL-01〜SL-06 の共通規則)
static_assert(noexcept(t::abs(1)) && noexcept(t::abs(1L)) && noexcept(t::labs(1L)));

// ------------------------------------------------- TC-SL-03: err版の変換例

// strtol の例の1行(§4.2 の表)。want は戻り値、end_off は *endptr の位置
// (nptr からのオフセット)、want_erange で *err の有無を検証する。
// 成功時に *err を書かないことは、番兵 0 が不変であることから確認する。
template <std::size_t N>
constexpr bool sl03_case(const char (&s)[N], int base, long want, std::size_t end_off,
                         bool want_erange) {
    int err = 0;
    char *end = nullptr;
    const long v = e::strtol(s, &end, base, &err);
    return v == want && end == s + end_off && err == (want_erange ? ILIBC_ERANGE : 0);
}

// strtol の例の全行(§4.2 の表。LONG_MAX=2147483647)
static_assert(sl03_case("123", 10, 123, 3, false), "TC-SL-03 123");
static_assert(sl03_case("  -42xyz", 10, -42, 5, false), "TC-SL-03 -42xyz");
static_assert(sl03_case("+5", 10, 5, 2, false), "TC-SL-03 +5");
static_assert(sl03_case("0x1F", 0, 31, 4, false), "TC-SL-03 0x1F base0");
static_assert(sl03_case("0x1F", 16, 31, 4, false), "TC-SL-03 0x1F base16");
static_assert(sl03_case("0x", 16, 0, 1, false), "TC-SL-03 0x 単独");
static_assert(sl03_case("0xg", 0, 0, 1, false), "TC-SL-03 0xg");
static_assert(sl03_case("017", 0, 15, 3, false), "TC-SL-03 017");
static_assert(sl03_case("019", 0, 1, 2, false), "TC-SL-03 019");
static_assert(sl03_case("z", 36, 35, 1, false), "TC-SL-03 z base36");
static_assert(sl03_case("", 10, 0, 0, false), "TC-SL-03 empty");
static_assert(sl03_case("abc", 10, 0, 0, false), "TC-SL-03 abc");
static_assert(sl03_case("-", 10, 0, 0, false), "TC-SL-03 -");
static_assert(sl03_case("- 1", 10, 0, 0, false), "TC-SL-03 - 1");
static_assert(sl03_case("2147483647", 10, LONG_MAX, 10, false), "TC-SL-03 LONG_MAX");
static_assert(sl03_case("2147483648", 10, LONG_MAX, 10, true), "TC-SL-03 LONG_MAX+1");
static_assert(sl03_case("-2147483648", 10, LONG_MIN, 11, false), "TC-SL-03 LONG_MIN");
static_assert(sl03_case("-2147483649", 10, LONG_MIN, 11, true), "TC-SL-03 LONG_MIN-1");
static_assert(sl03_case("99999999999999", 10, LONG_MAX, 14, true), "TC-SL-03 桁あふれ");
static_assert(sl03_case("42", 1, 0, 0, false), "TC-SL-03 無効な基数1");

// strtoul の例の全行(§4.2 の表。 ULONG_MAX=4294967295)
template <std::size_t N>
constexpr bool sl03u_case(const char (&s)[N], int base, unsigned long want, std::size_t end_off,
                          bool want_erange) {
    int err = 0;
    char *end = nullptr;
    const unsigned long v = e::strtoul(s, &end, base, &err);
    return v == want && end == s + end_off && err == (want_erange ? ILIBC_ERANGE : 0);
}

static_assert(sl03u_case("4294967295", 10, 4294967295UL, 10, false), "TC-SL-03 ULONG_MAX");
static_assert(sl03u_case("4294967296", 10, 4294967295UL, 10, true), "TC-SL-03 ULONG_MAX+1");
static_assert(sl03u_case("-1", 10, 4294967295UL, 2, false), "TC-SL-03 strtoul -1");
static_assert(sl03u_case("-4294967295", 10, 1UL, 11, false), "TC-SL-03 strtoul -ULONG_MAX");
static_assert(sl03u_case("-4294967296", 10, 4294967295UL, 11, true), "TC-SL-03 strtoul -ULONG_MAX-1");

// atoi / atol の err版: strtol 基数10 と等価(§4.3)
static_assert(e::atoi("42", nullptr) == 42, "TC-SL-03 atoi");
static_assert(e::atoi("-42", nullptr) == -42, "TC-SL-03 atoi -42");
static_assert(e::atol("123456789", nullptr) == 123456789L, "TC-SL-03 atol");
// 範囲外も err版なら定数式で評価できる(§3.4)
static_assert(e::atoi("2147483648", nullptr) == INT_MAX, "TC-SL-03 atoi 範囲外");
static_assert(e::atol("-2147483649", nullptr) == LONG_MIN, "TC-SL-03 atol 範囲外");

// ------------------------------------------------- TC-SL-04: std::版(範囲内)

static_assert(t::atoi("42") == 42, "TC-SL-04 atoi");
static_assert(t::atoi("-42") == -42, "TC-SL-04 atoi -42");
static_assert(t::atol("123456789") == 123456789L, "TC-SL-04 atol");
static_assert(t::strtol("0x7f", nullptr, 0) == 127L, "TC-SL-04 strtol 0x7f");
static_assert(t::strtol("-2147483648", nullptr, 10) == LONG_MIN, "TC-SL-04 strtol LONG_MIN");
static_assert(t::strtoul("4294967295", nullptr, 10) == 4294967295UL, "TC-SL-04 strtoul");
static_assert(t::strtoul("-1", nullptr, 10) == 4294967295UL, "TC-SL-04 strtoul -1");

// ------------------------------------------------- TC-SL-08: 基数の扱い

// base=2〜36 の各基数で、基数以上の数字で止まる(値1、endptr は2文字目)
constexpr bool sl08_digit_stop() {
    for (int base = 2; base <= 36; ++base) {
        char s[3] = {};
        s[0] = '1';
        s[1] = static_cast<char>(base < 10 ? '0' + base : 'a' + (base - 10));
        s[2] = '\0';
        int err = 0;
        char *end = nullptr;
        if (e::strtol(s, &end, base, &err) != 1) { return false; }
        if (end != s + 1 || err != 0) { return false; }
    }
    return true;
}
static_assert(sl08_digit_stop(), "TC-SL-08 数字の停止");

// 英字の大文字・小文字を区別しない(base=16: "AbC"=0xABC、base=36: "Z"=35)
static_assert(sl03_case("AbC", 16, 0xABC, 3, false), "TC-SL-08 大小文字 base16");
static_assert(sl03_case("Z", 36, 35, 1, false), "TC-SL-08 大文字 base36");

// 接頭辞: 0x/0X(base 0 と 16)、先頭の 0(base 0 で8進)、"0x"単独/"0xg" は
// 接頭辞にせず "0" だけを変換する(§4.2 規則3)
static_assert(sl03_case("0X1F", 16, 31, 4, false), "TC-SL-08 0X1F");
static_assert(sl03_case("0x1g", 0, 1, 3, false), "TC-SL-08 0x1g");
// "0x" は base=8/10 では接頭辞にならない("0"のみ、endptr は x)
static_assert(sl03_case("0x1F", 8, 0, 1, false), "TC-SL-08 0x1F base8");
static_assert(sl03_case("0x1F", 10, 0, 1, false), "TC-SL-08 0x1F base10");

// ------------------------------------------------- TC-SL-09: 無効な基数

// 無効な基数(負、1、37以上、INT_MIN、INT_MAX): 0、*endptr==nptr、エラーなし(D-L1)
constexpr bool sl09_invalid() {
    const int bases[5] = { -1, 1, 37, INT_MIN, INT_MAX };
    const char s[3] = { '4', '2', '\0' };
    for (int i = 0; i < 5; ++i) {
        int err = 0;
        char *end = nullptr;
        if (e::strtol(s, &end, bases[i], &err) != 0) { return false; }
        if (end != s || err != 0) { return false; }
        int err2 = 0;
        char *end2 = nullptr;
        if (e::strtoul(s, &end2, bases[i], &err2) != 0UL) { return false; }
        if (end2 != s || err2 != 0) { return false; }
    }
    return true;
}
static_assert(sl09_invalid(), "TC-SL-09 無効な基数");

// ------------------------------------------------- TC-SL-10: 空白・符号

// 空白類6文字(0x20、0x09〜0x0D)が先頭で読み飛ばされる
constexpr bool sl10_ws() {
    const char ws[6] = { ' ', '\t', '\n', '\v', '\f', '\r' };
    for (int i = 0; i < 6; ++i) {
        char s[4] = {};
        s[0] = ws[i];
        s[1] = '4';
        s[2] = '2';
        s[3] = '\0';
        int err = 0;
        char *end = nullptr;
        if (e::strtol(s, &end, 10, &err) != 42) { return false; }
        if (end != s + 3 || err != 0) { return false; }
    }
    return true;
}
static_assert(sl10_ws(), "TC-SL-10 空白の読み飛ばし");

// 変換なしの入力: 0を返し、*endptr は nptr の先頭(§3.3)
constexpr bool sl10_noconv(const char *s) {
    int err = 0;
    char *end = nullptr;
    return e::strtol(s, &end, 10, &err) == 0 && end == s && err == 0;
}
static_assert(sl10_noconv("+"), "TC-SL-10 +");
static_assert(sl10_noconv("-"), "TC-SL-10 -");
static_assert(sl10_noconv("+-1"), "TC-SL-10 +-1");
static_assert(sl10_noconv("- 1"), "TC-SL-10 - 1");
static_assert(sl10_noconv("+\t1"), "TC-SL-10 符号の後の空白");
static_assert(sl10_noconv("\x1c" "42"), "TC-SL-10 0x1C は空白でない");
static_assert(sl10_noconv("\x1f" "42"), "TC-SL-10 0x1F は空白でない");

// ------------------------------------------------- TC-SL-18: _itoa_s の変換例

// size ちょうど(終端含む)で成功し、配列版と一致する(want は終端付き文字列)。
// ポインタ版は size=w+1(必要な大きさちょうど)を渡す(TC-SL-18 の条件)。
constexpr bool sl18_row(int value, int radix, const char *want) {
    const std::size_t w = sl_len(want) + 1;
    char p[40] = {};
    char a[40] = {};
    if (e::_itoa_s(value, p, w, radix) != 0) { return false; }   // ポインタ版: ちょうどの size
    if (e::_itoa_s(value, a, radix) != 0) { return false; }      // 配列版: N=40
    for (std::size_t i = 0; i < w; ++i) {
        if (p[i] != want[i] || a[i] != want[i]) { return false; }
    }
    return true;
}

// §4.5 の変換の例の全行(成功する行)
static_assert(sl18_row(0, 10, "0"), "TC-SL-18 0 base10");
static_assert(sl18_row(123, 10, "123"), "TC-SL-18 123");
static_assert(sl18_row(-123, 10, "-123"), "TC-SL-18 -123");
static_assert(sl18_row(-1, 10, "-1"), "TC-SL-18 -1 base10");
static_assert(sl18_row(255, 16, "ff"), "TC-SL-18 255 base16");
static_assert(sl18_row(255, 2, "11111111"), "TC-SL-18 255 base2");
static_assert(sl18_row(35, 36, "z"), "TC-SL-18 35 base36");
static_assert(sl18_row(-1, 16, "ffffffff"), "TC-SL-18 -1 base16");
static_assert(sl18_row(-1, 2, "11111111111111111111111111111111"), "TC-SL-18 -1 base2");
static_assert(sl18_row(INT_MIN, 10, "-2147483648"), "TC-SL-18 INT_MIN base10");
static_assert(sl18_row(INT_MIN, 16, "80000000"), "TC-SL-18 INT_MIN base16");
static_assert(sl18_row(INT_MAX, 10, "2147483647"), "TC-SL-18 INT_MAX base10");

// 値(0、1、-1、9、10、35、36、255、256、65535、INT_MAX、INT_MIN)
// × 基数(2、8、10、16、36)の表。期待値は独立に計算した手書き値(TC-SL-18)
constexpr bool sl18_table() {
    const bool ok =
        sl18_row(0, 2, "0") && sl18_row(0, 8, "0") && sl18_row(0, 10, "0")
        && sl18_row(0, 16, "0") && sl18_row(0, 36, "0")
        && sl18_row(1, 2, "1") && sl18_row(1, 8, "1") && sl18_row(1, 10, "1")
        && sl18_row(1, 16, "1") && sl18_row(1, 36, "1")
        && sl18_row(-1, 2, "11111111111111111111111111111111")
        && sl18_row(-1, 8, "37777777777") && sl18_row(-1, 10, "-1")
        && sl18_row(-1, 16, "ffffffff") && sl18_row(-1, 36, "1z141z3")
        && sl18_row(9, 2, "1001") && sl18_row(9, 8, "11") && sl18_row(9, 10, "9")
        && sl18_row(9, 16, "9") && sl18_row(9, 36, "9")
        && sl18_row(10, 2, "1010") && sl18_row(10, 8, "12") && sl18_row(10, 10, "10")
        && sl18_row(10, 16, "a") && sl18_row(10, 36, "a")
        && sl18_row(35, 2, "100011") && sl18_row(35, 8, "43") && sl18_row(35, 10, "35")
        && sl18_row(35, 16, "23") && sl18_row(35, 36, "z")
        && sl18_row(36, 2, "100100") && sl18_row(36, 8, "44") && sl18_row(36, 10, "36")
        && sl18_row(36, 16, "24") && sl18_row(36, 36, "10")
        && sl18_row(255, 2, "11111111") && sl18_row(255, 8, "377") && sl18_row(255, 10, "255")
        && sl18_row(255, 16, "ff") && sl18_row(255, 36, "73")
        && sl18_row(256, 2, "100000000") && sl18_row(256, 8, "400") && sl18_row(256, 10, "256")
        && sl18_row(256, 16, "100") && sl18_row(256, 36, "74")
        && sl18_row(65535, 2, "1111111111111111") && sl18_row(65535, 8, "177777")
        && sl18_row(65535, 10, "65535") && sl18_row(65535, 16, "ffff")
        && sl18_row(65535, 36, "1ekf")
        && sl18_row(INT_MAX, 2, "1111111111111111111111111111111")
        && sl18_row(INT_MAX, 8, "17777777777") && sl18_row(INT_MAX, 10, "2147483647")
        && sl18_row(INT_MAX, 16, "7fffffff") && sl18_row(INT_MAX, 36, "zik0zj")
        && sl18_row(INT_MIN, 2, "10000000000000000000000000000000")
        && sl18_row(INT_MIN, 8, "20000000000") && sl18_row(INT_MIN, 10, "-2147483648")
        && sl18_row(INT_MIN, 16, "80000000") && sl18_row(INT_MIN, 36, "zik0zk");
    return ok;
}
static_assert(sl18_table(), "TC-SL-18 値×基数の表");

// ------------------------------------------------- TC-SL-19: エラー戻り値

// (a) buffer が nullptr: EINVAL(バッファは存在しない)
static_assert(e::_itoa_s(10, static_cast<char *>(nullptr), 8, 10) == ILIBC_EINVAL,
              "TC-SL-19 nullptr");

// (b) size が 0: EINVAL。buffer は番兵のまま不変(検査順1: buffer[0]も触らない)
constexpr bool sl19_size0() {
    char buf[4] = { 'Q', 'Q', 'Q', 'Q' };
    const int r = e::_itoa_s(10, buf, 0, 10);
    return r == ILIBC_EINVAL && buf[0] == 'Q' && buf[1] == 'Q' && buf[2] == 'Q' && buf[3] == 'Q';
}
static_assert(sl19_size0(), "TC-SL-19 size0");

// (c) 基数の誤り(−1、0、1、37、INT_MIN、INT_MAX): EINVAL、buffer[0] のみ '\0'
constexpr bool sl19_badradix() {
    const int bad[6] = { -1, 0, 1, 37, INT_MIN, INT_MAX };
    for (int i = 0; i < 6; ++i) {
        char buf[4] = { 'Q', 'Q', 'Q', 'Q' };
        const int r = e::_itoa_s(10, buf, 4, bad[i]);
        if (r != ILIBC_EINVAL) { return false; }
        if (buf[0] != '\0') { return false; }
        if (buf[1] != 'Q' || buf[2] != 'Q' || buf[3] != 'Q') { return false; }
    }
    return true;
}
static_assert(sl19_badradix(), "TC-SL-19 基数の誤り");

// (d) size が必要な大きさより1小さい: ERANGE、buffer[0] のみ '\0'("-123" は size 5 必要)
constexpr bool sl19_erange() {
    char buf[4] = { 'Q', 'Q', 'Q', 'Q' };
    const int r = e::_itoa_s(-123, buf, 4, 10);
    return r == ILIBC_ERANGE && buf[0] == '\0' && buf[1] == 'Q' && buf[2] == 'Q' && buf[3] == 'Q';
}
static_assert(sl19_erange(), "TC-SL-19 size不足");

// (e) 条件の重なり: nullptr+基数誤り、size0+基数誤りは、検査順(§4.5)に従い
// EINVAL で、buffer は不変
static_assert(e::_itoa_s(10, static_cast<char *>(nullptr), 8, 1) == ILIBC_EINVAL,
              "TC-SL-19 nullptr+基数");
constexpr bool sl19_size0_radix() {
    char buf[4] = { 'Q', 'Q', 'Q', 'Q' };
    const int r = e::_itoa_s(10, buf, 0, 1);
    return r == ILIBC_EINVAL && buf[0] == 'Q' && buf[1] == 'Q';
}
static_assert(sl19_size0_radix(), "TC-SL-19 size0+基数");

// ------------------------------------------------- TC-SL-20: 呼び出し形と型

// ポインタ版と配列版が同じ入力で同じ結果、戻り値型 int、noexcept
constexpr bool sl20_forms() {
    char b[8] = {};
    char *p = b;
    static_assert(std::is_same_v<decltype(e::_itoa_s(1, b, 10)), int>, "TC-SL-20 配列版の型");
    static_assert(std::is_same_v<decltype(e::_itoa_s(1, p, 8, 10)), int>, "TC-SL-20 ポインタ版の型");
    static_assert(noexcept(e::_itoa_s(1, b, 10)), "TC-SL-20 配列版 noexcept");
    static_assert(noexcept(e::_itoa_s(1, p, 8, 10)), "TC-SL-20 ポインタ版 noexcept");
    // 同じ入力・同じ結果(配列版は N から size を得る)。"-1234" は5文字+終端
    char a1[16] = {};
    char a2[16] = {};
    const int r1 = e::_itoa_s(-1234, a1, 16, 10);   // ポインタ版: size=16
    const int r2 = e::_itoa_s(-1234, a2, 10);       // 配列版: N=16、radix=10
    return r1 == 0 && r2 == 0 && a1[0] == a2[0] && a1[4] == a2[4] && a1[4] == '4'
        && a1[5] == '\0' && a2[5] == '\0';
}
static_assert(sl20_forms(), "TC-SL-20 呼び出し形");

// std:: の4関数が noexcept(SL-03〜SL-06)
static_assert(noexcept(t::atoi("")) && noexcept(t::atol("")));
static_assert(noexcept(t::strtol("", nullptr, 10)) && noexcept(t::strtoul("", nullptr, 10)));
