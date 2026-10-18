// hd07_reenable.cpp — TC-HD-06(a)/TC-HD-07(1/3): NDEBUG の定義・解除と
// 再インクルードで、assert が無効化・再有効化されることを検証する
// (ILIBC-AST-001 §3.3、ILIBC-HDR-001 §4.1)。-O0 -DILIBC_DEBUG で
// th_hook.cpp と一緒につくる(oneshot、TST §5.6.4: 終了コード42)。
#include <cassert>

// プラットフォーム層(§5.6.2)。[[noreturn]] の位置は e1_stdio.cpp の注記のとおり
extern "C" [[noreturn]] void ilibc_test_exit(int status);

int main() {
    int n = 0;

    // NDEBUG 定義後の再インクルードで無効化され、副作用が1回も起きないこと
#define NDEBUG
#include <cassert>
    assert(++n);
    if (n != 0) {
        ilibc_test_exit(1);  // 無効時にも式が評価された
    }

    // #undef と再インクルードで再び有効になり、偽の assert がフックを呼ぶこと。
    // 無効のままなら終了0で、このケースは不合格になる。
#undef NDEBUG
#include <cassert>
    assert(1 == 2);
    ilibc_test_exit(0);  // フックが呼ばれないまま終了した(不行)
}
