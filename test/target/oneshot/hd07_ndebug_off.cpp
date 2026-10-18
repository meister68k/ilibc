// hd07_ndebug_off.cpp — TC-HD-06(b)/TC-HD-07(2/3): NDEBUG 定義時の assert は
// 式を1回も評価せず、フックも呼ばないことを検証する(ILIBC-AST-001 §3.1)。
// -O0 -DILIBC_DEBUG で th_hook.cpp と一緒につくる(oneshot、TST §5.6.4:
// 終了コード0。フックが呼ばれてはならないケース)。
#include <cassert>

// プラットフォーム層(§5.6.2)
extern "C" [[noreturn]] void ilibc_test_exit(int status);

int main() {
    int n = 0;

    // NDEBUG 定義後の再インクルードで無効化される(AST §3.3)
#define NDEBUG
#include <cassert>
    assert(++n);  // 無効時の非評価(TC-HD-06(b))
    if (n != 0) {
        ilibc_test_exit(1);  // 副作用が起きてしまった
    }
    // 無効ならフックを呼ばずに通り、有効なら(偽のため)42で終わる
    assert(n == 1);
    ilibc_test_exit(0);  // フックが呼ばれないこと
}
