// cctype.cpp — <cctype>のコンパイル時試験(S)。
// spec/ilibc_cctype_spec.md §8 TC-CT-01〜06、spec/ilibc_test_spec.md §5.3。
// 関数ごとに別の static_assert にして、失敗箇所を絞る(§5.3)。
// 成功の判定はすべて -1(または true)。診断は式とメッセージでどのTCか分かるようにする。
#include "../common/ctype_cases.hpp"
#include <type_traits>

// ---- TC-CT-01: 13関数を -1〜255 の全257値で独立参照と比較
// (c=0〜255は本体、c=-1 は neg1。C++20/C++23 の両方でコンパイルすること)

static_assert(tc_ct01_neg1_ok(Direct{}, 0) && tc_ct01_first_failure(Direct{}, 0) == -1,
              "TC-CT-01 isalnum");
static_assert(tc_ct01_neg1_ok(Direct{}, 1) && tc_ct01_first_failure(Direct{}, 1) == -1,
              "TC-CT-01 isalpha");
static_assert(tc_ct01_neg1_ok(Direct{}, 2) && tc_ct01_first_failure(Direct{}, 2) == -1,
              "TC-CT-01 iscntrl");
static_assert(tc_ct01_neg1_ok(Direct{}, 3) && tc_ct01_first_failure(Direct{}, 3) == -1,
              "TC-CT-01 isdigit");
static_assert(tc_ct01_neg1_ok(Direct{}, 4) && tc_ct01_first_failure(Direct{}, 4) == -1,
              "TC-CT-01 isgraph");
static_assert(tc_ct01_neg1_ok(Direct{}, 5) && tc_ct01_first_failure(Direct{}, 5) == -1,
              "TC-CT-01 islower");
static_assert(tc_ct01_neg1_ok(Direct{}, 6) && tc_ct01_first_failure(Direct{}, 6) == -1,
              "TC-CT-01 isprint");
static_assert(tc_ct01_neg1_ok(Direct{}, 7) && tc_ct01_first_failure(Direct{}, 7) == -1,
              "TC-CT-01 ispunct");
static_assert(tc_ct01_neg1_ok(Direct{}, 8) && tc_ct01_first_failure(Direct{}, 8) == -1,
              "TC-CT-01 isspace");
static_assert(tc_ct01_neg1_ok(Direct{}, 9) && tc_ct01_first_failure(Direct{}, 9) == -1,
              "TC-CT-01 isupper");
static_assert(tc_ct01_neg1_ok(Direct{}, 10) && tc_ct01_first_failure(Direct{}, 10) == -1,
              "TC-CT-01 isxdigit");
static_assert(tc_ct01_neg1_ok(Direct{}, 11) && tc_ct01_first_failure(Direct{}, 11) == -1,
              "TC-CT-01 tolower");
static_assert(tc_ct01_neg1_ok(Direct{}, 12) && tc_ct01_first_failure(Direct{}, 12) == -1,
              "TC-CT-01 toupper");

// ---- TC-CT-02: 領域外・極端値、ビット別名化(下位8ビットだけを見る誤りを検出)
static_assert(tc_ct02_first_failure(Direct{}) == -1, "TC-CT-02");

// ---- TC-CT-03: 0〜127で真になる個数が仕様の個数表と一致
static_assert(tc_ct03_first_failure(Direct{}) == -1, "TC-CT-03");

// ---- TC-CT-04: 関係式R1〜R9(0〜255、極端値、別名化値、および c=-1)
static_assert(tc_ct04_neg1_ok(Direct{}) && tc_ct04_first_failure(Direct{}) == -1,
              "TC-CT-04");

// ---- TC-CT-05: 変換の性質V1〜V5、領域外入力の恒等性(および c=-1)
static_assert(tc_ct05_neg1_ok(Direct{}) && tc_ct05_first_failure(Direct{}) == -1,
              "TC-CT-05");

// ---- TC-CT-06: 型(bool/int)、noexcept、定数式で呼べること(関数ごと)

static_assert(std::is_same_v<decltype(t::isalnum(0)), bool> && noexcept(t::isalnum(0))
                  && t::isalnum('A') && !t::isalnum('@'),
              "TC-CT-06 isalnum");
static_assert(std::is_same_v<decltype(t::isalpha(0)), bool> && noexcept(t::isalpha(0))
                  && t::isalpha('Z') && !t::isalpha('9'),
              "TC-CT-06 isalpha");
static_assert(std::is_same_v<decltype(t::iscntrl(0)), bool> && noexcept(t::iscntrl(0))
                  && t::iscntrl('\n') && !t::iscntrl(' '),
              "TC-CT-06 iscntrl");
static_assert(std::is_same_v<decltype(t::isdigit(0)), bool> && noexcept(t::isdigit(0))
                  && t::isdigit('0') && !t::isdigit('a'),
              "TC-CT-06 isdigit");
static_assert(std::is_same_v<decltype(t::isgraph(0)), bool> && noexcept(t::isgraph(0))
                  && t::isgraph('!') && !t::isgraph(' '),
              "TC-CT-06 isgraph");
static_assert(std::is_same_v<decltype(t::islower(0)), bool> && noexcept(t::islower(0))
                  && t::islower('z') && !t::islower('Z'),
              "TC-CT-06 islower");
static_assert(std::is_same_v<decltype(t::isprint(0)), bool> && noexcept(t::isprint(0))
                  && t::isprint(' ') && !t::isprint('\t'),
              "TC-CT-06 isprint");
static_assert(std::is_same_v<decltype(t::ispunct(0)), bool> && noexcept(t::ispunct(0))
                  && t::ispunct('+') && !t::ispunct('a'),
              "TC-CT-06 ispunct");
static_assert(std::is_same_v<decltype(t::isspace(0)), bool> && noexcept(t::isspace(0))
                  && t::isspace('\r') && !t::isspace('x'),
              "TC-CT-06 isspace");
static_assert(std::is_same_v<decltype(t::isupper(0)), bool> && noexcept(t::isupper(0))
                  && t::isupper('A') && !t::isupper('a'),
              "TC-CT-06 isupper");
static_assert(std::is_same_v<decltype(t::isxdigit(0)), bool> && noexcept(t::isxdigit(0))
                  && t::isxdigit('f') && !t::isxdigit('g'),
              "TC-CT-06 isxdigit");
static_assert(std::is_same_v<decltype(t::tolower(0)), int> && noexcept(t::tolower(0))
                  && t::tolower('A') == 'a' && t::tolower('!') == '!',
              "TC-CT-06 tolower");
static_assert(std::is_same_v<decltype(t::toupper(0)), int> && noexcept(t::toupper(0))
                  && t::toupper('a') == 'A' && t::toupper('!') == '!',
              "TC-CT-06 toupper");
