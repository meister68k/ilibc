// str_cases.hpp — <cstring> の実行時試験本体(書き込み範囲・アライメント・
// memmove 重なり)。ホスト差分(H)と E1 実行(R)で共有する(TST §5.2 単一ソース)。
// spec/ilibc_cstring_spec.md §9 TC-ST-07〜09、spec/ilibc_test_spec.md §5.2/§5.7。
// 出力は含めない(T5)。各関数は最初の失敗の事例番号を返し、-1 は合格。
#pragma once

#include <cstring>
#include <ilibc/config.hpp>

namespace str_cases {

// 試験対象の名前空間(std、またはホスト差分の ilibc_under_test)(T1)
namespace t = ILIBC_NS;

// 実行時に値を隠し、最適化による畳み込みを防ぐ(T3)
inline int vol(int x) {
    volatile int v = x;
    return v;
}

// ------------------------------------------------- TC-ST-07: 書き込み範囲

// 前後をカナリア 0xA5、領域を下書き 0x3C で埋める
inline void fill_pat(unsigned char *buf, int pre, int region, int suf) {
    for (int i = 0; i < pre; ++i) {
        buf[i] = 0xA5;
    }
    for (int i = 0; i < region; ++i) {
        buf[pre + i] = 0x3C;
    }
    for (int i = 0; i < suf; ++i) {
        buf[pre + region + i] = 0xA5;
    }
}

// 書き込み範囲の検査: 前後のカナリアが保たれ、[pre, pre+want_n) が want、
// 書き込まれていないはずの [pre+extent, pre+region) が下書きのままか
inline bool extent_bad(const unsigned char *buf, int pre, int region, int suf, int extent,
                       const unsigned char *want, int want_n) {
    for (int i = 0; i < pre; ++i) {
        if (buf[i] != 0xA5) return true;
    }
    for (int i = 0; i < want_n; ++i) {
        if (buf[pre + i] != want[i]) return true;
    }
    for (int i = extent; i < region; ++i) {
        if (buf[pre + i] != 0x3C) return true;
    }
    for (int i = 0; i < suf; ++i) {
        if (buf[pre + region + i] != 0xA5) return true;
    }
    return false;
}

// 事例0: strcpy は "ab" + 終端の3バイトだけを書き込む
inline bool case0_strcpy() {
    unsigned char buf[32];
    fill_pat(buf, 8, 16, 8);
    char *dst = reinterpret_cast<char *>(buf + vol(8));
    char *r = t::strcpy(dst, "ab");
    const unsigned char want[3] = {'a', 'b', '\0'};
    return r == dst && !extent_bad(buf, 8, 16, 8, 3, want, 3);
}

// 事例1: strncpy は残りを '\0' で埋めてちょうど n バイトだけ書く(s2が短い)
inline bool case1_strncpy_pad() {
    unsigned char buf[32];
    fill_pat(buf, 8, 16, 8);
    char *dst = reinterpret_cast<char *>(buf + vol(8));
    char *r = t::strncpy(dst, "xy", static_cast<std::size_t>(vol(5)));
    const unsigned char want[5] = {'x', 'y', '\0', '\0', '\0'};
    return r == dst && !extent_bad(buf, 8, 16, 8, 5, want, 5);
}

// 事例2: strncpy は s2 が n 以上のとき終端せず、ちょうど n バイト書く
inline bool case2_strncpy_noterm() {
    unsigned char buf[32];
    fill_pat(buf, 8, 16, 8);
    char *dst = reinterpret_cast<char *>(buf + vol(8));
    char *r = t::strncpy(dst, "1234567", static_cast<std::size_t>(vol(5)));
    const unsigned char want[5] = {'1', '2', '3', '4', '5'};
    return r == dst && !extent_bad(buf, 8, 16, 8, 5, want, 5);
}

// 領域の先頭に "hi\0" を入れて連結系の事例の土台にする
inline void put_prefix(unsigned char *buf, int pre) {
    buf[pre + 0] = 'h';
    buf[pre + 1] = 'i';
    buf[pre + 2] = '\0';
}

// 事例3: strcat は strlen(s1) + strlen(s2) + 1 バイトだけ書く
inline bool case3_strcat() {
    unsigned char buf[32];
    fill_pat(buf, 8, 16, 8);
    put_prefix(buf, 8);
    char *dst = reinterpret_cast<char *>(buf + vol(8));
    char *r = t::strcat(dst, "yo");
    const unsigned char want[5] = {'h', 'i', 'y', 'o', '\0'};
    return r == dst && !extent_bad(buf, 8, 16, 8, 5, want, 5);
}

// 事例4: strncat は最大 n+1 バイト(文字 n + 終端)を末尾に書く
inline bool case4_strncat() {
    unsigned char buf[32];
    fill_pat(buf, 8, 16, 8);
    put_prefix(buf, 8);
    char *dst = reinterpret_cast<char *>(buf + vol(8));
    char *r = t::strncat(dst, "abcd", static_cast<std::size_t>(vol(2)));
    const unsigned char want[5] = {'h', 'i', 'a', 'b', '\0'};
    return r == dst && !extent_bad(buf, 8, 16, 8, 5, want, 5);
}

// 事例5: strncat の n=0 は終端の書き直し1バイトだけ
inline bool case5_strncat_zero() {
    unsigned char buf[32];
    fill_pat(buf, 8, 16, 8);
    put_prefix(buf, 8);
    char *dst = reinterpret_cast<char *>(buf + vol(8));
    char *r = t::strncat(dst, "zz", static_cast<std::size_t>(vol(0)));
    const unsigned char want[3] = {'h', 'i', '\0'};
    return r == dst && !extent_bad(buf, 8, 16, 8, 3, want, 3);
}

// 事例6: memcpy は n バイトだけ書く
inline bool case6_memcpy() {
    unsigned char buf[32];
    unsigned char src[8];
    fill_pat(buf, 8, 16, 8);
    for (int i = 0; i < 8; ++i) {
        src[i] = static_cast<unsigned char>(i + 1);
    }
    unsigned char *dst = buf + vol(8);
    unsigned char *r = static_cast<unsigned char *>(
        t::memcpy(dst, src, static_cast<std::size_t>(vol(6))));
    const unsigned char want[6] = {1, 2, 3, 4, 5, 6};
    return r == dst && !extent_bad(buf, 8, 16, 8, 6, want, 6);
}

// 事例7: memset は n バイトだけ書く
inline bool case7_memset() {
    unsigned char buf[32];
    fill_pat(buf, 8, 16, 8);
    unsigned char *dst = buf + vol(8);
    unsigned char *r = static_cast<unsigned char *>(
        t::memset(dst, vol(0x7E), static_cast<std::size_t>(vol(6))));
    const unsigned char want[6] = {0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E};
    return r == dst && !extent_bad(buf, 8, 16, 8, 6, want, 6);
}

// 事例8: memmove(重ならない場合)は n バイトだけ書く
inline bool case8_memmove() {
    unsigned char buf[32];
    unsigned char src[8];
    fill_pat(buf, 8, 16, 8);
    for (int i = 0; i < 8; ++i) {
        src[i] = static_cast<unsigned char>(i + 1);
    }
    unsigned char *dst = buf + vol(8);
    unsigned char *r = static_cast<unsigned char *>(
        t::memmove(dst, src, static_cast<std::size_t>(vol(6))));
    const unsigned char want[6] = {1, 2, 3, 4, 5, 6};
    return r == dst && !extent_bad(buf, 8, 16, 8, 6, want, 6);
}

// 書き込み範囲9事例を実行し、最初の失敗の事例番号(0〜8)を返す(-1なら合格)
inline int tc_st07_first_failure() {
    if (!case0_strcpy()) return 0;
    if (!case1_strncpy_pad()) return 1;
    if (!case2_strncpy_noterm()) return 2;
    if (!case3_strcat()) return 3;
    if (!case4_strncat()) return 4;
    if (!case5_strncat_zero()) return 5;
    if (!case6_memcpy()) return 6;
    if (!case7_memset()) return 7;
    if (!case8_memmove()) return 8;
    return -1;
}

// ------------------------------------------------- TC-ST-08: アラインメント

// 24バイト領域に src を設定し、dst を 0x5A で埋める
inline void align_setup(unsigned char *src, unsigned char *dst) {
    for (int i = 0; i < 24; ++i) {
        src[i] = static_cast<unsigned char>(i * 5 + 3);
        dst[i] = 0x5A;
    }
}

// memcpy を先頭オフセット so/d で実行し、手書きの期待配列と一致するか
inline bool case_memcpy_align(int so, int d) {
    unsigned char src[24];
    unsigned char dst[24];
    unsigned char want[24];
    align_setup(src, dst);
    for (int i = 0; i < 24; ++i) {
        want[i] = 0x5A;
    }
    for (int i = 0; i < 16; ++i) {
        want[d + i] = src[so + i];
    }
    t::memcpy(dst + vol(d), src + vol(so), static_cast<std::size_t>(vol(16)));
    for (int i = 0; i < 24; ++i) {
        if (dst[i] != want[i]) return false;
    }
    return true;
}

// memmove(重ならない2配列)を先頭オフセット so/d で実行し、期待配列と一致するか
inline bool case_memmove_align(int so, int d) {
    unsigned char src[24];
    unsigned char dst[24];
    unsigned char want[24];
    align_setup(src, dst);
    for (int i = 0; i < 24; ++i) {
        want[i] = 0x5A;
    }
    for (int i = 0; i < 16; ++i) {
        want[d + i] = src[so + i];
    }
    t::memmove(dst + vol(d), src + vol(so), static_cast<std::size_t>(vol(16)));
    for (int i = 0; i < 24; ++i) {
        if (dst[i] != want[i]) return false;
    }
    return true;
}

// memset を先頭オフセット o で実行し、期待配列と一致するか
inline bool case_memset_align(int o) {
    unsigned char dst[24];
    unsigned char want[24];
    for (int i = 0; i < 24; ++i) {
        dst[i] = 0x5A;
        want[i] = static_cast<unsigned char>((i >= o && i < o + 16) ? 0xC3 : 0x5A);
    }
    t::memset(dst + vol(o), vol(0xC3), static_cast<std::size_t>(vol(16)));
    for (int i = 0; i < 24; ++i) {
        if (dst[i] != want[i]) return false;
    }
    return true;
}

// memcmp を先頭オフセット so/d で実行し、手書きの期待値(先頭バイトの差 so-d)と
// 一致するか(値 a[i]=0x20+i はすべて異なり、先頭で必ず不一致になる)
inline bool case_memcmp_align(int so, int d) {
    unsigned char a[24];
    for (int i = 0; i < 24; ++i) {
        a[i] = static_cast<unsigned char>(0x20 + i);
    }
    const int r = t::memcmp(a + vol(so), a + vol(d), static_cast<std::size_t>(vol(16)));
    return r == so - d;
}

// memchr を先頭オフセット so で実行し、値(絶対位置 d+3 にだけ存在する)の
// 位置が手書きの期待どおりか(ウィンドウ外なら nullptr)
inline bool case_memchr_align(int so, int d) {
    unsigned char a[24];
    for (int i = 0; i < 24; ++i) {
        a[i] = static_cast<unsigned char>(0x20 + i);
    }
    const int target = 0x20 + d + 3;
    const unsigned char *r = static_cast<const unsigned char *>(
        t::memchr(a + vol(so), target, static_cast<std::size_t>(vol(16))));
    const int at = d + 3;
    if (at < so || at >= so + 16) {
        return r == nullptr;
    }
    return r == a + at;
}

// アラインメント試験の失敗番号の符号付きエンコード(TC-ST-08 の case= 表示用):
// 0〜63=memcpy、64〜127=memmove、128〜135=memset(オフセットのみ)、
// 136〜199=memcmp、200〜263=memchr(各領域内は so*8+d)
inline int tc_st08_first_failure() {
    for (int so = 0; so < 8; ++so) {
        for (int d = 0; d < 8; ++d) {
            if (!case_memcpy_align(so, d)) return so * 8 + d;
            if (!case_memmove_align(so, d)) return 64 + so * 8 + d;
            if (!case_memcmp_align(so, d)) return 136 + so * 8 + d;
            if (!case_memchr_align(so, d)) return 200 + so * 8 + d;
        }
        if (!case_memset_align(so)) return 128 + so;
    }
    return -1;
}

// ------------------------------------------------- TC-ST-09: memmove 重なり

// n=0〜32、コピー先−コピー元のオフセット差 −32〜+32 の全組み合わせを、
// 一時バッファ経由のコピーと比較する。失敗番号は n*65+(delta+32)(-1は合格)
inline int tc_st09_first_failure() {
    unsigned char buf[112];
    unsigned char tmp[32];
    unsigned char want[112];
    for (int n = 0; n <= 32; ++n) {
        for (int delta = -32; delta <= 32; ++delta) {
            const int s = vol(40);
            const int d = s + vol(delta);
            const int nn = vol(n);
            for (int i = 0; i < 112; ++i) {
                buf[i] = static_cast<unsigned char>(i * 13 + 7);
                want[i] = buf[i];
            }
            for (int i = 0; i < nn; ++i) {
                tmp[i] = buf[s + i];            // 元の内容を退避(参照の基準)
            }
            t::memmove(buf + d, buf + s, static_cast<std::size_t>(nn));
            for (int i = 0; i < nn; ++i) {
                want[d + i] = tmp[i];
            }
            for (int i = 0; i < 112; ++i) {
                if (buf[i] != want[i]) {
                    return n * 65 + (delta + 32);
                }
            }
        }
    }
    return -1;
}

}  // namespace str_cases
