// cstring.cpp — <cstring>のコンパイル時試験(S)。
// spec/ilibc_cstring_spec.md §9 TC-ST-01〜03、05 と spec/ilibc_test_spec.md §5.3。
// 期待値はすべて実装と独立に手書きした生値である(TC-ST-01)。
// 関数ごとに別の static_assert にして、失敗箇所を絞る(§5.3)。
#include <cstring>
#include <type_traits>
#include <ilibc/config.hpp>

// 試験対象の名前空間(std、またはホスト差分の ilibc_under_test)(T1)
namespace t = ILIBC_NS;
// 内部核(仕様の記述に合わせる)
namespace d = ilibc::detail;

// ------------------------------------------------- TC-ST-01: 文字列系13関数

// strcpy: コピー内容・終端・戻り値が第1引数であること
constexpr bool st01_strcpy() {
    char buf[8] = {'Q', 'Q', 'Q', 'Q', 'Q', 'Q', 'Q', 'Q'};
    char *r = t::strcpy(buf, "ab");
    return r == buf && buf[0] == 'a' && buf[1] == 'b' && buf[2] == '\0'
        && buf[3] == 'Q';                       // 終端の後ろは書き換わらない
}
static_assert(st01_strcpy(), "TC-ST-01 strcpy");

// strncpy: 残りを'\0'で埋める/ちょうどnで終端なし/n=0で何も書かない/戻り値
constexpr bool st01_strncpy() {
    char a[8] = {'Q', 'Q', 'Q', 'Q', 'Q', 'Q', 'Q', 'Q'};
    char b[8] = {'Q', 'Q', 'Q', 'Q', 'Q', 'Q', 'Q', 'Q'};
    char c[8] = {'Q', 'Q', 'Q', 'Q', 'Q', 'Q', 'Q', 'Q'};
    char e[4] = {'Q', 'Q', 'Q', 'Q'};
    const char *ra = t::strncpy(a, "xy", 5);    // s2がn未満: 後ろを'\0'で埋める
    const char *rb = t::strncpy(b, "12345", 5); // s2がちょうどn: 終端なし
    const char *rc = t::strncpy(c, "123456", 5); // s2がn超: 先頭n文字のみ
    const char *re = t::strncpy(e, "zz", 0);    // n=0: 何も書かない
    return ra == a && rb == b && rc == c && re == e
        && a[0] == 'x' && a[1] == 'y' && a[2] == '\0' && a[3] == '\0' && a[4] == '\0'
        && a[5] == 'Q'
        && b[0] == '1' && b[4] == '5'
        && c[0] == '1' && c[4] == '5'
        && e[0] == 'Q' && e[1] == 'Q';
}
static_assert(st01_strncpy(), "TC-ST-01 strncpy");

// strcat: 末尾に連結、終端を含めてコピー、戻り値が第1引数
constexpr bool st01_strcat() {
    char buf[8] = {'h', 'i', '\0', 'Q', 'Q', 'Q', 'Q', 'Q'};
    char *r = t::strcat(buf, "!");
    return r == buf && buf[0] == 'h' && buf[1] == 'i' && buf[2] == '!'
        && buf[3] == '\0' && buf[4] == 'Q';
}
static_assert(st01_strcat(), "TC-ST-01 strcat");

// strcat: 空文字列への連結(s2が空でも終端は再配置される)
constexpr bool st01_strcat_empty() {
    char buf[4] = {'a', '\0', 'Q', 'Q'};
    char *r = t::strcat(buf, "");
    return r == buf && buf[0] == 'a' && buf[1] == '\0' && buf[2] == 'Q';
}
static_assert(st01_strcat_empty(), "TC-ST-01 strcat empty s2");

