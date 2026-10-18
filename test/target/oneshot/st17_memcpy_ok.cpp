// st17_memcpy_ok.cpp — TC-ST-17(3/3): 重ならない memcpy では
// __ilibc_assert_fail が呼ばれず、正常終了すること(呼ばれてはならないケース、
// TST §5.6.4: 終了コード0)。-O0 -DILIBC_DEBUG で th_hook.cpp と一緒につくる。
#include <cstring>

namespace t = ILIBC_NS;

// プラットフォーム層(§5.6.2)
extern "C" [[noreturn]] void ilibc_test_exit(int status);

int main(int argc, char **) {
    unsigned char a[16];
    unsigned char b[16];
    for (int i = 0; i < 16; ++i) {
        b[i] = static_cast<unsigned char>(i * 5 + 3);
        a[i] = 0xA5;
    }
    const std::size_t n = (argc > 99) ? static_cast<std::size_t>(8) : static_cast<std::size_t>(16);
    t::memcpy(a, b, n);  // 別配列同士: 重ならない
    for (int i = 0; i < 16; ++i) {
        if (a[i] != b[i]) {
            ilibc_test_exit(1);  // コピー結果が壊れている
        }
    }
    ilibc_test_exit(0);  // フックが呼ばれないこと
}
