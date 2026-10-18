// cf_printf_macro_off.cpp — ILIBC_ENABLE_PRINTF_MACRO を指定しないと
// <cstdio> は printf マクロを一切生成しないため、printf の使用はコンパイル
// エラーになる(TC-IO-12、ILIBC-IO-001 §9)。指定時は規定形のマクロが使えて、
// ゼロ個の値引数も成立する。
// EXPECT-GCC:   printf.*was not declared
// EXPECT-CLANG: printf.*was not declared
#define ILIBC_IO_BACKEND_POSIX
#ifdef CF_FAIL
// 失敗用: マクロなしでは printf は宣言されない
#include <cstdio>
int main() { return printf("x=%d", 1); }
#else
// 対照(-DCF_CONTROL): マクロON(ゼロ値引数も可)
#  define ILIBC_ENABLE_PRINTF_MACRO
#  include <cstdio>
int main() { return printf("") ? 1 : 0; }
#endif
