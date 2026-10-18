// format_runtime.cpp — <ilibc/format.hpp> のE1実行時試験(R: TC-IO-02/04/05/06/07)。
// spec/ilibc_cstdio_spec.md §3・§6・§7 を検証する。書式はコンパイル時解析に
// よって受理され、実行時は戻り値・切詰め・NUL終端・2パスの不変性を確かめる。
// 出力・終了は §5.6.2 のプラットフォーム層(e1_stdio)に任せ、照合はこの
// ファイル内の比較のみにとどめる(R1: ライブラリ・ホスト文字列関数に依存しない)。
#include "../../common/harness/th_output.hpp"
#include <ilibc/format.hpp>
#include <cstddef>

// プラットフォーム層(§5.6.2)。[[noreturn]] の位置は e1_stdio.cpp の注記のとおり
extern "C" [[noreturn]] void ilibc_test_exit(int status);

// NUL終端文字列 a と b が一致するか(ホスト/ilibc の文字列関数を使わない)
static int eq(const char *a, const char *b) {
    while (*a != '\0' && *a == *b) {
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

// p の先頭 n バイトを c で埋める(カナリア設定に使う)
static void fill(char *p, std::size_t n, char c) {
    for (std::size_t i = 0; i < n; ++i) {
        p[i] = c;
    }
}

// TC-IO-02: %d/%s/%%/空書式/多引数(成功0、失敗は最初の検査番号)
static int tc_io02(void) {
    char b[32];

    // %d・%s・%% の混合と、必要な長さ(NUL除く)の戻り値
    if (ilibc::sprintf<"a=%d b=%s %%">(b, 42, "hi") != 11) { return 1; }
    if (!eq(b, "a=42 b=hi %")) { return 2; }
    // 空書式: 何も出さず、先頭にNULを置いて0を返す
    if (ilibc::sprintf<"">(b) != 0) { return 3; }
    if (b[0] != '\0') { return 4; }
    // 多引数: 個数・順序が一致すること
    if (ilibc::sprintf<"%d/%s/%d/%s">(b, -7, "x", 0, "yy") != 9) { return 5; }
    if (!eq(b, "-7/x/0/yy")) { return 6; }
    // snprint_f でも同じ書式が働く
    if (ilibc::snprint_f<"p=%d">(b, sizeof b, 5) != 3) { return 7; }
    if (!eq(b, "p=5")) { return 8; }
    // %% は出力上1バイトの % になる
    if (ilibc::sprintf<"[%%]">(b) != 3) { return 9; }
    if (!eq(b, "[%]")) { return 10; }
    return 0;
}

// TC-IO-04: %d の INT_MIN/INT_MAX/負数/0(ILIBC-IO-001 §6)
static int tc_io04(void) {
    char b[16];

    if (ilibc::sprintf<"%d">(b, 0) != 1 || !eq(b, "0")) { return 1; }
    if (ilibc::sprintf<"%d">(b, -1) != 2 || !eq(b, "-1")) { return 2; }
    if (ilibc::sprintf<"%d">(b, 2147483647) != 10 || !eq(b, "2147483647")) { return 3; }
    if (ilibc::sprintf<"%d">(b, -2147483647 - 1) != 11 || !eq(b, "-2147483648")) { return 4; }
    return 0;
}

// TC-IO-05: %s の空文字列/長い文字列(40文字)/ポインタ変数(ILIBC-IO-001 §3)
static int tc_io05(void) {
    char b[64];
    char word[41];
    const char *pw;

    // 空文字列は長さ0として扱われる
    if (ilibc::sprintf<"[%s]">(b, "") != 2 || !eq(b, "[]")) { return 1; }
    // 長い文字列: "s=" + 40文字
    fill(word, 40, 'a');
    word[40] = '\0';
    if (ilibc::sprintf<"s=%s">(b, word) != 42) { return 2; }
    if (b[0] != 's' || b[1] != '=') { return 3; }
    for (int i = 0; i < 40; ++i) {
        if (b[2 + i] != 'a') { return 4; }
    }
    if (b[42] != '\0') { return 5; }
    // char* 変数も %s に渡せる(変換可能であること)
    pw = word;
    if (ilibc::snprint_f<"%s">(b, sizeof b, pw) != 40) { return 6; }
    if (!eq(b, word)) { return 7; }
    return 0;
}

// TC-IO-06: snprint_f の容量0/1・切詰め・NUL・必要長(ILIBC-IO-001 §7)
static int tc_io06(void) {
    char t[8];

    // capacity==0: 何も書かず、dst==nullptr も許容され、必要長を返す
    if (ilibc::snprint_f<"abc">(nullptr, 0) != 3) { return 1; }
    // capacity==1: NULのみを置き、後ろは変えない。戻り値は切り詰められない
    fill(t, sizeof t, 'Q');
    if (ilibc::snprint_f<"abc">(t, 1) != 3) { return 2; }
    if (t[0] != '\0' || t[1] != 'Q') { return 3; }
    // 切り詰め: 最大 capacity-1 文字+NUL、外側カナリアは不変
    fill(t, sizeof t, 'Q');
    if (ilibc::snprint_f<"abcdefg">(t, 4) != 7) { return 4; }
    if (!eq(t, "abc")) { return 5; }
    if (t[4] != 'Q' || t[7] != 'Q') { return 6; }
    // ちょうど収まる(capacity == 必要長+1)
    fill(t, sizeof t, 'Q');
    if (ilibc::snprint_f<"abc">(t, 4) != 3) { return 7; }
    if (!eq(t, "abc") || t[4] != 'Q') { return 8; }
    // 1バイト不足でも必要長は完全に返る(内容だけ切られる)
    fill(t, sizeof t, 'Q');
    if (ilibc::snprint_f<"abc">(t, 3) != 3) { return 9; }
    if (!eq(t, "ab")) { return 10; }
    return 0;
}

// TC-IO-07: 安全 sprintf の容量不足時不変・境界・2パス・引数1回評価
// (ILIBC-IO-001 §7)
static int tc_io07(void) {
    char c6[6];
    char c5[5];
    int cnt = 0;

    // 容量不足 → -1 で配列を一切変えない(全6バイトがカナリアのまま)
    fill(c6, sizeof c6, 'Q');
    if (ilibc::sprintf<"123456">(c6) != -1) { return 1; }
    for (std::size_t i = 0; i < sizeof c6; ++i) {
        if (c6[i] != 'Q') { return 2; }
    }
    // 必要長+1 == N の境界では成功する
    fill(c6, sizeof c6, 'Q');
    if (ilibc::sprintf<"12345">(c6) != 5) { return 3; }
    if (!eq(c6, "12345")) { return 4; }
    // N == 必要長(終端1バイト分不足)でも -1・不変
    fill(c5, sizeof c5, 'Q');
    if (ilibc::sprintf<"12345">(c5) != -1) { return 5; }
    for (std::size_t i = 0; i < sizeof c5; ++i) {
        if (c5[i] != 'Q') { return 6; }
    }
    // 引数式は各1回だけ評価される(2パスでも式そのものは再評価しない)
    if (ilibc::sprintf<"%d">(c6, ++cnt) != 1) { return 7; }
    if (cnt != 1 || !eq(c6, "1")) { return 8; }
    return 0;
}

int main(void) {
    int nfail = 0;

    // 各TCの最初の失敗箇所を番号で報告する
    const int r2 = tc_io02();
    if (r2 == 0) {
        th_puts("TC-IO-02 PASS\n");
    } else {
        th_puts("TC-IO-02 FAIL case=");
        th_put_int(r2);
        th_putc('\n');
        ++nfail;
    }
    const int r4 = tc_io04();
    if (r4 == 0) {
        th_puts("TC-IO-04 PASS\n");
    } else {
        th_puts("TC-IO-04 FAIL case=");
        th_put_int(r4);
        th_putc('\n');
        ++nfail;
    }
    const int r5 = tc_io05();
    if (r5 == 0) {
        th_puts("TC-IO-05 PASS\n");
    } else {
        th_puts("TC-IO-05 FAIL case=");
        th_put_int(r5);
        th_putc('\n');
        ++nfail;
    }
    const int r6 = tc_io06();
    if (r6 == 0) {
        th_puts("TC-IO-06 PASS\n");
    } else {
        th_puts("TC-IO-06 FAIL case=");
        th_put_int(r6);
        th_putc('\n');
        ++nfail;
    }
    const int r7 = tc_io07();
    if (r7 == 0) {
        th_puts("TC-IO-07 PASS\n");
    } else {
        th_puts("TC-IO-07 FAIL case=");
        th_put_int(r7);
        th_putc('\n');
        ++nfail;
    }
    th_puts("SUMMARY pass=");
    th_put_int(5 - nfail);
    th_puts(" fail=");
    th_put_int(nfail);
    th_putc('\n');
    ilibc_test_exit((nfail == 0) ? 0 : 1);
}
