// cf_stub_signal_h.cpp — スタブ <signal.h> は ILIBC_STUB_PASSTHROUGH なしで、
// 仕様のメッセージ(ilibc:、ヘッダ名、is not provided、§7.1 の理由)付きで
// コンパイルエラーになる(TC-HD-11、ILIBC-HDR-001 §7.1/§7.3 S1)。
// 対照(-DCF_CONTROL)ではスタブを含めない(素通りの検査は TC-HD-12)。
// EXPECT-GCC:   ilibc: <signal.h> is not provided \(no OS\)
// EXPECT-CLANG: ilibc: <signal.h> is not provided
#ifndef CF_CONTROL
#include <signal.h>
#endif
int main() { return 0; }
