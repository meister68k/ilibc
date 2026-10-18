// cstdlib_runtime.cpp — <cstdlib>のE1実行時試験(R: TC-SL-14/15、TC-SL-22〜24)。
// 先頭でブートストラップ(spec/ilibc_test_spec.md §5.8)を行い、err版・
// _itoa_s・abs を volatile 経由の短い固定データで最小検査してから、
// TC-SL-06〜11・22〜24を共有本体 sl_cases.hpp で実行する
// (spec/ilibc_cstdlib_spec.md §9)。
#include "../../common/harness/th_output.hpp"
#include "../../common/sl_cases.hpp"

// プラットフォーム層(§5.6.2)。putc は th_output.hpp が宣言する。
// [[noreturn]] の位置は e1_stdio.cpp の注記のとおり(GCCが受理する順)
extern "C" [[noreturn]] void ilibc_test_exit(int status);

// 試験対象の名前空間(T1)。E1 では std(=ilibc本体)。
namespace t = ILIBC_NS;

// ブートストラップ(§5.8): strtol・_itoa_s・abs を、volatile 経由の固定データで
// 検査する。成功で1、失敗で0を返す(呼び出し側が BOOTSTRAP FAIL・終了コード2で止める)。
static int bootstrap(void) {
    // "123" をローカル配列に置き、volatile 経由で渡す
    char s[4];
    s[0] = '1';
    s[1] = '2';
    s[2] = '3';
    s[3] = '\0';
    const volatile char *ps = s;
    const int base = sl_cases::vol(10);
    const long v = ilibc::strtol(const_cast<const char *>(ps), nullptr, base, nullptr);
    if (v != 123L) {
        return 0;
    }
    char b[12];
    int i;
    for (i = 0; i < 12; ++i) {
        b[i] = '\0';
    }
    const volatile int vr = ilibc::_itoa_s(static_cast<int>(v), b, 12, base);
    if (vr != 0) {
        return 0;
    }
    if (b[0] != '1' || b[1] != '2' || b[2] != '3' || b[3] != '\0') {
        return 0;
    }
    const volatile int va = t::abs(sl_cases::vol(-5));
    if (va != 5) {
        return 0;
    }
    return 1;
}

int main(void) {
    // まずブートストラップ(§5.8)。失敗したら以降の実行時試験は行わない
    if (!bootstrap()) {
        th_puts("BOOTSTRAP FAIL\n");
        ilibc_test_exit(2);
    }

    // TC-SL-06〜11・22〜24: ホスト差分と同じ共有本体(H: TST §5.2 単一ソース)
    const int c6 = sl_cases::tc_sl06_first_failure();
    const int c7 = sl_cases::tc_sl07_first_failure();
    const int c8 = sl_cases::tc_sl08_first_failure();
    const int c9 = sl_cases::tc_sl09_first_failure();
    const int c10 = sl_cases::tc_sl10_first_failure();
    const int c11 = sl_cases::tc_sl11_first_failure();
    const int c22 = sl_cases::tc_sl22_first_failure();
    const int c23 = sl_cases::tc_sl23_first_failure();
    const int c24 = sl_cases::tc_sl24_first_failure();
    int nfail = 0;

    if (c6 < 0) {
        th_puts("TC-SL-06 PASS\n");
    } else {
        th_puts("TC-SL-06 FAIL case=");
        th_put_int(c6);
        th_putc('\n');
        ++nfail;
    }
    if (c7 < 0) {
        th_puts("TC-SL-07 PASS\n");
    } else {
        th_puts("TC-SL-07 FAIL case=");
        th_put_int(c7);
        th_putc('\n');
        ++nfail;
    }
    if (c8 < 0) {
        th_puts("TC-SL-08 PASS\n");
    } else {
        th_puts("TC-SL-08 FAIL case=");
        th_put_int(c8);
        th_putc('\n');
        ++nfail;
    }
    if (c9 < 0) {
        th_puts("TC-SL-09 PASS\n");
    } else {
        th_puts("TC-SL-09 FAIL case=");
        th_put_int(c9);
        th_putc('\n');
        ++nfail;
    }
    if (c10 < 0) {
        th_puts("TC-SL-10 PASS\n");
    } else {
        th_puts("TC-SL-10 FAIL case=");
        th_put_int(c10);
        th_putc('\n');
        ++nfail;
    }
    if (c11 < 0) {
        th_puts("TC-SL-11 PASS\n");
    } else {
        th_puts("TC-SL-11 FAIL case=");
        th_put_int(c11);
        th_putc('\n');
        ++nfail;
    }
    if (c22 < 0) {
        th_puts("TC-SL-22 PASS\n");
    } else {
        th_puts("TC-SL-22 FAIL case=");
        th_put_int(c22);
        th_putc('\n');
        ++nfail;
    }
    if (c23 < 0) {
        th_puts("TC-SL-23 PASS\n");
    } else {
        th_puts("TC-SL-23 FAIL case=");
        th_put_int(c23);
        th_putc('\n');
        ++nfail;
    }
    if (c24 < 0) {
        th_puts("TC-SL-24 PASS\n");
    } else {
        th_puts("TC-SL-24 FAIL case=");
        th_put_int(c24);
        th_putc('\n');
        ++nfail;
    }

    th_puts("SUMMARY pass=");
    th_put_int(9 - nfail);
    th_puts(" fail=");
    th_put_int(nfail);
    th_putc('\n');
    ilibc_test_exit((nfail == 0) ? 0 : 1);
}
