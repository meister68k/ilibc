// cstring_sizes_probe.cpp — TC-ST-14 用: 文字列系関数を非インライン化したときの
// コードサイズを記録するためのプローブ。関数アドレスを外部リンクの配列に取り、
// 最適化で消えない out-of-line コピーを強制する。
// 配列は定数初期化(静的初期化セクションは増えない)。このファイルは -c のみで
// 使い、ilibc.a には入れない(本体のアートファクト検査 TC-ST-12 と分離する)。
#include <cstring>

namespace t = ILIBC_NS;

// const 戻り/引数と非 const 版、memset の int 引数は型が違うので分ける
using mem2_fn = void *(*)(void *, const void *, std::size_t);
using memset_fn = void *(*)(void *, int, std::size_t);
using c_memchr_fn = const void *(*)(const void *, int, std::size_t);
using memchr_fn = void *(*)(void *, int, std::size_t);
using memcmp_fn = int (*)(const void *, const void *, std::size_t);
using sz1_fn = std::size_t (*)(const char *);
using sz2_fn = std::size_t (*)(const char *, const char *);
using cmp2_fn = int (*)(const char *, const char *);
using cmp3_fn = int (*)(const char *, const char *, std::size_t);
using cch1_fn = const char *(*)(const char *, int);
using ch1_fn = char *(*)(char *, int);
using cch2_fn = const char *(*)(const char *, const char *);
using ch2_fn = char *(*)(char *, const char *);
using ch3_fn = char *(*)(char *, const char *);
using ch4_fn = char *(*)(char *, const char *, std::size_t);

// volatile で保持し、最適化で参照が消えないようにする
mem2_fn volatile keep_mem2[] = {&t::memcpy, &t::memmove};
memset_fn volatile keep_memset[] = {&t::memset};
c_memchr_fn volatile keep_c_memchr[] = {&t::memchr};
memchr_fn volatile keep_memchr[] = {&t::memchr};
memcmp_fn volatile keep_memcmp[] = {&t::memcmp};
sz1_fn volatile keep_sz1[] = {&t::strlen};
sz2_fn volatile keep_sz2[] = {&t::strspn, &t::strcspn};
cmp2_fn volatile keep_cmp2[] = {&t::strcmp};
cmp3_fn volatile keep_cmp3[] = {&t::strncmp};
cch1_fn volatile keep_cch1[] = {&t::strchr, &t::strrchr};
ch1_fn volatile keep_ch1[] = {&t::strchr, &t::strrchr};
cch2_fn volatile keep_cch2[] = {&t::strstr, &t::strpbrk};
ch2_fn volatile keep_ch2[] = {&t::strstr, &t::strpbrk};
ch3_fn volatile keep_ch3[] = {&t::strcpy, &t::strcat};
ch4_fn volatile keep_ch4[] = {&t::strncpy, &t::strncat};