// strncat: n未満/ちょうどn/超過、必ず終端、n=0は終端のみ、戻り値
constexpr bool st01_strncat() {
    char a[8] = {'x', '\0', 'Q', 'Q', 'Q', 'Q', 'Q', 'Q'};
    char b[8] = {'x', '\0', 'Q', 'Q', 'Q', 'Q', 'Q', 'Q'};
    char c[8] = {'x', '\0', 'Q', 'Q', 'Q', 'Q', 'Q', 'Q'};
    char e[4] = {'x', 'y', '\0', 'Q'};
    const char *ra = t::strncat(a, "abc", 2);   // n<長さ: n文字だけ
    const char *rb = t::strncat(b, "ab", 5);    // n>長さ: 全文字+終端
    const char *rc = t::strncat(c, "abc", 3);   // n=長さ: 全文字+終端
    const char *re = t::strncat(e, "zz", 0);    // n=0: 終端のみ書き直す
    return ra == a && rb == b && rc == c && re == e
        && a[0] == 'x' && a[1] == 'a' && a[2] == 'b' && a[3] == '\0' && a[4] == 'Q'
        && b[0] == 'x' && b[1] == 'a' && b[2] == 'b' && b[3] == '\0'
        && c[0] == 'x' && c[1] == 'a' && c[2] == 'b' && c[3] == 'c' && c[4] == '\0'
        && e[0] == 'x' && e[1] == 'y' && e[2] == '\0' && e[3] == 'Q';
}
static_assert(st01_strncat(), "TC-ST-01 strncat");

// strcmp: 等しい・接頭辞・大小・空文字列(符号のみで判定)
static_assert(t::strcmp("abc", "abc") == 0, "TC-ST-01 strcmp equal");
static_assert(t::strcmp("", "") == 0, "TC-ST-01 strcmp both empty");
static_assert(t::strcmp("", "a") < 0, "TC-ST-01 strcmp empty vs a");
static_assert(t::strcmp("ab", "abc") < 0, "TC-ST-01 strcmp prefix");
static_assert(t::strcmp("abc", "ab") > 0, "TC-ST-01 strcmp longer");
static_assert(t::strcmp("A", "a") < 0, "TC-ST-01 strcmp case");

// strncmp: n=0で0、終端で停止、'\0'の後の差は比較されない
static_assert(t::strncmp("abc", "abd", 3) < 0, "TC-ST-01 strncmp differ");
static_assert(t::strncmp("abc", "abd", 2) == 0, "TC-ST-01 strncmp limit");
static_assert(t::strncmp("abc", "abc", 10) == 0, "TC-ST-01 strncmp past end");
static_assert(t::strncmp("abc", "xyz", 0) == 0, "TC-ST-01 strncmp n=0");
static_assert(t::strncmp("ab", "abc", 5) < 0, "TC-ST-01 strncmp stop at NUL");
static_assert(t::strncmp("ab\0X", "ab\0Y", 4) == 0, "TC-ST-01 strncmp after NUL");

// strchr: 先頭/途中/不一致/終端、c=256(=(char)0)は終端を探す
constexpr bool st01_strchr() {
    const char s[] = "abca";
    const char buf[4] = {'x', '\xff', 'y', '\0'};
    return t::strchr(s, 'a') == s
        && t::strchr(s, 'c') == s + 2
        && t::strchr(s, 'z') == nullptr
        && t::strchr(s, '\0') == s + 4
        && t::strchr(buf, -1) == buf + 1        // c=-1 → (char)0xFF
        && t::strchr(s, 256) == s + 4;          // c=256 → (char)0 → 終端
}
static_assert(st01_strchr(), "TC-ST-01 strchr");

// strchr(非const版): 戻り値は char *
constexpr bool st01_strchr_mut() {
    char buf[4] = {'a', 'b', 'c', '\0'};
    char *r = t::strchr(buf, 'b');
    return r == buf + 1;
}
static_assert(st01_strchr_mut(), "TC-ST-01 strchr non-const");

// strrchr: 複数一致で最後、単数、不一致、終端
constexpr bool st01_strrchr() {
    const char s[] = "abca";
    const char e[] = "";
    return t::strrchr(s, 'a') == s + 3
        && t::strrchr(s, 'b') == s + 1
        && t::strrchr(s, 'z') == nullptr
        && t::strrchr(s, '\0') == s + 4
        && t::strrchr(e, '\0') == e;             // 空文字列: 終端を返す
}
static_assert(st01_strrchr(), "TC-ST-01 strrchr");

