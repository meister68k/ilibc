// cstdlib_sample.cpp — <cstdlib>だけを使うプログラム。
// TC-SL-17 成果物検査(-ffreestanding -fno-builtin でコンパイルし、
// <cstdlib>由来の未解決シンボルがなく(errno 変数はインライン変数なので
// 定義済み)、C++ランタイム依存や静的初期化がないこと)の対象。
// spec/ilibc_cstdlib_spec.md §9 TC-SL-17。
// 引数由来にしないとコンパイル時に畳み込まれ、コード生成と未解決シンボルの
// 検査ができなくなる。
#include <cstdlib>

namespace t = ILIBC_NS;

int main(int argc, char **) {
    // abs / labs(SL-01/02)
    const int a = t::abs((argc > 1) ? -7 : -6);
    const long l = t::labs((argc > 2) ? -70L : -60L);

    // strtol / strtoul(SL-05/06): endptr と errno の副作用を確認する形で使う
    const char *s1 = (argc > 1) ? "0x1f" : "0x20";
    const char *s2 = (argc > 2) ? "-1" : "-2";
    char *end1 = nullptr;
    char *end2 = nullptr;
    const long v1 = t::strtol(s1, &end1, 0);
    const unsigned long v2 = t::strtoul(s2, &end2, 10);
    const int consumed = (end1 != nullptr && end2 != nullptr && v1 > 0 && v2 > 0) ? 1 : 0;

    // atoi / atol(SL-03/04)
    const int i1 = t::atoi((argc > 3) ? "123" : "124");
    const long l1 = t::atol((argc > 4) ? "-9" : "-8");

    // マクロ(§2.4): 宣言互換のための定義であること
    const int exit_ok = (EXIT_SUCCESS == 0 && EXIT_FAILURE == 1) ? 1 : 0;
    const int mb = (MB_CUR_MAX == 1) ? 1 : 0;

    // 拡張(ilibc::_itoa_s): 戻り値と結果を使う
    char buf[12];
    const int r = ilibc::_itoa_s(-a, buf, 12, 10);
    const int itoa_ok = (r == 0 && buf[0] == '-') ? 1 : 0;

    // 結果を(argc非依存に畳めない形で)合算して終了コードにする
    const int hits = (a == 7) + (l == 70L) + consumed + (i1 == 123) + (l1 == -9L)
        + exit_ok + mb + itoa_ok;
    return (hits >= 8) ? 0 : 1;
}
