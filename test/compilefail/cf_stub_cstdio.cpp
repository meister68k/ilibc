// cf_stub_cstdio.cpp — 条件付き公開の <cstdio> は、バックエンド未指定かつ
// ILIBC_STUB_PASSTHROUGH なしで、仕様のメッセージ(ilibc:、ヘッダ名、
// is not provided、§5.1 の理由)付きでコンパイルエラーになる
// (TC-HD-11・TC-IO-15、ILIBC-HDR-001 §7.1・ILIBC-IO-001 §5.1)。
// 対照(-DCF_CONTROL)ではスタブを含めない(素通りの検査は TC-HD-12)。
// EXPECT-GCC:   ilibc: <cstdio> is not provided \(no I/O\)
// EXPECT-CLANG: ilibc: <cstdio> is not provided
#ifndef CF_CONTROL
#include <cstdio>
#endif
int main() { return 0; }
