// th_hook.cpp — oneshot 試験用の __ilibc_assert_fail フック(TST §5.6.4)。
// 式のテキスト・ファイル名・行番号を出力して、終了コード42で終わる。
// 各ケースは別実行ファイルとし、呼ばれるべきでないケースは0で終わる。
#include "th_output.hpp"
#include <ilibc/assert_fail.hpp>

// プラットフォーム層(§5.6.2)。[[noreturn]] の位置は e1_stdio.cpp の注記のとおり
extern "C" [[noreturn]] void ilibc_test_exit(int status);

// ILIBC_DEBUG のアサーション失敗フック(ILIBC-HDR-001 §4.3)。戻ってはならない。
extern "C" [[noreturn]] void __ilibc_assert_fail(const char *expr, const char *file,
                                                  int line) noexcept {
    th_puts("ASSERT expr=");
    th_puts(expr);
    th_puts(" file=");
    th_puts(file);
    th_puts(" line=");
    th_put_int(line);
    th_putc('\n');
    ilibc_test_exit(42);
}
