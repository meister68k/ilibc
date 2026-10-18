// hd07_ilibase_independent.cpp — TC-HD-07(3/3): ILIBC_ASSERT は NDEBUG に
// 依存せず、ILIBC_DEBUG だけで有効なこと(ILIBC-HDR-001 §4.4、
// ILIBC-AST-001 §5)。同じ翻訳単位で、無効な assert と有効な ILIBC_ASSERT を
// 並べ、どちらがフックを呼んだかを式のテキストで切り分ける。
// -O0 -DILIBC_DEBUG で th_hook.cpp と一緒につくる(終了コード42)。
#include <ilibc/config.hpp>  // ILIBC_ASSERT(ILIBC_DEBUG 定義時に有効)
#include <cassert>

// プラットフォーム層(§5.6.2)
extern "C" [[noreturn]] void ilibc_test_exit(int status);

int main() {
    // NDEBUG 定義時の assert は無効。ここで 42 で終わってはならない
#define NDEBUG
#include <cassert>
    assert(2 + 2 == 5);
    // ILIBC_ASSERT は NDEBUG とは独立に有効(HDR §4.4)。ここでフックが呼ばれる
    ILIBC_ASSERT(3 * 3 == 8);
    ilibc_test_exit(0);  // ILIBC_ASSERT が呼ばれないまま終了した(不行)
}
