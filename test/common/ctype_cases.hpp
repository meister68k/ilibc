// ctype_cases.hpp — <cctype>の単一ソース試験本体(Direct/Opaque 共通)。
// spec/ilibc_test_spec.md §5.2(T1〜T6)と spec/ilibc_cctype_spec.md §8(TC-CT-01〜05)に従う。
// コンパイル時(test/constexpr)と実行時(test/target)は、この本体を引数の渡し方だけ変えて共有する。
#pragma once

#include <climits>
#include <cctype>
#include <ilibc/config.hpp>
#include "oracle/ctype_ref.hpp"

// 試験対象の名前空間(std、またはホスト差分の ilibc_under_test)(T1)
namespace t = ILIBC_NS;
// 独立参照の省略名(仕様の記述に合わせる)
namespace ref = ilibc_test::ref;

// ------------------------------------------------- 引数の渡し方(T2/T3)

// 値をそのまま返す(コンパイル時の試験用)
struct Direct {
    constexpr int operator()(int x) const { return x; }
};

// volatile 経由で値を隠し、最適化によるコンパイル時の畳み込みを防ぐ(実行時の試験用)
struct Opaque {
    int operator()(int x) const { volatile int v = x; return v; }
};

// 関数番号の個数(0=isalnum … 12=toupper、ILIBC-CTP-001 §5.1/§5.2の順)
inline constexpr int kFuncCount = 13;

// ------------------------------------------------- 共通の補助

// 0〜127のうち pred が真になる値の個数を数える(TC-CT-03)
template <class Id, class Pred>
constexpr int count_true(Id id, Pred pred) {
    int n = 0;
    for (int c = 0; c <= 127; ++c) {
        if (pred(id(c))) ++n;
    }
    return n;
}

// 11の分類関数がすべて偽か(TC-CT-02)
constexpr bool classify_all_false(int x) {
    return !t::isalnum(x) && !t::isalpha(x) && !t::iscntrl(x) && !t::isdigit(x)
        && !t::isgraph(x) && !t::islower(x) && !t::isprint(x) && !t::ispunct(x)
        && !t::isspace(x) && !t::isupper(x) && !t::isxdigit(x);
}

// 変換2関数が入力値をそのまま返すか(TC-CT-02、TC-CT-05)
constexpr bool convert_identity(int x) {
    return t::tolower(x) == x && t::toupper(x) == x;
}

// 値 x で関係式R1〜R9(ILIBC-CTP-001 §3.3)が成立するか(TC-CT-04)
constexpr bool relations_hold(int x) {
    const bool upper = t::isupper(x);
    const bool lower = t::islower(x);
    const bool alpha = t::isalpha(x);
    const bool digit = t::isdigit(x);
    const bool alnum = t::isalnum(x);
    const bool xdigit = t::isxdigit(x);
    const bool space = t::isspace(x);
    const bool cntrl = t::iscntrl(x);
    const bool punct = t::ispunct(x);
    const bool graph = t::isgraph(x);
    const bool print = t::isprint(x);

    if (alpha != (upper || lower)) return false;               // R1
    if (alnum != (alpha || digit)) return false;               // R2
    if (graph != (alnum || punct)) return false;               // R3
    if (print != (graph || x == ' ')) return false;            // R4
    if (upper && lower) return false;                          // R5(互いに排他)
    if (upper && digit) return false;
    if (upper && punct) return false;
    if (lower && digit) return false;
    if (lower && punct) return false;
    if (digit && punct) return false;
    if (xdigit && !(digit || alpha)) return false;             // R6
    if (cntrl && print) return false;                          // R7
    if (space && x != ' ' && !cntrl) return false;             // R8
    // R9: 領域外(c<0 または c>127)なら分類11関数がすべて偽
    // (仕様の「13の分類関数」は分類関数11個の意。変換2関数の対応は V3 で検査する)
    if ((x < 0 || x > 127)
        && (alnum || alpha || cntrl || digit || graph || lower || print
            || punct || space || upper || xdigit)) {
        return false;
    }
    return true;
}

