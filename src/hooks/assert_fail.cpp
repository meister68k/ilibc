// assert_fail.cpp — assert 失敗フックの既定実装(弱シンボル、ILIBC-HDR-001 §9.2、
// ILIBC-AST-001 §4.2)。ilibc.a には入れず、ilibc_hooks.a に格納する(§9.1)。
// 出力手段を前提とせず、停止を継続する。空の無限ループは最適化で除去される
// おそれがあるため、asm volatile("") を入れる(仕様で明示された実装上の例外)。
// 利用者が強シンボルでフックを定義した場合は、そちらが使われる。
#include <ilibc/assert_fail.hpp>

// 利用者が定義しない場合の停止動作。引数は使わない(出力手段がない)。
// [[noreturn]] の契約のとおり、呼び出し元へ戻ってはならない。
extern "C" [[noreturn]] __attribute__((weak))
void __ilibc_assert_fail(const char *, const char *, int) noexcept {
    for (;;) {
        asm volatile("");
    }
}
