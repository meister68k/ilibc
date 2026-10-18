// ctype_ref.hpp — <cctype>試験の独立参照。
// spec/ilibc_cctype_spec.md §3.2(区間表)を、文字列リテラルの走査で導く。
// spec/ilibc_test_spec.md §6: 実装と同じ範囲式('A' <= c && c <= 'Z'の変形)を書いてはならない。
#pragma once

namespace ilibc_test::ref {

// lit(終端のNULを含む配列)に、整数cと一致する値が含まれるか
template <unsigned N>
constexpr bool member_of(int c, const char (&lit)[N]) {
    for (unsigned i = 0; i + 1 < N; ++i) {  // 末尾のNULは集合に含めない
        if (static_cast<int>(static_cast<unsigned char>(lit[i])) == c) {
            return true;
        }
    }
    return false;
}

// ------------------------------------------------------- 文字クラス集合

// 0x00-0x1F と 0x7F(iscntrl)
inline constexpr char kCntrl[] =
    "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
    "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x7f";
// 0x09-0x0D と 0x20(isspace)
inline constexpr char kSpace[] = "\x09\x0a\x0b\x0c\x0d\x20";
// '0'-'9'(isdigit)
inline constexpr char kDigit[] = "0123456789";
// 'A'-'Z'(isupper)
inline constexpr char kUpper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
// 'a'-'z'(islower)
inline constexpr char kLower[] = "abcdefghijklmnopqrstuvwxyz";
// 'A'-'F'(isxdigit の大文字側)
inline constexpr char kXUpper[] = "ABCDEF";
// 'a'-'f'(isxdigit の小文字側)
inline constexpr char kXLower[] = "abcdef";
// 'A'-'Z' と 'a'-'z'(isalpha)
inline constexpr char kAlpha[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
// 0x21-0x7E のうち英数字以外(ispunct)
inline constexpr char kPunct[] = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

// ------------------------------------------------------- 述語(走査と集合の和)

// 英数字か(A-Z a-z 0-9)
constexpr bool is_alnum(int c) { return member_of(c, kAlpha) || member_of(c, kDigit); }
// 英字か(A-Z a-z)
constexpr bool is_alpha(int c) { return member_of(c, kAlpha); }
// 制御文字か(0x00-0x1F と 0x7F)
constexpr bool is_cntrl(int c) { return member_of(c, kCntrl); }
// 10進数字か('0'-'9')
constexpr bool is_digit(int c) { return member_of(c, kDigit); }
// 空白以外の表示可能文字か(punct・digit・alpha の和。区間表の行から導く)
constexpr bool is_graph(int c) {
    return member_of(c, kPunct) || member_of(c, kDigit) || member_of(c, kAlpha);
}
// 小文字か('a'-'z')
constexpr bool is_lower(int c) { return member_of(c, kLower); }
// 表示可能文字か(0x20 と isgraph の和。区間表の行から導く)
constexpr bool is_print(int c) { return c == 0x20 || is_graph(c); }
// 区切り文字か(記号。punct の走査)
constexpr bool is_punct(int c) { return member_of(c, kPunct); }
// 空白類か(0x09-0x0D と 0x20)
constexpr bool is_space(int c) { return member_of(c, kSpace); }
// 大文字か('A'-'Z')
constexpr bool is_upper(int c) { return member_of(c, kUpper); }
// 16進数字か(0-9 a-f A-F)
constexpr bool is_xdigit(int c) {
    return member_of(c, kDigit) || member_of(c, kXUpper) || member_of(c, kXLower);
}
// 大文字なら +32、そうでなければそのまま(ILIBC-CTP-001 §5.2 CT-12)
constexpr int to_lower(int c) { return member_of(c, kUpper) ? c + 32 : c; }
// 小文字なら -32、そうでなければそのまま(ILIBC-CTP-001 §5.2 CT-13)
constexpr int to_upper(int c) { return member_of(c, kLower) ? c - 32 : c; }

// ------------------------------------------------------- 個数(ILIBC-CTP-001 §3.2)

// 0〜127のうち真になる文字数(仕様の個数表から。TC-CT-03が照合する)
inline constexpr int kCountCntrl = 33;
inline constexpr int kCountPrint = 95;
inline constexpr int kCountGraph = 94;
inline constexpr int kCountSpace = 6;
inline constexpr int kCountDigit = 10;
inline constexpr int kCountXdigit = 22;
inline constexpr int kCountUpper = 26;
inline constexpr int kCountLower = 26;
inline constexpr int kCountAlpha = 52;
inline constexpr int kCountAlnum = 62;
inline constexpr int kCountPunct = 32;
// 変換される文字数(ILIBC-CTP-001 §5.2 V5。TC-CT-05が照合する)
inline constexpr int kCountToLower = 26;
inline constexpr int kCountToUpper = 26;

}  // namespace ilibc_test::ref