// 値 x で変換の性質V1〜V4(ILIBC-CTP-001 §5.2)が成立するか(TC-CT-05)
constexpr bool conversion_props_hold(int x) {
    const int lo = t::tolower(x);
    const int up = t::toupper(x);
    if (t::isupper(x) && !(lo == x + 32 && t::islower(lo))) return false;  // V1
    if (t::islower(x) && !(up == x - 32 && t::isupper(up))) return false;  // V2
    if (!t::isupper(x) && !t::islower(x) && (lo != x || up != x)) return false;  // V3
    if (t::isalpha(x) && !(t::tolower(up) == lo && t::toupper(lo) == up)) return false;  // V4
    return true;
}

// 関数番号 which(0〜12)の関数を引数 x に適用した結果と、
// 独立参照が生値 raw に対して返す結果が一致するか(TC-CT-01)
constexpr bool ct01_pair(int which, int x, int raw) {
    switch (which) {
    case 0:  return t::isalnum(x) == ref::is_alnum(raw);
    case 1:  return t::isalpha(x) == ref::is_alpha(raw);
    case 2:  return t::iscntrl(x) == ref::is_cntrl(raw);
    case 3:  return t::isdigit(x) == ref::is_digit(raw);
    case 4:  return t::isgraph(x) == ref::is_graph(raw);
    case 5:  return t::islower(x) == ref::is_lower(raw);
    case 6:  return t::isprint(x) == ref::is_print(raw);
    case 7:  return t::ispunct(x) == ref::is_punct(raw);
    case 8:  return t::isspace(x) == ref::is_space(raw);
    case 9:  return t::isupper(x) == ref::is_upper(raw);
    case 10: return t::isxdigit(x) == ref::is_xdigit(raw);
    case 11: return t::tolower(x) == ref::to_lower(raw);
    case 12: return t::toupper(x) == ref::to_upper(raw);
    default: return false;  // 関数番号が範囲外(呼び出し側の誤り)
    }
}

// TC-CT-02 の極端値集合(ILIBC-CTP-001 §8)
inline constexpr int kExtremes[] = {INT_MIN, INT_MIN + 1, -256, -2, 256, INT_MAX};

// 0〜255の各cについて、下位ビットだけを見る実装の誤りを検出するビット別名化値(4種)を渡す
// (c | 0x80000000 は unsigned 経由で int に変換する。ILIBC-CTP-001 §8 TC-CT-02)
template <class F>
constexpr void for_each_alias(F f) {
    for (int c = 0; c < 256; ++c) {
        f(c + 256);
        f(c + 0x10000);
        f(c + 0x01000000);
        f(static_cast<int>(static_cast<unsigned>(c) | 0x80000000u));
    }
}

// ------------------------------------------------- TC-CT-01: 全257値の照合

// which関数の最初の不一致値(c=0〜255)を返す(-1なら一致)。
// c=-1 は成功標識(-1)と失敗値が衝突するため、tc_ct01_neg1_okで個別に検査する。
template <class Id>
constexpr int tc_ct01_first_failure(Id id, int which) {
    for (int c = 0; c < 256; ++c) {
        if (!ct01_pair(which, id(c), c)) return c;
    }
    return -1;
}

// which関数を c=-1(EOF)で独立参照と照合する(TC-CT-01 の -1 の分。
// 失敗しても成功標識と区別できないため、戻り値を bool にして衝突を避ける)
template <class Id>
constexpr bool tc_ct01_neg1_ok(Id id, int which) {
    return ct01_pair(which, id(-1), -1);
}

// ------------------------------------------------- TC-CT-02: 極端値・別名化

// 極端値・ビット別名化値で、分類がすべて偽・変換が恒等か(最初の失敗値、-1なら合格)
// 入力集合に -1 は含まれないため、成功標識(-1)と衝突しない
template <class Id>
constexpr int tc_ct02_first_failure(Id id) {
    for (int e : kExtremes) {
        const int x = id(e);
        if (!classify_all_false(x) || !convert_identity(x)) return x;
    }
    int result = -1;
    for_each_alias([&](int v) {
        if (result != -1) return;  // 既に失敗あり
        const int x = id(v);
        if (!classify_all_false(x) || !convert_identity(x)) result = x;
    });
    return result;
}

// ------------------------------------------------- TC-CT-03: 個数の照合

