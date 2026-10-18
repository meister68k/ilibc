// cf_stub_clocale.cpp — スタブ <clocale> は ILIBC_STUB_PASSTHROUGH なしで、
// 仕様のメッセージ(ilibc:、ヘッダ名、is not provided、§7.1 の理由)付きで
// コンパイルエラーになる(TC-HD-11、ILIBC-HDR-001 §7.1/§7.3 S1)。
// 対照(-DCF_CONTROL)ではスタブを含めない(素通りの検査は TC-HD-12)。
// EXPECT-GCC:   ilibc: <clocale> is not provided \(only the C locale\)
// EXPECT-CLANG: ilibc: <clocale> is not provided
#ifndef CF_CONTROL
#include <clocale>
#endif
int main() { return 0; }
