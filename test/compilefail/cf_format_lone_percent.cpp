// cf_format_lone_percent.cpp — 書式が単独の % で終わるとコンパイルエラーに
// なる(TC-IO-03、ILIBC-IO-001 §3)。
// EXPECT-GCC:   ilibc: invalid format token
// EXPECT-CLANG: ilibc: invalid format token
#include <ilibc/format.hpp>

#ifdef CF_FAIL
// 失敗用: % の後ろに変換指定がない
int main() {
    char buf[16];
    return ilibc::snprint_f<"abc%">(buf, sizeof buf);
}
#else
// 対照(-DCF_CONTROL): %% なら有効なトークン
int main() {
    char buf[16];
    return ilibc::snprint_f<"abc%%">(buf, sizeof buf);
}
#endif
