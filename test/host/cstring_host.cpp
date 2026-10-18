// cstring_host.cpp — <cstring>のホストlibc差分試験(H: TC-ST-06)と、書き込み
// 範囲・アライメント・memmove重なりのホスト実行(H: TC-ST-07〜09)。
// spec/ilibc_cstring_spec.md §9、spec/ilibc_test_spec.md §5.7(§5.7.2 比較規則)。
// ビルド: -m32 -static -DILIBC_HOST_TEST -U_FORTIFY_SOURCE。
// 本体は ilibc_under_test 名前空間、オラクルはホストlibcの実関数。
// <clocale>/<cstdio> はスタブされるため、素通りしてコンパイラ付属を使う
// (ヘッダ仕様 §7.3 S3。ホスト施設を使う試験コードだけの設定)。
#define ILIBC_STUB_PASSTHROUGH
#include <clocale>  // setlocale
#include <cstdio>   // 失敗の報告(ホスト専用のため stdio を使う)
#include <cstring>  // 本ライブラリ(ILIBC_HOST_TEST で ilibc_under_test)

#include "../common/str_cases.hpp"  // TC-ST-07〜09 の共有本体(TST §5.2)

// 試験対象の名前空間(T1)
namespace t = ILIBC_NS;

// ホストlibcの18関数。
// ホストの <string.h> を含めると本ライブラリの宣言と衝突するため、含めずに
// C の実関数を宣言して直接呼ぶ(オラクルはホストlibc、TST §5.7.1)。
extern "C" {
void *memcpy(void *, const void *, std::size_t);
void *memmove(void *, const void *, std::size_t);
int memcmp(const void *, const void *, std::size_t);
void *memchr(const void *, int, std::size_t);
void *memset(void *, int, std::size_t);
char *strcpy(char *, const char *);
char *strncpy(char *, const char *, std::size_t);
char *strcat(char *, const char *);
char *strncat(char *, const char *, std::size_t);
int strcmp(const char *, const char *);
int strncmp(const char *, const char *, std::size_t);
char *strchr(const char *, int);
char *strrchr(const char *, int);
char *strstr(const char *, const char *);
std::size_t strspn(const char *, const char *);
std::size_t strcspn(const char *, const char *);
char *strpbrk(const char *, const char *);
std::size_t strlen(const char *);
}

// 文字列の最大長(TC-ST-06: 0〜64)と、書き込み先バッファの大きさ。
// バッファは最大長より大きく取り、末尾を比較して超過書き込みも検出する。
constexpr int kMaxLen = 64;
constexpr int kBuf = 80;
constexpr int kIter = 1000;  // 差分試験の反復回数(決定的: シード固定)

// TC-ST-06 の失敗数と、最初の20件までの不一致
static int g_fail6 = 0;
static int g_printed = 0;
static unsigned g_seed_log = 0x12345678u;

// 不一致を1件記録する(再現にはシードと反復番号を使う、TST P8)
static void report(const char *fn, int iter) {
    ++g_fail6;
    if (g_printed < 20) {
        ++g_printed;
        std::printf("TC-ST-06 FAIL fn=%s iter=%d seed=0x%08x\n", fn, iter, g_seed_log);
    }
}

// --------------------------------------------------------------- 入力生成

// xorshift32乱数(TST §5.7.1。シードはログに出す)
static unsigned g_rng = 0x12345678u;
static unsigned rnd() {
    unsigned x = g_rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    g_rng = x;
    return x;
}

// 0以上 n 未満(n > 0)
static int rnd_below(int n) {
    return static_cast<int>(rnd() % static_cast<unsigned>(n));
}

// バイト値: 0 と 0x80 以上を高頻度に含める(TC-ST-06)
static unsigned char rnd_byte() {
    const unsigned r = rnd();
    switch (r & 7u) {
    case 0:
    case 1:
        return 0x00;                                     // 終端・内部ゼロ
    case 2:
    case 3:
    case 4:
        return static_cast<unsigned char>(0x80u | ((r >> 8) & 0x7Fu));  // 上位ビットあり
    default:
        return static_cast<unsigned char>((r >> 8) & 0x7Fu);            // 0x00〜0x7F
    }
}

