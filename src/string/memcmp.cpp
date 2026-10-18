// memcmp.cpp — ST-03: ILIBC_NS::memcmp の実体(ILIBC-STR-001 §5.1)。
// ヘッダの宣言の asm ラベルにより、リンク名は C の "memcmp" になる(§7.2)。
// このファイルは GCC で -fno-tree-loop-distribute-patterns を指定してビルドする
// (§3)。指定しないとループが同名関数への呼び出しに置換され、無限再帰になる。
#include <cstring>

ILIBC_NS_BEGIN

int memcmp(const void *s1, const void *s2, std::size_t n) noexcept {
    ILIBC_ASSERT(s1 != nullptr);
    ILIBC_ASSERT(s2 != nullptr);
    // 符号なしバイトとして比較し、最初に異なる位置の差を返す(§4.4)
    return ilibc::detail::mem_compare(static_cast<const unsigned char *>(s1),
                                       static_cast<const unsigned char *>(s2), n);
}

ILIBC_NS_END
