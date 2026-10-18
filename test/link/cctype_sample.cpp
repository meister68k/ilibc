// cctype_sample.cpp — <cctype>だけを使う最小プログラム(TC-CT-11 成果物検査の対象)。
// spec/ilibc_cctype_spec.md §8: リンク結果に (a)<cctype>由来のシンボル、
// (b)由来の定数データ、(c)C++ランタイム依存・静的初期化がないこと。
#include <cctype>

int main(int argc, char **) {
    // 引数由来にしないとコンパイル時に畳み込まれ、コード生成が検査できない
    const int c = (argc > 1) ? static_cast<int>('B') : static_cast<int>('A');
    return (ILIBC_NS::isalpha(c) && ILIBC_NS::isalnum(c) && !ILIBC_NS::isspace(c)) ? 0 : 1;
}