// 文字列用のバイト(終端を壊さないよう0は別の値に置き換える)
static unsigned char rnd_str_byte() {
    const unsigned char b = rnd_byte();
    return (b != 0) ? b : static_cast<unsigned char>('a' + rnd_below(5));
}

// 集合に含まれない 1〜255 のバイトを1つ選ぶ(0は終端になるため避ける)
static unsigned char not_in_set(const unsigned char *cs) {
    const int start = rnd_below(255);
    for (int i = 0; i < 255; ++i) {
        const int c = 1 + ((start + i) % 255);
        if (c != cs[0] && c != cs[1] && c != cs[2]) {
            return static_cast<unsigned char>(c);
        }
    }
    return 0x7F;  // 到達しない(除外されるのは3値だけ)
}

// 走査試験用の文字列を作る: 先頭kバイト + 終点1バイト + 末尾。
// prefix_in=true なら先頭が集合内・終点が集合外(strspn用)、
// false なら先頭が集合外・終点が集合内(strcspn/strpbrk用)。
static void build_run(char *run, int k, const unsigned char *cs, bool prefix_in) {
    for (int i = 0; i < k; ++i) {
        run[i] = prefix_in ? static_cast<char>(cs[rnd_below(3)])
                           : static_cast<char>(not_in_set(cs));
    }
    run[k] = prefix_in ? static_cast<char>(not_in_set(cs))
                       : static_cast<char>(cs[rnd_below(3)]);
    const int tail = rnd_below(kMaxLen - k);  // k ≤ 16 より正
    for (int i = k + 1; i < k + 1 + tail; ++i) {
        run[i] = static_cast<char>(rnd_str_byte());
    }
    run[k + 1 + tail] = '\0';
}

// ---------------------------------------------------------------- 比較補助

// 比較関数は符号のみ(TST §5.7.2 #1)
static bool same_sign(int a, int b) {
    return (a < 0) == (b < 0) && (a > 0) == (b > 0);
}

// ポインタを先頭からのオフセットに変換する(nullptrは-1)(TST §5.7.2 #2)
template <class B, class P>
static long off(const B *base, const P *p) {
    return (p == nullptr) ? -1L : static_cast<long>(p - base);
}

