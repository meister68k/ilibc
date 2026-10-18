// cf_cerrno_errno_conflict.cpp — <cerrno> より先に別のヘッダが errno マクロを
// 定義していた場合、無断で上書きせず #error でビルドを停止する
// (TC-HD-09(e)、ILIBC-HDR-001 §5.1、ILIBC-ERR-001 §5)。
// EXPECT-GCC:   ibc: errno is already defined
// EXPECT-CLANG: ibc: errno is already defined
#ifdef CF_FAIL
// 失敗用: 別ヘッダが先に errno を定義済みの状態を模す
#  define errno 1
#endif
#include <cerrno>
#ifdef CF_FAIL
// #error の後も処理は続くため、errno の再定義は警告になるが失敗原因は #error。
// ここでは名前を使わない
int main() { return 1; }
#else
// 対照(-DCF_CONTROL): 衝突がなければ公開名を通常どおり使える
int main() { errno = 0; return (EDOM == 33 && ERANGE == 34 && EINVAL == 22) ? 0 : 1; }
#endif
