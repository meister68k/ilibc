// memcpy.cpp — ST-01: ILIBC_NS::memcpy の実体(ILIBC-STR-001 §5.1)。
// ヘッダの宣言の asm ラベルにより、リンク名は C の "memcpy" になる(§7.2)。
// このファイルは GCC で -fno-tree-loop-distribute-patterns を指定してビルドする
// (§3)。指定しないとループが同名関数への呼び出しに置換され、無限再帰になる。
#include <cstring>

ILIBC_NS_BEGIN

void *memcpy(void *s1, const void *s2, std::size_t n) noexcept {
    ILIBC_ASSERT(s1 != nullptr);
    ILIBC_ASSERT(s2 != nullptr);
#if defined(ILIBC_DEBUG)
    // 領域の重なり検査(§4.7): n>0 のとき、両領域の間隔が n 以上であること。
    // アドレス差を uintptr_t で引き算して比較する(オーバーフローしない)
    const std::uintptr_t a = reinterpret_cast<std::uintptr_t>(s1);
    const std::uintptr_t b = reinterpret_cast<std::uintptr_t>(s2);
    const std::uintptr_t gap = (a <= b) ? (b - a) : (a - b);
    ILIBC_ASSERT(n == 0 || gap >= static_cast<std::uintptr_t>(n));
#endif
    ilibc::detail::mem_copy_fwd(static_cast<unsigned char *>(s1),
                                 static_cast<const unsigned char *>(s2), n);
    return s1;
}

ILIBC_NS_END