// 2つの領域の先頭 n バイトが一致するか(超過書き込みも検出できるよう
// 書き込み前の初期値まで含めて比較する)
template <class A, class B>
static bool same_bytes(const A *a, const B *b, int n) {
    for (int i = 0; i < n; ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

// n 要素を値 v で埋める
template <class E>
static void fill_n(E *p, int n, E v) {
    for (int i = 0; i < n; ++i) {
        p[i] = v;
    }
}

// n 要素をコピーする
template <class E>
static void copy_n(E *d, const E *s, int n) {
    for (int i = 0; i < n; ++i) {
        d[i] = s[i];
    }
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

// --------------------------------------------------------- TC-ST-06 本体

// 1反復の差分試験(18関数すべて)。入力は乱数、比較規則は TST §5.7.2。
static void diff_iter(int iter) {
    // --- 入力: 長さ0〜64の文字列2本(内部に0を含む)と生バイト列 ---
    char s1[kMaxLen + 1];
    char s2[kMaxLen + 1];
    const int l1 = rnd_below(kMaxLen + 1);
    const int l2 = rnd_below(kMaxLen + 1);
    for (int i = 0; i < l1; ++i) {
        s1[i] = static_cast<char>(rnd_byte());
    }
    s1[l1] = '\0';
    for (int i = 0; i < l2; ++i) {
        s2[i] = static_cast<char>(rnd_byte());
    }
    s2[l2] = '\0';

    unsigned char src[kBuf];
    for (int i = 0; i < kBuf; ++i) {
        src[i] = rnd_byte();
    }
    const int n = rnd_below(kMaxLen + 1);  // mem* の長さ 0〜64

    // --- strlen (ST-18): 精確値
    if (t::strlen(s1) != ::strlen(s1)) {
        report("strlen", iter);
    }

    // --- strcpy (ST-06): 書き込み先2本を同一初期値にして全体を比較 ---
    char d1[kBuf];
    char d2[kBuf];
    fill_n(d1, kBuf, static_cast<char>(0xAA));
    fill_n(d2, kBuf, static_cast<char>(0xAA));
    t::strcpy(d1, s1);
    ::strcpy(d2, s1);
    if (!same_bytes(d1, d2, kBuf)) {
        report("strcpy", iter);
    }

    // --- strncpy (ST-07): n=0〜64(ちょうど0と、nちょうどを書く)
    const int nn1 = rnd_below(kMaxLen + 1);
    fill_n(d1, kBuf, static_cast<char>(0xAA));
    fill_n(d2, kBuf, static_cast<char>(0xAA));
    t::strncpy(d1, s1, static_cast<std::size_t>(nn1));
    ::strncpy(d2, s1, static_cast<std::size_t>(nn1));
    if (!same_bytes(d1, d2, kBuf)) {
        report("strncpy", iter);
    }

    // --- strcat (ST-08): 短い基底文字列の後へ連結(基底は乱数で作る) ---
    const int bl = rnd_below(9);
    fill_n(d1, kBuf, static_cast<char>(0xAA));
    for (int i = 0; i < bl; ++i) {
        d1[i] = static_cast<char>(rnd_str_byte());
    }
    d1[bl] = '\0';
    copy_n(d2, d1, kBuf);
    t::strcat(d1, s1);
    ::strcat(d2, s1);
    if (!same_bytes(d1, d2, kBuf)) {
        report("strcat", iter);
    }

    // --- strncat (ST-09): 最大 n+1 バイト(文字n + 終端)
    const int nn2 = rnd_below(kMaxLen + 1);
    fill_n(d1, kBuf, static_cast<char>(0xAA));
    for (int i = 0; i < bl; ++i) {
        d1[i] = static_cast<char>(rnd_str_byte());
    }
    d1[bl] = '\0';
    copy_n(d2, d1, kBuf);
    t::strncat(d1, s1, static_cast<std::size_t>(nn2));
    ::strncat(d2, s1, static_cast<std::size_t>(nn2));
    if (!same_bytes(d1, d2, kBuf)) {
        report("strncat", iter);
    }

    // --- strcmp (ST-10): 符号のみ(TST §5.7.2 #1)
    if (!same_sign(t::strcmp(s1, s2), ::strcmp(s1, s2))) {
        report("strcmp", iter);
    }

    // --- strncmp (ST-11): n は文字列長を超えてもよい(0〜70)
    const int nn3 = rnd_below(71);
    if (!same_sign(t::strncmp(s1, s2, static_cast<std::size_t>(nn3)),
                   ::strncmp(s1, s2, static_cast<std::size_t>(nn3)))) {
        report("strncmp", iter);
    }

    // --- strchr / strrchr (ST-12/13): 半数は探す値を植える。オフセット比較
    {
        const int c = static_cast<int>(rnd() & 0xFFu);
        if ((rnd() & 1u) != 0u && l1 > 0) {
            s1[rnd_below(l1)] = static_cast<char>(c);  // 終端より前に植える
        }
        if (off(s1, t::strchr(s1, c)) != off(s1, ::strchr(s1, c))) {
            report("strchr", iter);
        }
        if (off(s1, t::strrchr(s1, c)) != off(s1, ::strrchr(s1, c))) {
            report("strrchr", iter);
        }
    }

    // --- strstr (ST-14): 半数は needle を haystack に植える(発見経路) ---
    {
        char needle[kMaxLen + 1];
        char hay[kMaxLen + 1];
        const int tl = rnd_below(kMaxLen + 1);
        for (int i = 0; i < tl; ++i) {
            needle[i] = static_cast<char>(rnd_str_byte());
        }
        needle[tl] = '\0';
        const int hl = rnd_below(kMaxLen + 1);
        const bool plant = tl <= hl && (rnd() & 1u) != 0u;
        if (plant) {
            const int pos = rnd_below(hl - tl + 1);
            for (int i = 0; i < pos; ++i) {
                hay[i] = static_cast<char>(rnd_str_byte());  // 植える前は0なし
            }
            for (int i = 0; i < tl; ++i) {
                hay[pos + i] = needle[i];
            }
            for (int i = pos + tl; i < hl; ++i) {
                hay[i] = static_cast<char>(rnd_str_byte());
            }
        } else {
            for (int i = 0; i < hl; ++i) {
                hay[i] = static_cast<char>(rnd_byte());
            }
        }
        hay[hl] = '\0';
        if (off(hay, t::strstr(hay, needle)) != off(hay, ::strstr(hay, needle))) {
            report("strstr", iter);
        }
    }

    // --- 文字集合(非零3バイト)と、集合内外で止まる走査文字列 ---
    unsigned char cs[3];
    char set[4];
    for (int i = 0; i < 3; ++i) {
        cs[i] = static_cast<unsigned char>(1 + rnd_below(255));
        set[i] = static_cast<char>(cs[i]);
    }
    set[3] = '\0';
    char runx[kMaxLen + 1];  // 先頭が集合内 → strspn が k で止まる
    char runy[kMaxLen + 1];  // 先頭が集合外 → strcspn/strpbrk が k で止まる
    build_run(runx, rnd_below(17), cs, true);
    build_run(runy, rnd_below(17), cs, false);

    // --- strspn / strcspn (ST-15/16): 精確値で比較 ---
    if (t::strspn(runx, set) != ::strspn(runx, set)) {
        report("strspn", iter);
    }
    if (t::strspn(runy, set) != ::strspn(runy, set)) {
        report("strspn.runy", iter);
    }
    if (t::strspn(s1, set) != ::strspn(s1, set)) {
        report("strspn.rand", iter);
    }
    if (t::strcspn(runy, set) != ::strcspn(runy, set)) {
        report("strcspn", iter);
    }
    if (t::strcspn(runx, set) != ::strcspn(runx, set)) {
        report("strcspn.runx", iter);
    }
    if (t::strcspn(s1, s2) != ::strcspn(s1, s2)) {
        report("strcspn.rand", iter);
    }

    // --- strpbrk (ST-17): オフセット比較(発見・不発見の両経路) ---
    if (off(runx, t::strpbrk(runx, set)) != off(runx, ::strpbrk(runx, set))) {
        report("strpbrk.runx", iter);
    }
    if (off(runy, t::strpbrk(runy, set)) != off(runy, ::strpbrk(runy, set))) {
        report("strpbrk.runy", iter);
    }
    if (off(s1, t::strpbrk(s1, s2)) != off(s1, ::strpbrk(s1, s2))) {
        report("strpbrk.rand", iter);
    }

    // --- memcpy (ST-01): 2本の同一初期値で全体比較 ---
    char e1[kBuf];
    char e2[kBuf];
    fill_n(e1, kBuf, static_cast<char>(0x5A));
    fill_n(e2, kBuf, static_cast<char>(0x5A));
    t::memcpy(e1, src, static_cast<std::size_t>(n));
    ::memcpy(e2, src, static_cast<std::size_t>(n));
    if (!same_bytes(e1, e2, kBuf)) {
        report("memcpy", iter);
    }

    // --- memset (ST-05): c は0〜1023(上位ビットの切り詰めも試す) ---
    fill_n(e1, kBuf, static_cast<char>(0x5A));
    fill_n(e2, kBuf, static_cast<char>(0x5A));
    const int cv = static_cast<int>(rnd() & 0x3FFu);
    t::memset(e1, cv, static_cast<std::size_t>(n));
    ::memset(e2, cv, static_cast<std::size_t>(n));
    if (!same_bytes(e1, e2, kBuf)) {
        report("memset", iter);
    }

    // --- memmove (ST-02): 非重なり + 重なり(オフセット差 -16〜+16) ---
    fill_n(e1, kBuf, static_cast<char>(0x5A));
    fill_n(e2, kBuf, static_cast<char>(0x5A));
    t::memmove(e1, src, static_cast<std::size_t>(n));
    ::memmove(e2, src, static_cast<std::size_t>(n));
    if (!same_bytes(e1, e2, kBuf)) {
        report("memmove", iter);
    }
    {
        unsigned char g1[kBuf];
        unsigned char g2[kBuf];
        for (int i = 0; i < kBuf; ++i) {
            g1[i] = rnd_byte();
            g2[i] = g1[i];
        }
        const int delta = rnd_below(33) - 16;  // -16〜+16
        const int nn4 = rnd_below(25);         // 範囲 [40+delta, 40+delta+nn4) が収まるよう制限
        t::memmove(g1 + 40 + delta, g1 + 40, static_cast<std::size_t>(nn4));
        ::memmove(g2 + 40 + delta, g2 + 40, static_cast<std::size_t>(nn4));
        if (!same_bytes(g1, g2, kBuf)) {
            report("memmove.ovl", iter);
        }

        // --- memcmp (ST-03): 符号のみ(TST §5.7.2 #1) ---
        const int nn5 = rnd_below(kMaxLen + 1);
        if (!same_sign(t::memcmp(src, g1, static_cast<std::size_t>(nn5)),
                       ::memcmp(src, g1, static_cast<std::size_t>(nn5)))) {
            report("memcmp", iter);
        }
    }

    // --- memchr (ST-04): 半数は探す値を植える。非const/const両経路、
    //     先頭からのオフセットで比較(TST §5.7.2 #2)
    {
        const int c = static_cast<int>(rnd() & 0xFFu);
        if ((rnd() & 1u) != 0u) {
            src[rnd_below(kMaxLen + 1)] = static_cast<unsigned char>(c);
        }
        const int nn6 = rnd_below(kMaxLen + 1);
        const void *p1 = t::memchr(src, c, static_cast<std::size_t>(nn6));
        const void *p2 = ::memchr(src, c, static_cast<std::size_t>(nn6));
        if (off(src, static_cast<const unsigned char *>(p1)) !=
            off(src, static_cast<const unsigned char *>(p2))) {
            report("memchr", iter);
        }
        const void *q1 = t::memchr(static_cast<const void *>(src), c,
                                   static_cast<std::size_t>(nn6));
        const void *q2 = ::memchr(static_cast<const void *>(src), c,
                                  static_cast<std::size_t>(nn6));
        if (off(src, static_cast<const unsigned char *>(q1)) !=
            off(src, static_cast<const unsigned char *>(q2))) {
            report("memchr.const", iter);
        }
    }
}

int main(int argc, char **argv) {
    // "C"ロケール固定(ILIBC-TST-001 §5.7.1)
    std::setlocale(LC_ALL, "C");

    // シード: 既定は固定値(決定的、TST P8)。argv[1] で上書きでき、必ずログに出す
    unsigned seed = 0x12345678u;
    if (argc > 1) {
        if (!parse_seed(argv[1], &seed)) {
            std::printf("bad seed: %s\n", argv[1]);
            return 2;
        }
    }
    if (seed == 0u) {
        seed = 1u;  // xorshift32 の不動点を避ける
    }
    g_rng = seed;
    g_seed_log = seed;
    std::printf("seed=0x%08x iter=%d\n", seed, kIter);

    // TC-ST-06: 18関数のホストlibc差分
    for (int iter = 0; iter < kIter; ++iter) {
        diff_iter(iter);
    }

    // TC-ST-07〜09: str_cases.hpp の共有本体(H側)
    const int c7 = str_cases::tc_st07_first_failure();
    const int c8 = str_cases::tc_st08_first_failure();
    const int c9 = str_cases::tc_st09_first_failure();
    if (c7 >= 0) {
        std::printf("TC-ST-07 FAIL str_cases.hpp case=%d seed=0x%08x\n", c7, seed);
    }
    if (c8 >= 0) {
        std::printf("TC-ST-08 FAIL str_cases.hpp case=%d seed=0x%08x\n", c8, seed);
    }
    if (c9 >= 0) {
        std::printf("TC-ST-09 FAIL str_cases.hpp case=%d seed=0x%08x\n", c9, seed);
    }

    int nfail = 0;
    if (g_fail6 == 0) {
        std::printf("TC-ST-06 PASS\n");
    } else {
        ++nfail;
    }
    if (c7 < 0) {
        std::printf("TC-ST-07 PASS\n");
    } else {
        ++nfail;
    }
    if (c8 < 0) {
        std::printf("TC-ST-08 PASS\n");
    } else {
        ++nfail;
    }
    if (c9 < 0) {
        std::printf("TC-ST-09 PASS\n");
    } else {
        ++nfail;
    }
    std::printf("SUMMARY pass=%d fail=%d\n", 4 - nfail, nfail);
    return (nfail == 0) ? 0 : 1;
}
