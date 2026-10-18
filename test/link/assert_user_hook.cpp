// assert_user_hook.cpp — TC-HD-16(a): 利用者が強シンボルでフックを定義した
// 場合に、ilibc_hooks.a の既定弱実装ではなく、利用者の定義が使われること。
// spec/ilibc_cassert_spec.md §8 TC-HD-16(a)、ILIBC-HDR-001 §4.3・§9.1。
// リンク検査のみを目的とし、この実行ファイルは実行しない。
#include <ilibc/assert_fail.hpp>

// 利用者提供の強フック。契約のとおり復帰してはならない(HDR §4.3)。
extern "C" [[noreturn]] void __ilibc_assert_fail(const char *, const char *, int) noexcept {
    for (;;) {
        asm volatile("");
    }
}

int main() { return 0; }
