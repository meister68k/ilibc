// cf_stdio_h_with_backend.cpp — <stdio.h> はバックエンド選択の有無に
// よらず常に #error の禁止スタブである(TC-IO-18、ILIBC-IO-001 §5.1/§10)。
// 対照では同じ設定で条件付き公開の <cstdio> を使う。
// EXPECT-GCC:   ilibc: <stdio.h> is not provided
// EXPECT-CLANG: ilibc: <stdio.h> is not provided
#define ILIBC_IO_BACKEND_POSIX
#ifndef CF_CONTROL
#include <stdio.h>
#else
#include <cstdio>
#endif
int main() { return 0; }