// strstr: 空のs2→s1、部分一致後に失敗して再開、末尾一致、失敗
constexpr bool st01_strstr() {
    const char s[] = "aaab";
    const char e[] = "";
    return t::strstr(s, "aab") == s + 1         // 再開(§5.4 試験の重点)
        && t::strstr(s, "") == s                // 空のs2はs1を返す
        && t::strstr(s, "aaab") == s
        && t::strstr(s, "b") == s + 3
        && t::strstr(s, "abcd") == nullptr
        && t::strstr(e, "a") == nullptr
        && t::strstr(e, "") == e;
}
static_assert(st01_strstr(), "TC-ST-01 strstr");

// strspn: 全一致・先頭不一致・空のs1/s2・s2の重複文字
static_assert(t::strspn("123abc", "12345") == 3, "TC-ST-01 strspn basic");
static_assert(t::strspn("abab", "ba") == 4, "TC-ST-01 strspn all match");
static_assert(t::strspn("abc", "123") == 0, "TC-ST-01 strspn first mismatch");
static_assert(t::strspn("", "12") == 0, "TC-ST-01 strspn empty s1");
static_assert(t::strspn("aa", "") == 0, "TC-ST-01 strspn empty s2");
static_assert(t::strspn("aab", "aba") == 3, "TC-ST-01 strspn dup in s2");

// strcspn: 一致なし・先頭一致・s2が空ならstrlen(s1)・空のs1
static_assert(t::strcspn("abc", "xyz") == 3, "TC-ST-01 strcspn no match");
static_assert(t::strcspn("abc123", "0123456789") == 3, "TC-ST-01 strcspn basic");
static_assert(t::strcspn("123", "123") == 0, "TC-ST-01 strcspn first match");
static_assert(t::strcspn("abc", "") == 3, "TC-ST-01 strcspn empty s2");
static_assert(t::strcspn("", "a") == 0, "TC-ST-01 strcspn empty s1");

// strpbrk: 途中/不一致/空のs1/s2/s2の順序に依存しない、非const版
constexpr bool st01_strpbrk() {
    const char s[] = "abc1def";
    char m[4] = {'x', 'y', 'z', '\0'};
    return t::strpbrk(s, "123") == s + 3
        && t::strpbrk(s, "cba") == s            // s2の順序に依存しない
        && t::strpbrk(s, "zy") == nullptr
        && t::strpbrk(s, "") == nullptr
        && t::strpbrk("", "a") == nullptr
        && t::strpbrk(m, "zy") == m + 1;         // 非const版は char *
}
static_assert(st01_strpbrk(), "TC-ST-01 strpbrk");

// strlen: 空・1文字・0x80以上を含む
static_assert(t::strlen("") == 0, "TC-ST-01 strlen empty");
static_assert(t::strlen("a") == 1, "TC-ST-01 strlen one");
static_assert(t::strlen("hello") == 5, "TC-ST-01 strlen five");
static_assert(t::strlen("\xff\x80z") == 3, "TC-ST-01 strlen high bytes");

// ------------------------------------------------- TC-ST-02: ilibc::detail の核

// mem_copy_fwd: 重ならない範囲のコピーと n=0
constexpr bool st02_copy_fwd() {
    unsigned char dst[8] = {0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55};
    const unsigned char src[4] = {1, 2, 3, 4};
    d::mem_copy_fwd(dst, src, 4);
    d::mem_copy_fwd(dst + 4, src, 0);            // n=0 は何もしない
    return dst[0] == 1 && dst[1] == 2 && dst[2] == 3 && dst[3] == 4
        && dst[4] == 0x55 && dst[7] == 0x55;
}
static_assert(st02_copy_fwd(), "TC-ST-02 mem_copy_fwd");