// 0〜127の個数が仕様の個数表と一致するか。
// 戻り値: -1(全一致)、0〜12=個数が一致しなかった関数番号(isalnum…toupper の順)
template <class Id>
constexpr int tc_ct03_first_failure(Id id) {
    if (count_true(id, [](int x) { return t::isalnum(x); }) != ref::kCountAlnum) return 0;
    if (count_true(id, [](int x) { return t::isalpha(x); }) != ref::kCountAlpha) return 1;
    if (count_true(id, [](int x) { return t::iscntrl(x); }) != ref::kCountCntrl) return 2;
    if (count_true(id, [](int x) { return t::isdigit(x); }) != ref::kCountDigit) return 3;
    if (count_true(id, [](int x) { return t::isgraph(x); }) != ref::kCountGraph) return 4;
    if (count_true(id, [](int x) { return t::islower(x); }) != ref::kCountLower) return 5;
    if (count_true(id, [](int x) { return t::isprint(x); }) != ref::kCountPrint) return 6;
    if (count_true(id, [](int x) { return t::ispunct(x); }) != ref::kCountPunct) return 7;
    if (count_true(id, [](int x) { return t::isspace(x); }) != ref::kCountSpace) return 8;
    if (count_true(id, [](int x) { return t::isupper(x); }) != ref::kCountUpper) return 9;
    if (count_true(id, [](int x) { return t::isxdigit(x); }) != ref::kCountXdigit) return 10;
    if (count_true(id, [](int x) { return t::tolower(x) != x; }) != ref::kCountToLower) return 11;
    if (count_true(id, [](int x) { return t::toupper(x) != x; }) != ref::kCountToUpper) return 12;
    return -1;
}

// ------------------------------------------------- TC-CT-04: 関係式 R1-R9

// c=0〜255・極端値・ビット別名化値で関係式が成立するか(最初の失敗値、-1なら成立)
// c=-1 は tc_ct04_neg1_ok で個別に検査するため、本体は 0 から数える
// (失敗値 -1 と成功標識 -1 の衝突を避ける)
template <class Id>
constexpr int tc_ct04_first_failure(Id id) {
    for (int c = 0; c < 256; ++c) {
        if (!relations_hold(id(c))) return c;
    }
    // 仕様: R1〜R9は「−1〜255、および TC-CT-02の値」で検証する
    for (int e : kExtremes) {
        const int x = id(e);
        if (!relations_hold(x)) return x;
    }
    int result = -1;
    for_each_alias([&](int v) {
        if (result != -1) return;
        const int x = id(v);
        if (!relations_hold(x)) result = x;
    });
    return result;
}

// 関係式を c=-1(EOF)で検査する(TC-CT-04 の -1 の分。bool で返して標識衝突を避ける)
template <class Id>
constexpr bool tc_ct04_neg1_ok(Id id) {
    return relations_hold(id(-1));
}

// ------------------------------------------------- TC-CT-05: 性質 V1-V5

// TC-CT-05 の戻り値: V5(個数)の失敗を示す(入力値集合に存在しない値)
inline constexpr int kCountFailure = 0x40000000;

// 変換の性質V1〜V5と領域外入力の恒等性(最初の失敗値、-1なら合格。
// kCountFailure なら V5 の個数不一致。c=-1 は tc_ct05_neg1_ok で個別に検査する)
template <class Id>
constexpr int tc_ct05_first_failure(Id id) {
    // V1〜V4: c=0〜255
    for (int c = 0; c < 256; ++c) {
        if (!conversion_props_hold(id(c))) return c;
    }
    // V5: 変換される文字が各26個か(変換対象は 0〜127 に収まるので 0〜255で数える)
    int n_lower = 0;
    int n_upper = 0;
    for (int c = 0; c < 256; ++c) {
        const int x = id(c);
        if (t::tolower(x) != x) ++n_lower;
        if (t::toupper(x) != x) ++n_upper;
    }
    if (n_lower != ref::kCountToLower || n_upper != ref::kCountToUpper) return kCountFailure;
    // 領域外入力(極端値・ビット別名化値)がそのまま返ること
    for (int e : kExtremes) {
        const int x = id(e);
        if (!convert_identity(x)) return x;
    }
    int result = -1;
    for_each_alias([&](int v) {
        if (result != -1) return;
        const int x = id(v);
        if (!convert_identity(x)) result = x;
    });
    return result;
}

// 変換の性質と恒等性を c=-1(EOF)で検査する(TC-CT-05 の -1 の分。V3が恒等性を含む)
template <class Id>
constexpr bool tc_ct05_neg1_ok(Id id) {
    const int x = id(-1);
    return conversion_props_hold(x) && convert_identity(x);
}
