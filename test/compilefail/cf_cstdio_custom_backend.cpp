// cf_cstdio_custom_backend.cpp — ILIBC_IO_BACKEND_CUSTOM は将来の独自OS用の
// 予約選択で、初版では未実装のため <cstdio> のインクルードがコンパイル
// エラーになる(TC-IO-16、ILIBC-IO-001 §5.1)。
// EXPECT-GCC:   ILIBC_IO_BACKEND_CUSTOM is not implemented
// EXPECT-CLANG: ILIBC_IO_BACKEND_CUSTOM is not implemented
#ifdef CF_FAIL
#  define ILIBC_IO_BACKEND_CUSTOM
#else
// 対照(-DCF_CONTROL): POSIX なら成立する
#  define ILIBC_IO_BACKEND_POSIX
#endif
#include <cstdio>
int main() { return ilibc::putstr("") ? 1 : 0; }
