// assert_sample.cpp — TC-HD-16 のリンク検査用サンプル。assert を有効のまま
// (NDEBUG なし)で使うため、__ilibc_assert_fail への参照が残る。
// spec/ilibc_cassert_spec.md §8 TC-HD-16(b)(c)。
#include <cassert>

int main(int argc, char **) {
    // 実行時値にしないとコンパイル時に畳み込まれ、フック参照が消える
    assert(argc == 42);
    return 0;
}