// mem_copy_fwd: 重なる部分範囲(同じ配列内)の挙動を手書き値で確認
constexpr bool st02_copy_fwd_overlap() {
    unsigned char a[6] = {1, 2, 3, 4, 5, 6};
    d::mem_copy_fwd(a + 2, a, 3);                // d>s: 先頭から複製するため伝播する
    if (!(a[0] == 1 && a[1] == 2 && a[2] == 1 && a[3] == 2 && a[4] == 1 && a[5] == 6)) {
        return false;
    }
    unsigned char b[6] = {1, 2, 3, 4, 5, 6};
    d::mem_copy_fwd(b, b + 2, 3);                // d<s: 正しく複製される
    return b[0] == 3 && b[1] == 4 && b[2] == 5 && b[3] == 4 && b[4] == 5 && b[5] == 6;
}
static_assert(st02_copy_fwd_overlap(), "TC-ST-02 mem_copy_fwd overlap");

// mem_copy_bwd: 重なる部分範囲(同じ配列内)の挙動を手書き値で確認
constexpr bool st02_copy_bwd_overlap() {
    unsigned char a[6] = {1, 2, 3, 4, 5, 6};
    d::mem_copy_bwd(a + 2, a, 3);                // d>s: 末尾から複製し正しく移る
    if (!(a[0] == 1 && a[1] == 2 && a[2] == 1 && a[3] == 2 && a[4] == 3 && a[5] == 6)) {
        return false;
    }
    unsigned char b[6] = {1, 2, 3, 4, 5, 6};
    d::mem_copy_bwd(b, b + 2, 3);                // d<s: 逆方向のため伝播する
    if (!(b[0] == 5 && b[1] == 4 && b[2] == 5 && b[3] == 4 && b[4] == 5 && b[5] == 6)) {
        return false;
    }
    unsigned char c[3] = {9, 8, 7};
    d::mem_copy_bwd(c, c, 0);                    // n=0 は何もしない
    return c[0] == 9 && c[1] == 8 && c[2] == 7;
}
static_assert(st02_copy_bwd_overlap(), "TC-ST-02 mem_copy_bwd overlap");

// mem_compare: 等しい・n=0・先頭と末尾のみ相違・差の値
constexpr bool st02_compare() {
    const unsigned char a[3] = {1, 2, 3};
    const unsigned char b[3] = {1, 9, 3};
    const unsigned char c[3] = {1, 2, 4};
    const unsigned char d2[2] = {1, 2};
    return d::mem_compare(a, a, 3) == 0
        && d::mem_compare(a, a, 0) == 0
        && d::mem_compare(a, b, 3) == 2 - 9      // 最初の相違位置の差(§4.4)
        && d::mem_compare(a, c, 3) == 3 - 4      // 末尾のみ相違
        && d::mem_compare(a, d2, 2) == 0;
}
static_assert(st02_compare(), "TC-ST-02 mem_compare");

// mem_find: 先頭・途中・不一致・n=0
constexpr bool st02_find() {
    const unsigned char a[4] = {5, 6, 7, 6};
    return d::mem_find(a, 5, 4) == a
        && d::mem_find(a, 6, 4) == a + 1
        && d::mem_find(a, 9, 4) == nullptr
        && d::mem_find(a, 5, 0) == nullptr;      // n=0 は見つからない
}
static_assert(st02_find(), "TC-ST-02 mem_find");

// mem_fill: 設定と n=0
constexpr bool st02_fill() {
    unsigned char buf[6] = {1, 1, 1, 1, 9, 9};
    d::mem_fill(buf, 0xAB, 4);
    d::mem_fill(buf + 4, 0xCD, 0);               // n=0 は何もしない
    return buf[0] == 0xAB && buf[3] == 0xAB && buf[4] == 9 && buf[5] == 9;
}
static_assert(st02_fill(), "TC-ST-02 mem_fill");

// ------------------------------------------------- TC-ST-03: 0x80以上のバイト

