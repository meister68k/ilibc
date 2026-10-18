// cctype_host.cpp — <cctype>のホストlibc差分試験(H: TC-CT-07)。
// spec/ilibc_cctype_spec.md §8、spec/ilibc_test_spec.md §5.7。
// ビルド: -m32 -DILIBC_HOST_TEST -U_FORTIFY_SOURCE。
// 本体は ilibc_under_test 名前空間、オラクルはホストlibcの実関数(-1〜255のみ比較)。
// <clocale>/<cstdio> はスタブされるため、素通りしてコンパイラ付属を使う
// (ヘッダ仕様 §7.3 S3。ホスト施設を使う試験コードだけの設定)。
#define ILIBC_STUB_PASSTHROUGH
#include <cctype>   // 本ライブラリ(ILIBC_HOST_TEST で ilibc_under_test)
#include <clocale>  // setlocale
#include <cstdio>   // 失敗の報告(ホスト専用の試験プログラムのため stdio を使う)

// 試験対象の名前空間(T1)
namespace t = ILIBC_NS;

// ホストlibcの分類・変換関数。
// ホストの <ctype.h>/<cctype> を含めると、関数形式マクロが本ライブラリの
// 呼び出しを乗っ取るため、マクロを引入れず C の実関数を宣言して直接呼ぶ
// (glibc は13関数すべてを動的・静的にエクスポートしている)。
extern "C" {
int isalnum(int);
int isalpha(int);
int iscntrl(int);
int isdigit(int);
int isgraph(int);
int islower(int);
int isprint(int);
int ispunct(int);
int isspace(int);
int isupper(int);
int isxdigit(int);
int tolower(int);
int toupper(int);
}

// 失敗数と、最初の20件までの不一致(ホスト専用のため stdio で報告)
static int g_fail = 0;
static int g_printed = 0;

// 関数名 fn と値 c の不一致を1件記録する
static void report(const char *fn, int c) {
    ++g_fail;
    if (g_printed < 20) {
        ++g_printed;
        std::printf("TC-CT-07 FAIL %s c=%d\n", fn, c);
    }
}

int main(void) {
    // "C"ロケール固定(ILIBC-CTP-001 §3.1、ILIBC-TST-001 §5.7.1)
    std::setlocale(LC_ALL, "C");

    for (int c = -1; c < 256; ++c) {
        // 分類11関数: bool と ホストの 0/非0 を比較(ILIBC-TST-001 §5.7.2 #3)
        if (t::isalnum(c) != (::isalnum(c) != 0)) report("isalnum", c);
        if (t::isalpha(c) != (::isalpha(c) != 0)) report("isalpha", c);
        if (t::iscntrl(c) != (::iscntrl(c) != 0)) report("iscntrl", c);
        if (t::isdigit(c) != (::isdigit(c) != 0)) report("isdigit", c);
        if (t::isgraph(c) != (::isgraph(c) != 0)) report("isgraph", c);
        if (t::islower(c) != (::islower(c) != 0)) report("islower", c);
        if (t::isprint(c) != (::isprint(c) != 0)) report("isprint", c);
        if (t::ispunct(c) != (::ispunct(c) != 0)) report("ispunct", c);
        if (t::isspace(c) != (::isspace(c) != 0)) report("isspace", c);
        if (t::isupper(c) != (::isupper(c) != 0)) report("isupper", c);
        if (t::isxdigit(c) != (::isxdigit(c) != 0)) report("isxdigit", c);
        // 変換2関数: 値をそのまま比較
        if (t::tolower(c) != ::tolower(c)) report("tolower", c);
        if (t::toupper(c) != ::toupper(c)) report("toupper", c);
    }

    if (g_fail == 0) {
        std::printf("TC-CT-07 PASS\n");
    }
    std::printf("SUMMARY pass=%d fail=%d\n", g_fail == 0 ? 1 : 0, g_fail == 0 ? 0 : 1);
    return g_fail == 0 ? 0 : 1;
}
