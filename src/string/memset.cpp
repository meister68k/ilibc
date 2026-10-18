// memset.cpp — ST-05: ILIBC_NS::memset の実体(ILIBC-STR-001 §5.1)。
// ヘッダの宣言の asm ラベルにより、リンク名は C の "memset" になる(§7.2)。
// このファイルは GCC で -fno-tree-loop-distribute-patterns を指定してビルドする
// (§3)。指定しないとループが同名関数への呼び出しに置換され、無限再帰になる。
#include <cstring>

ILIBC_NS_BEGIN

void *memset(void *s, int c, std::size_t n) noexcept {
    ILIBC_ASSERT(s != nullptr);
    ilibc::detail::mem_fill(static_cast<unsigned char *>(s),
                             static_cast<unsigned char>(c), n);
    return s;
}

ILIBC_NS_END
