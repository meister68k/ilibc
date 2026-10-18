// cassert.cpp — <cassert>のコンパイル時試験(S)。
// spec/ilibc_cassert_spec.md §8 TC-HD-06(c)(d) と spec/ilibc_test_spec.md §5.3。
// 真の assert は定数式の中で使える(AST §3.4、HDR §4.1)。
// NDEBUG 定義時は式を評価しないため、どの static_assert も成立する(§3.1)。
#include <cassert>
#include <type_traits>

// ------------------------------------------------- TC-HD-06(d): 定数式で真

// 真の条件の assert を含む constexpr 関数が、定数評価で通ること
constexpr int hd06_checked(int n) {
    assert(n >= 0);  // 真: 定数式として成立する(HDR §4.1)
    return n;
}
static_assert(hd06_checked(0) == 0, "TC-HD-06(d) assert(true) in constant expression");
static_assert(hd06_checked(1) == 1, "TC-HD-06(d) assert(true) in constant expression");

// 偽の assert は非 constexpr のフック呼び出しにより定数式でなくなるため、
// コンパイルエラーになることの対照側(失敗用は cf_cassert_constexpr_false.cpp)

// ------------------------------------------------- TC-HD-06(c): コンマを含む式

// 可変長マクロ引数のため、コンマを含む式を受け取れる(HDR §4.1)
constexpr bool hd06_comma(int n) {
    assert(((void)(n + 1 == 2), true));  // カンマ演算子(左辺は副作用なし、void 化)
    assert((std::is_same_v<int, int>));  // テンプレート実引数のコンマ
    return n >= 1;  // NDEBUG 時にも n が使われ、未使用引数の警告を防ぐ
}
static_assert(hd06_comma(1), "TC-HD-06(c) comma in assert operand");
