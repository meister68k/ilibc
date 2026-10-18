// memchr.cpp — ST-04: ilibc::detail::memchr_base の実体(ILIBC-STR-001 §5.1/§7.2)。
// asm ラベルにより、リンク名は C の "memchr" になる。std:: の2オーバーロードは
// ヘッダ側の inline ラッパで、この実体を呼ぶ。
// このファイルは GCC で -fno-tree-loop-distribute-patterns を指定してビルドする
// (§3)。指定しないとループが memchr 呼び出しに置換され、無限再帰になる。
#include <cstring>

namespace ilibc::detail {

void *memchr_base(const void *s, int c, std::size_t n) noexcept {
    ILIBC_ASSERT(s != nullptr);  // 2つの std:: オーバーロード共通の引数検査(§4.7)
    const unsigned char *p = mem_find(static_cast<const unsigned char *>(s),
                                       static_cast<unsigned char>(c), n);
    return const_cast<unsigned char *>(p);
}

}  // namespace ilibc::detail
