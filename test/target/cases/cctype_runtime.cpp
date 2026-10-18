// cctype_runtime.cpp — <cctype>の実行時試験(R: TC-CT-08)。
// spec/ilibc_cctype_spec.md §8: TC-CT-01・TC-CT-02を実行時にも行い、
// constexpr の評価と実行時のコード生成の食い違いを検出する。
// 引数は Opaque(volatile 経由)で渡して畳み込みを避ける(T3)。
#include "../../common/ctype_cases.hpp"
#include "../../common/harness/th_output.hpp"

// プラットフォーム層(§5.6.2)。putc は th_output.hpp が宣言する。
// [[noreturn]] の位置は e1_stdio.cpp の注記のとおり(GCCが受理する順)
extern "C" [[noreturn]] void ilibc_test_exit(int status);

// TC-CT-08: 13関数の全257値照合(0〜255の本体と c=-1)をOpaqueで実行し、
// 失敗した検査を1行ずつ出力する。戻りは失敗した検査数
static int run_tc_ct01(void) {
    int failed = 0;
    for (int which = 0; which < kFuncCount; ++which) {
        if (!tc_ct01_neg1_ok(Opaque{}, which)) {
            th_puts("TC-CT-08 FAIL ct01.neg1 which=");
            th_put_int(which);
            th_putc('\n');
            ++failed;
        }
        const int v = tc_ct01_first_failure(Opaque{}, which);
        if (v != -1) {
            th_puts("TC-CT-08 FAIL ct01 which=");
            th_put_int(which);
            th_puts(" value=");
            th_put_int(v);
            th_putc('\n');
            ++failed;
        }
    }
    return failed;
}

// TC-CT-02: 極端値・ビット別名化をOpaqueで実行する。戻りは失敗数(0または1)
static int run_tc_ct02(void) {
    const int v = tc_ct02_first_failure(Opaque{});
    if (v == -1) {
        return 0;
    }
    th_puts("TC-CT-08 FAIL ct02 value=");
    th_put_int(v);
    th_putc('\n');
    return 1;
}

int main(void) {
    // 出力・終了はプラットフォーム層経由。ilibc の関数は使わない(R1)
    const int failed = run_tc_ct01() + run_tc_ct02();
    if (failed == 0) {
        th_puts("TC-CT-08 PASS\n");
        th_puts("SUMMARY pass=1 fail=0\n");
        ilibc_test_exit(0);
    }
    th_puts("SUMMARY pass=0 fail=1\n");
    ilibc_test_exit(1);
}
