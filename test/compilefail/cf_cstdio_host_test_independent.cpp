// cf_cstdio_host_test_independent.cpp — ILIBC_HOST_TEST はホストlibcとの
// シンボル隔離用の独立した設定で、バックエンド選択には使えない
// (TC-IO-16、ILIBC-IO-001 §5.1)。単独指定では <cstdio> は未選択のままで
// コンパイルエラーになる。
// EXPECT-GCC:   ilibc: <cstdio> is not provided \(no I/O\)
// EXPECT-CLANG: ilibc: <cstdio> is not provided
#define ILIBC_HOST_TEST
#ifdef CF_FAIL
// 失敗用: HOST_TEST だけではバックエンド未選択のまま
#else
// 対照(-DCF_CONTROL): POSIX を併せ指定すれば成立する
#  define ILIBC_IO_BACKEND_POSIX
#endif
#include <cstdio>
int main() { return ilibc::putstr("") ? 1 : 0; }
