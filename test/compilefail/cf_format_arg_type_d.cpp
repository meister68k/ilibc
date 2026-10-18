// cf_format_arg_type_d.cpp — %d に int 以外(long 等)を渡すとコンパイルエラーに
// なる(TC-IO-03、ILIBC-IO-001 §3)。幅の揃った固定長型の許容と誤りを仕様どおり
// に区別する。
// EXPECT-GCC:   ilibc: %d needs exactly int
// EXPECT-CLANG: ilibc: %d needs exactly int
#include <ilibc/format.hpp>

#ifdef CF_FAIL
// 失敗用: long は int ではない
int main() {
    char buf[16];
    return ilibc::snprint_f<"a%d">(buf, sizeof buf, 1L);
}
#else
// 対照(-DCF_CONTROL): int なら成立する
int main() {
    char buf[16];
    return ilibc::snprint_f<"a%d">(buf, sizeof buf, 1);
}
#endif