// 比較関数が符号なしバイトとして働くこと(符号付きcharなら誤る入力)
constexpr bool st03_high_bytes() {
    const unsigned char hi[1] = {0x80};
    const unsigned char lo[1] = {0x7F};
    const unsigned char ff[1] = {0xFF};
    const unsigned char one[1] = {0x01};
    return d::mem_compare(hi, lo, 1) > 0          // 符号付きcharなら負になる
        && d::mem_compare(ff, one, 1) == 255 - 1;
}
static_assert(st03_high_bytes(), "TC-ST-03 mem_compare high bytes");
static_assert(t::strcmp("\xff", "a") > 0, "TC-ST-03 strcmp 0xff vs a");
static_assert(t::strcmp("\x80", "\x7f") > 0, "TC-ST-03 strcmp 0x80 vs 0x7f");
static_assert(t::strcmp("a", "\x80") < 0, "TC-ST-03 strcmp a vs 0x80");
static_assert(t::strcmp("\xfe\xff", "\xff\x01") < 0, "TC-ST-03 strcmp first-diff only");
static_assert(t::strncmp("\x80", "\x7f", 1) > 0, "TC-ST-03 strncmp high byte");
static_assert(t::strncmp("a\xff", "a\x01", 2) > 0, "TC-ST-03 strncmp second byte");

// ------------------------------------------------- TC-ST-05: オーバーロードの型

// const/非constオーバーロードの戻り値型が入力のconst性に従うこと
static_assert(std::is_same_v<decltype(t::memchr(static_cast<void *>(nullptr), 0, 0)), void *>,
              "TC-ST-05 memchr void*");
static_assert(
    std::is_same_v<decltype(t::memchr(static_cast<const void *>(nullptr), 0, 0)), const void *>,
    "TC-ST-05 memchr const void*");
static_assert(std::is_same_v<decltype(t::strchr(static_cast<char *>(nullptr), 0)), char *>,
              "TC-ST-05 strchr char*");
static_assert(std::is_same_v<decltype(t::strchr(static_cast<const char *>(nullptr), 0)),
                             const char *>,
              "TC-ST-05 strchr const char*");
static_assert(std::is_same_v<decltype(t::strrchr(static_cast<char *>(nullptr), 0)), char *>,
              "TC-ST-05 strrchr char*");
static_assert(std::is_same_v<decltype(t::strrchr(static_cast<const char *>(nullptr), 0)),
                             const char *>,
              "TC-ST-05 strrchr const char*");
static_assert(std::is_same_v<decltype(t::strstr(static_cast<char *>(nullptr), "")), char *>,
              "TC-ST-05 strstr char*");
static_assert(std::is_same_v<decltype(t::strstr(static_cast<const char *>(nullptr), "")),
                             const char *>,
              "TC-ST-05 strstr const char*");
static_assert(std::is_same_v<decltype(t::strpbrk(static_cast<char *>(nullptr), "")), char *>,
              "TC-ST-05 strpbrk char*");
static_assert(std::is_same_v<decltype(t::strpbrk(static_cast<const char *>(nullptr), "")),
                             const char *>,
              "TC-ST-05 strpbrk const char*");

// その他の戻り値型(§2.1のシグネチャ)
static_assert(std::is_same_v<decltype(t::strlen("")), std::size_t>, "TC-ST-05 strlen");
static_assert(std::is_same_v<decltype(t::strcmp("", "")), int>, "TC-ST-05 strcmp");
static_assert(std::is_same_v<decltype(t::memcpy(nullptr, nullptr, 0)), void *>,
              "TC-ST-05 memcpy");
static_assert(std::is_same_v<decltype(t::memmove(nullptr, nullptr, 0)), void *>,
              "TC-ST-05 memmove");
static_assert(std::is_same_v<decltype(t::memset(nullptr, 0, 0)), void *>, "TC-ST-05 memset");

// noexcept(§2.1: 全関数に noexcept)
static_assert(noexcept(t::strlen("")), "noexcept strlen");
static_assert(noexcept(t::memcpy(nullptr, nullptr, 0)), "noexcept memcpy");
static_assert(noexcept(t::memchr(static_cast<void *>(nullptr), 0, 0)), "noexcept memchr");
static_assert(noexcept(t::strchr("", 0)), "noexcept strchr");
static_assert(noexcept(t::strcmp("", "")), "noexcept strcmp");
