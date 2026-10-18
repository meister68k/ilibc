// st17_strlen_null.cpp — TC-ST-17(1/3): ヌルポインタに strlen を呼ぶと、
// ILIBC_DEBUG のアサーションで __ilibc_assert_fail が呼ばれること
// (ILIBC-STR-001 §4.7、TST §5.6.4: 呼ばれた場合は終了コード42)。
// -O0 -DILIBC_DEBUG で、th_hook.cpp と一緒につくる。
#include <cstring>

namespace t = ILIBC_NS;

// プラットフォーム層(§5.6.2)
extern "C" [[noreturn]] void ilibc_test_exit(int status);

int main(int argc, char **) {
    // 引数由来にしないとコンパイル時に畳み込まれ、実行時検査にならない
    const char *p = (argc > 99) ? "x" : nullptr;
    (void)t::strlen(p);  // [[nodiscard]]のため明示的に破棄。アサーションで止まる
    ilibc_test_exit(0);  // ここに来れば失敗(42にならない)
}
