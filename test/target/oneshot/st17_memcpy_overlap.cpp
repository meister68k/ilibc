// st17_memcpy_overlap.cpp — TC-ST-17(2/3): 重なる領域への memcpy で
// ILIBC_DEBUG のアサーションで __ilibc_assert_fail が呼ばれること
// (ILIBC-STR-001 §4.7、TST §5.6.4: 呼ばれた場合は終了コード42)。
// -O0 -DILIBC_DEBUG で、th_hook.cpp と一緒につくる。
#include <cstring>

namespace t = ILIBC_NS;

// プラットフォーム層(§5.6.2)
extern "C" [[noreturn]] void ilibc_test_exit(int status);

int main(int argc, char **) {
    unsigned char buf[16];
    for (int i = 0; i < 16; ++i) {
        buf[i] = static_cast<unsigned char>(i + 1);
    }
    // 重なり幅は引数由来にしないとコンパイル時に畳み込まれない
    const std::size_t n = (argc > 99) ? static_cast<std::size_t>(4) : static_cast<std::size_t>(8);
    t::memcpy(buf + 1, buf, n);  // gap=1 < n: 重なり → アサーションで止まる
    ilibc_test_exit(0);          // ここに来れば失敗(42にならない)
}
