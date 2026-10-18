// cstring_sample.cpp — <cstring>だけを使うプログラム。
// TC-ST-13 利用者側リンク検査(-ffreestanding -fno-builtin -nostdlib でコンパイルし、
// ilibc.a なしでは未解決が mem* の5個の部分集合、あれば0になる)の対象。
// spec/ilibc_cstring_spec.md §9 TC-ST-12/13。
// 引数由来にしないとコンパイル時に畳み込まれ、コード生成と未解決シンボルの
// 検査ができなくなる。
#include <cstring>

namespace t = ILIBC_NS;

int main(int argc, char **) {
    const char *s1 = (argc > 1) ? "abcdef" : "abcxyz";
    const char *s2 = (argc > 2) ? "cd" : "cX";
    char w1[64];
    char w2[64];
    for (int i = 0; i < 64; ++i) {
        w1[i] = static_cast<char>(0x5A);
        w2[i] = static_cast<char>(0xA5);
    }

    // コピー・連結(ST-01/02/05〜09)。memmove は重なり経路を含む
    t::memcpy(w1, s1, 7);
    t::memmove(w1 + 1, w1, 6);
    t::memset(w2, 0x3C, 32);
    t::strcpy(w2, s1);
    t::strncpy(w2, s1, 4);
    t::strcat(w2, s2);
    t::strncat(w2, s2, 2);

    // 比較(ST-03/10/11)
    const int cmp = t::memcmp(w1, w2, 8) + t::strcmp(s1, s2) + t::strncmp(s1, s2, 3);

    // 検索(ST-04/12〜17)。見つからない場合は nullptr / 0 でも成立する
    const void *mc = t::memchr(w1, 'c', 16);
    const char *p1 = t::strchr(s1, 'c');
    const char *p2 = t::strrchr(s1, 'c');
    const char *p3 = t::strstr(s1, s2);
    const std::size_t sp = t::strspn(s1, "abc");
    const std::size_t csp = t::strcspn(s1, "xyz");
    const char *p4 = t::strpbrk(s1, s2);
    const std::size_t ln = t::strlen(s1);

    // 結果を(argc非依存に畳めない形で)合算して終了コードにする
    const int hits = (mc != nullptr) + (p1 != nullptr) + (p2 != nullptr) + (p3 != nullptr) +
                     (p4 != nullptr) + (ln > 0) + (cmp != 0) + (sp > 0) + (csp > 0) +
                     (w1[0] == w2[0]);
    return (hits > 3) ? 0 : 1;
}
