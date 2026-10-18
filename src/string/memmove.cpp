// memmove.cpp — ST-02: ILIBC_NS::memmove の実体(ILIBC-STR-001 §5.1)。
// ヘッダの宣言の asm ラベルにより、リンク名は C の "memmove" になる(§7.2)。
// このファイルは GCC で -fno-tree-loop-distribute-patterns を指定してビルドする
// (§3)。指定しないとループが同名関数への呼び出しに置換され、無限再帰になる。
#include <cstring>

ILIBC_NS_BEGIN

void *memmove(void *s1, const void *s2, std::size_t n) noexcept {
    ILIBC_ASSERT(s1 != nullptr);
    ILIBC_ASSERT(s2 != nullptr);
    unsigned char *d = static_cast<unsigned char *>(s1);
    const unsigned char *s = static_cast<const unsigned char *>(s2);
    // コピー先がコピー元以下なら先頭から、上なら末尾から(§4.1 平坦アドレス空間前提)
    if (reinterpret_cast<std::uintptr_t>(d) <= reinterpret_cast<std::uintptr_t>(s)) {
        ilibc::detail::mem_copy_fwd(d, s, n);
    } else {
        ilibc::detail::mem_copy_bwd(d, s, n);
    }
    return s1;
}

ILIBC_NS_END
