# `<cstdlib>` 関数仕様書

| 項目 | 内容 |
|---|---|
| 文書ID | ILIBC-STL-001 |
| ライブラリ名 | ilibc（旧仮称: libc89） |
| 版 | 0.3（ドラフト） |
| 親文書 | ILIBC-OVR-001 全体概要仕様書 v0.5 |
| 対象関数 | SL-01〜SL-06（6個。`std::`のシグネチャは7個）、`ilibc::`のerr引数版4個、非標準の拡張関数`ilibc::_itoa_s`（2シグネチャ） |
| 対象ヘッダ | `<cstdlib>`（正）、`<stdlib.h>`（互換ラッパ） |
| 動作の基準 | ISO/IEC 9899:1990 §7.10。インタフェースはISO/IEC 14882:2024（C++23）からの意図的な逸脱を含む（第7章） |

---

## 1. 範囲と参照文書

- 本書は、`<cstdlib>`の6関数と、`ilibc::`のerr引数版4関数の、宣言、動作、基準実装、試験観点を定める。
- `<cstdlib>`が通常持つ、メモリ管理（`malloc`系）、終了処理（`abort`／`exit`／`atexit`）、環境（`getenv`／`system`）、浮動小数点変換（`atof`／`strtod`）、乱数（`rand`／`srand`）、ソート・探索（`qsort`／`bsearch`）、商と剰余（`div`／`ldiv`）、マルチバイト変換は、提供しない（概要仕様書 第7章）。
- 非標準の拡張関数として、整数を文字列に変換する`ilibc::_itoa_s`を提供する（第4.5節）。標準関数ではなく、`std::`にも、グローバルにも導入しない。

| 参照文書 | 参照箇所 |
|---|---|
| ILIBC-OVR-001 | 第5.3節（関数一覧）、第5.5節（`ilibc::`の関数）、第8.2節（`constexpr`方針）、第8.5節A（err引数版）、第9.2節（`errno`）、D-11、D-17 |
| ILIBC-CTP-001 | `isspace`の定義（`strtol`が使う） |
| ILIBC-TST-001 | 試験の共通方式 |

---

## 2. 提供物

### 2.1 標準名（`std::`）

`<cstdlib>`は、`namespace std`に次の関数を宣言する。全関数に`noexcept`と`constexpr`を付ける。**ND**は`[[nodiscard]]`を付けることを示す。

| ID | 関数 | シグネチャ | ND |
|---|---|---|---|
| SL-01 | `abs` † | `constexpr int abs(int j) noexcept;` | ○ |
| SL-02 | `labs` | `constexpr long labs(long j) noexcept;` | ○ |
| SL-03 | `atoi` | `constexpr int atoi(const char *nptr) noexcept;` | ○ |
| SL-04 | `atol` | `constexpr long atol(const char *nptr) noexcept;` | ○ |
| SL-05 | `strtol` | `constexpr long strtol(const char *nptr, char **endptr, int base) noexcept;` | － |
| SL-06 | `strtoul` | `constexpr unsigned long strtoul(const char *nptr, char **endptr, int base) noexcept;` | － |

**†** C++規格により、`abs`には`long`版のオーバーロード`constexpr long abs(long j) noexcept;`がある（`labs`と同じ処理）。`long long`版、浮動小数点版は提供しない。`std::abs(1LL)`、`std::abs(1U)`、`std::abs(1.0)`は、`int`版と`long`版の間で曖昧になり、**コンパイルエラー**になる。

- `strtol`と`strtoul`に`[[nodiscard]]`を付けないのは、`endptr`と`errno`への出力が、副作用として使われるため。
- すべてヘッダ内の`constexpr inline`関数。**`.cpp`ファイルもリンクシンボルも持たない**。

### 2.2 互換ラッパ `<stdlib.h>`

```cpp
#include <cstdlib>
using std::abs;   using std::labs;
using std::atoi;  using std::atol;
using std::strtol; using std::strtoul;
using std::size_t;
```

独自の宣言・定義を持たない。

### 2.3 拡張名（`ilibc::`）— エラー出力先を引数で指定する版

| 関数 | シグネチャ |
|---|---|
| `ilibc::strtol` | `constexpr long strtol(const char *nptr, char **endptr, int base, int *err) noexcept;` |
| `ilibc::strtoul` | `constexpr unsigned long strtoul(const char *nptr, char **endptr, int base, int *err) noexcept;` |
| `ilibc::atoi` | `constexpr int atoi(const char *nptr, int *err) noexcept;` |
| `ilibc::atol` | `constexpr long atol(const char *nptr, int *err) noexcept;` |

規則（概要仕様書 第8.5節A）:

| # | 規則 |
|---|---|
| 1 | 範囲外のとき、`err`が非`nullptr`なら、`*err`に`ERANGE`を設定する。**成功時は`*err`に書かない** |
| 2 | `err`が`nullptr`のとき、報告を捨てる |
| 3 | `errno`を変更しない |
| 4 | 報告するエラーは`ERANGE`のみ |
| 5 | `std::`の4関数は、`&errno`を渡してこれらを呼ぶラッパ。変換の本体は、`ilibc::`版にある |

### 2.4 型・マクロ

| 名前 | 内容 |
|---|---|
| `std::size_t`、`NULL` | `<cstddef>`（コンパイラ付属）から得る |
| `EXIT_SUCCESS` | 0（宣言互換のためのみ定義。`exit`は非提供。概要仕様書 D-17） |
| `EXIT_FAILURE` | 1（同上） |
| `MB_CUR_MAX` | `(static_cast<std::size_t>(1))`（`"C"`ロケール。D-17） |
| `ERANGE` | `<cerrno>`から得る |
| `EINVAL` | `<cerrno>`から得る（`_itoa_s`の引数エラー） |
| `LONG_MAX`、`LONG_MIN`、`ULONG_MAX` | `<climits>`（コンパイラ付属）から得る |

`div_t`、`ldiv_t`、`RAND_MAX`は定義しない。

### 2.5 拡張関数（非標準）`ilibc::_itoa_s`

整数を文字列に変換する、非標準の拡張関数（概要仕様書 第8.5節C）。MSVCの`_itoa_s`に準拠し、**エラーコードを戻り値で返す**（成功は0）。**`std::`にも、グローバルにも導入しない**（互換ラッパ`<stdlib.h>`の対象外）。

| # | シグネチャ | 備考 |
|---|---|---|
| 1 | `[[nodiscard]] constexpr int _itoa_s(int value, char *buffer, std::size_t size, int radix) noexcept;` | 基本形 |
| 2 | `template <std::size_t N> [[nodiscard]] constexpr int _itoa_s(int value, char (&buffer)[N], int radix) noexcept;` | 配列版。バッファ長を、配列の型`N`から得る（`size`を省略） |

**呼び出し形と選ばれる形**

| 呼び出し | 選ばれる形 |
|---|---|
| `_itoa_s(v, buf, n, r)`（`buf`は`char *`または配列） | #1 |
| `_itoa_s(v, arr, r)`（`arr`は`char`の配列） | #2 |

- 戻り値の型は`int`（`errno`と同じ型）。`errno_t`は定義しない。
- `char *`のポインタに、`size`を省略した3引数の形（`_itoa_s(v, ptr, r)`）で呼ぶと、バッファ長が分からないため、**コンパイルエラー**になる。
- `buffer`の要素型は`char`のみ。`unsigned char`、`signed char`の配列は、コンパイルエラーになる。
- **`errno`を使わない**。`err`引数も持たない。グローバル状態に触れないため、完全に再入可能で、失敗の結果までコンパイル時に評価できる。


---

## 3. 共通規則

### 3.1 前提

- ILP32（`int`と`long`は同じ32ビット。概要仕様書 第2.1節の`static_assert`）。`atoi`は、`strtol`の結果をそのまま`int`として返す。
- `"C"`ロケール。空白の判定は、`std::isspace`（0x20と0x09〜0x0D）。

### 3.2 `errno`とエラー報告

| 関数 | `errno` | `*err` |
|---|---|---|
| `std::strtol`、`std::strtoul`、`std::atoi`、`std::atol` | 範囲外のとき`ERANGE`を設定。成功時は変更しない | （引数なし） |
| `ilibc::`の同4関数 | **変更しない** | 範囲外のとき`ERANGE`（`err`が非`nullptr`のとき）。成功時は変更しない |
| `std::abs`、`std::labs` | 変更しない | （引数なし） |

- `errno`は、利用者が呼び出し前に0にする。ライブラリは0に戻さない。
- `atoi`／`atol`は、`strtol(nptr, nullptr, 10)`と、**`errno`も含めて等価**。C89は、`atoi`／`atol`が`errno`を設定してよいかを制限していないため、規格の範囲内（範囲外の結果の定義を追加する）。
- `ilibc::_itoa_s`は、`errno`を変更しない。エラーは、戻り値で返す（第4.5節）。

### 3.3 エラーとして検出されないもの

次の入力は、`ERANGE`を設定しない（C89が定義するのは`ERANGE`と`EDOM`のみで、本ライブラリは`ERANGE`だけを報告する）。

| 入力 | 結果 | 検出するには |
|---|---|---|
| 数字が1つもない（`""`、`"abc"`、`"-"`） | 0を返し、`errno`／`*err`はそのまま | `strtol`の`endptr`が`nptr`と等しいことを調べる |
| 数字の後ろに余分な文字がある（`"12abc"`） | 12を返す | `endptr`が文字列の終端を指すことを調べる |
| 無効な基数 | 0を返す（第4.2節） | 基数を検査する（D-L1） |

`atoi`／`atol`は、`endptr`を持たないため、これらを検出できない。必要なら`strtol`を使う。

### 3.4 `constexpr`

- 全関数が`constexpr`。
- **`errno`を書く経路**（範囲外）に入ると、その評価は定数式でなくなる（コンパイルエラー）。範囲内の入力は、コンパイル時に評価できる。

```cpp
static_assert(std::strtol("2147483647", nullptr, 10) == 2147483647L);   // OK
// static_assert(std::strtol("2147483648", nullptr, 10) == 0);          // コンパイルエラー（範囲外）
```

- 範囲外の結果までコンパイル時に調べたいときは、`ilibc::`のerr版を使う。

```cpp
constexpr bool test() {
    int e = 0;
    return ilibc::strtol("2147483648", nullptr, 10, &e) == LONG_MAX && e == ERANGE;
}
static_assert(test());                                                   // OK
```

- 定数式の評価中の未定義動作（未終端の文字列、符号付きオーバーフロー、`abs(INT_MIN)`）は、コンパイルエラーとして診断される。

### 3.5 再入・資源

| 項目 | 内容 |
|---|---|
| 内部状態 | なし（`errno`以外のグローバル変数、静的変数を持たない） |
| 再入可能性 | `ilibc::`のerr版（`strtol`系）、`ilibc::_itoa_s`、`abs`、`labs`は、**完全に再入可能**。`std::`の4関数は、範囲外のときだけ`errno`を書く。割り込みハンドラから呼ぶ場合は、err版を使う |
| スタック | 数ワード以下（再帰なし） |
| ヒープ | 使わない |
| ROM定数データ／RAM | 0バイト／0バイト |

---

## 4. 関数仕様

### 4.1 `abs`、`labs`（SL-01、SL-02）

| 項目 | 内容 |
|---|---|
| 動作 | 絶対値を返す |
| 戻り値 | `j >= 0`なら`j`、そうでなければ`-j` |
| 未定義動作 | `abs(INT_MIN)`、`labs(LONG_MIN)`（結果が表現できない。規格どおり）。定数式の評価では診断される |
| 試験の重点 | 0、±1、`INT_MAX`、`-INT_MAX`、`INT_MIN`（コンパイル失敗試験） |

### 4.2 `strtol`、`strtoul`（SL-05、SL-06）

**変換の定義**（C89 §7.10.1.5、§7.10.1.6）

次の順に、文字列を解釈する。

| # | 規則 |
|---|---|
| 1 | **空白の読み飛ばし**: 先頭の`std::isspace`が真の文字（0x20、0x09〜0x0D）を読み飛ばす |
| 2 | **符号**: 任意の`+`または`-`。符号は1つまで |
| 3 | **基数**: `base`が0のとき、`0x`／`0X`で始まれば16進数、`0`で始まれば8進数、それ以外は10進数。`base`が16のとき、`0x`／`0X`の接頭辞を読み飛ばす。ただし、接頭辞の直後が16進数字でない場合（`"0x"`、`"0xg"`）、接頭辞は読み飛ばさず、`"0"`だけを変換する（`endptr`は`x`を指す）。`base`が2〜36のとき、その基数。**それ以外（負、1、37以上）は無効な基数**（下記） |
| 4 | **数字**: `0`〜`9`は0〜9、`a`〜`z`（および`A`〜`Z`）は10〜35。`base`未満の値の数字が、続く限り変換する |
| 5 | **終了位置**: 変換した最後の数字の次の位置を、`*endptr`に格納する（`endptr`が非`nullptr`のとき）。数字が1つも変換されなかったとき、`*endptr`は`nptr`（入力の先頭）。この場合、返す値は0 |
| 6 | **範囲外**: 結果が表現できないとき、`strtol`は`LONG_MAX`（正）または`LONG_MIN`（負）、`strtoul`は`ULONG_MAX`を返し、`ERANGE`を報告する。この場合も、有効な数字をすべて読み進めて、`endptr`を設定する |
| 7 | **`strtoul`の負号**: `-`が付くと、変換した値を符号なしで否定（2^32を法とする）する。たとえば`"-1"`は`ULONG_MAX`で、**範囲外ではない**（エラー報告なし）。絶対値が`ULONG_MAX`を超えたときのみ範囲外 |

**無効な基数**（負、1、37以上）の扱い（規格は未規定）: 変換を行わず、0を返し、`*endptr`に`nptr`を格納する。**エラーは報告しない**（D-L1）。

**変換の例**（`strtol`。`LONG_MAX`=2147483647、`LONG_MIN`=−2147483648）

| 入力 | `base` | 戻り値 | `endptr`の位置 | エラー |
|---|---|---|---|---|
| `"123"` | 10 | 123 | 終端 | なし |
| `"  -42xyz"` | 10 | −42 | `x`の位置 | なし |
| `"+5"` | 10 | 5 | 終端 | なし |
| `"0x1F"` | 0 | 31 | 終端 | なし |
| `"0x1F"` | 16 | 31 | 終端 | なし |
| `"0x"` | 16 | 0 | `x`の位置 | なし |
| `"0xg"` | 0 | 0 | `x`の位置 | なし |
| `"017"` | 0 | 15 | 終端 | なし |
| `"019"` | 0 | 1 | `9`の位置 | なし |
| `"z"` | 36 | 35 | 終端 | なし |
| `""` | 10 | 0 | 先頭（`nptr`） | なし |
| `"abc"` | 10 | 0 | 先頭（`nptr`） | なし |
| `"-"` | 10 | 0 | 先頭（`nptr`） | なし |
| `"- 1"` | 10 | 0 | 先頭（`nptr`） | なし |
| `"2147483647"` | 10 | `LONG_MAX` | 終端 | なし |
| `"2147483648"` | 10 | `LONG_MAX` | 終端 | `ERANGE` |
| `"-2147483648"` | 10 | `LONG_MIN` | 終端 | なし |
| `"-2147483649"` | 10 | `LONG_MIN` | 終端 | `ERANGE` |
| `"99999999999999"` | 10 | `LONG_MAX` | 終端 | `ERANGE` |
| 任意 | 1 | 0 | 先頭（`nptr`） | なし（無効な基数） |

**`strtoul`の例**

| 入力 | `base` | 戻り値 | エラー |
|---|---|---|---|
| `"4294967295"` | 10 | `ULONG_MAX` | なし |
| `"4294967296"` | 10 | `ULONG_MAX` | `ERANGE` |
| `"-1"` | 10 | `ULONG_MAX` | なし |
| `"-4294967295"` | 10 | 1 | なし |
| `"-4294967296"` | 10 | `ULONG_MAX` | `ERANGE` |

### 4.3 `atoi`、`atol`（SL-03、SL-04）

| 項目 | 内容 |
|---|---|
| 動作 | `atoi(s)`は`(int)strtol(s, nullptr, 10)`、`atol(s)`は`strtol(s, nullptr, 10)`と**等価**（`errno`の設定を含む） |
| 戻り値 | 変換した値。範囲外のとき、`INT_MAX`／`INT_MIN`（`atol`は`LONG_MAX`／`LONG_MIN`）。数字がないとき0 |
| `errno` | 範囲外のとき`ERANGE`。成功時は変更しない |
| 備考 | ILP32では、`int`と`long`の範囲が同じなので、`strtol`の結果をそのまま返す |

### 4.4 `ilibc::`のerr版

第2.3節の規則に従う。`std::`版と、戻り値・`endptr`の動作は同一で、エラーの出力先だけが違う。

### 4.5 `ilibc::_itoa_s`（非標準の拡張関数）

| 項目 | 内容 |
|---|---|
| 動作 | `value`を`radix`進数の文字列に変換し、`buffer`に、終端`'\0'`付きで格納する |
| 引数 | `value`: 変換する整数。`buffer`: 出力先。`size`: `buffer`の大きさ（文字数。**終端を含む**）。配列版は`N`。`radix`: 基数（2〜36） |
| 戻り値 | `int`のエラーコード。**成功は0**。失敗は`EINVAL`または`ERANGE`（0以外の正の値） |
| `errno` | **変更しない**。グローバル状態を使わない |
| 引数の検査 | 検査は**仕様の一部**（未定義動作ではない）。他の標準関数が「検査しない」のとは異なる |

**検査と結果**（上から順に評価する）

| 順 | 条件 | 戻り値 | `buffer`の内容 |
|---|---|---|---|
| 1 | `buffer`が`nullptr`、または`size`が0 | `EINVAL` | **変更しない** |
| 2 | `radix`が2未満、または36を超える | `EINVAL` | `buffer[0]`を`'\0'`にする |
| 3 | `size`が、必要な大きさ（終端を含む）より小さい | `ERANGE` | `buffer[0]`を`'\0'`にする。他は変更しない |
| — | 上記以外 | 0 | 変換結果 |

**変換規則**

- **基数が10で、`value`が負のとき**: 先頭に`-`を付け、続けて絶対値を書く（`INT_MIN`も扱える）。
- **それ以外**: `value`を、32ビットの符号なし整数（2の補数のビット列）として変換する。`-`は付けない（例: `-1`を基数16で変換すると`"ffffffff"`）。
- 桁は、`0`〜`9`、`a`〜`z`（**小文字**）。先頭に不要な`0`を付けない（値0は`"0"`）。
- **必要な大きさ**: 文字数（符号を含む）＋1（終端）。最大は、基数2で32桁＋終端の**33**。基数10の`-2147483648`は、11文字＋1の12。**33バイトあれば、どの値・基数でも足りる**。
- 整数の除算・剰余を使う。ハードウェア除算がないコアでは、コンパイラ実行時ライブラリに依存する（概要仕様書 第10.5節）。

**使い方**

```cpp
char buf[12];                                  // "-2147483648" と終端
if (ilibc::_itoa_s(n, buf, 10) == 0) {         // 配列版: バッファ長は型から得られる
    uart_puts(buf);
}
```

**変換の例**

| `value` | `radix` | `size` | 戻り値 | `buffer`の結果 |
|---|---|---|---|---|
| 0 | 10 | 2 | 0 | `"0"` |
| 123 | 10 | 4 | 0 | `"123"` |
| -123 | 10 | 5 | 0 | `"-123"` |
| -123 | 10 | 4 | `ERANGE` | `""` |
| -1 | 10 | 3 | 0 | `"-1"` |
| 255 | 16 | 3 | 0 | `"ff"` |
| 255 | 16 | 2 | `ERANGE` | `""` |
| 255 | 2 | 9 | 0 | `"11111111"` |
| 35 | 36 | 2 | 0 | `"z"` |
| -1 | 16 | 9 | 0 | `"ffffffff"` |
| -1 | 2 | 33 | 0 | `1`が32個 |
| `INT_MIN` | 10 | 12 | 0 | `"-2147483648"` |
| `INT_MIN` | 16 | 9 | 0 | `"80000000"` |
| `INT_MAX` | 10 | 11 | 0 | `"2147483647"` |
| 10 | 1 | 8 | `EINVAL` | `""` |
| 10 | 37 | 8 | `EINVAL` | `""` |
| 10 | 10 | 0 | `EINVAL` | 変更しない |
| 10 | 10 | 8（`buffer`が`nullptr`） | `EINVAL` | （なし） |

**備考**

- 領域の重なりや、`buffer`の大きさの偽り（`size`が実際の配列より大きい）は、呼び出し側の責任（未定義動作）。
- `constexpr`。局所の配列に対して、コンパイル時に変換できる。**失敗する入力も、定数式のまま評価できる**（エラーは戻り値で、`errno`に書かないため）。
- `[[nodiscard]]`により、戻り値（エラーコード）を無視する呼び出しは、警告になる。

---

## 5. 基準実装

### 5.1 変換の核（`ilibc::detail`）

`strtol`と`strtoul`は、符号なしの絶対値を読む**共通の解析関数**を使う。

```cpp
namespace ilibc::detail {

struct parse_result {
    unsigned long magnitude;   // 絶対値（overflowがtrueのとき不定）
    const char   *end;         // 最後に変換した数字の次。変換なしなら入力の先頭
    bool          negative;    // 符号が'-'
    bool          overflow;    // 絶対値がunsigned longに収まらない
};

// 文字の数字としての値。数字でなければ36（どの基数よりも大きい）
constexpr int digit_value(char c) noexcept {
    if (c >= '0' && c <= '9') { return c - '0'; }
    if (c >= 'a' && c <= 'z') { return c - 'a' + 10; }
    if (c >= 'A' && c <= 'Z') { return c - 'A' + 10; }
    return 36;
}

constexpr parse_result parse_integer(const char *s, int base) noexcept {
    parse_result r = { 0UL, s, false, false };                  // 既定は「変換なし」
    if (base < 0 || base == 1 || base > 36) { return r; }       // 無効な基数

    std::size_t i = 0;
    while (std::isspace(s[i])) { ++i; }                         // 先頭の空白類
    if (s[i] == '+' || s[i] == '-') { r.negative = (s[i] == '-'); ++i; }

    // 接頭辞と基数の決定
    if ((base == 0 || base == 16) && s[i] == '0' && (s[i + 1] == 'x' || s[i + 1] == 'X')
        && digit_value(s[i + 2]) < 16) {
        i += 2;                                                 // 0x の直後が16進数字のときだけ読み飛ばす
        base = 16;
    } else if (base == 0) {
        base = (s[i] == '0') ? 8 : 10;
    }

    // 数字の読み取り（オーバーフローを、掛け算の前に判定する）
    const unsigned long b     = static_cast<unsigned long>(base);
    const unsigned long limit = ULONG_MAX / b;                  // これを超える値に、さらに桁を足すとオーバーフロー
    const unsigned long last  = ULONG_MAX % b;                  // 値がlimitと等しいとき、足せる最大の桁
    bool any = false;
    for (;; ++i) {
        const int d = digit_value(s[i]);
        if (d >= base) { break; }
        any = true;
        const unsigned long dv = static_cast<unsigned long>(d);
        if (r.overflow || r.magnitude > limit || (r.magnitude == limit && dv > last)) {
            r.overflow = true;                                  // 読み進めるが、値は更新しない
        } else {
            r.magnitude = r.magnitude * b + dv;
        }
    }
    if (any) { r.end = s + i; }
    return r;
}

}  // namespace ilibc::detail
```

### 5.2 `ilibc::`のerr版

```cpp
namespace ilibc {

constexpr long strtol(const char *nptr, char **endptr, int base, int *err) noexcept {
    const detail::parse_result r = detail::parse_integer(nptr, base);
    if (endptr != nullptr) { *endptr = const_cast<char *>(r.end); }

    constexpr unsigned long max_pos = static_cast<unsigned long>(LONG_MAX);
    const unsigned long limit = r.negative ? max_pos + 1UL : max_pos;     // 負なら|LONG_MIN|まで許す
    if (r.overflow || r.magnitude > limit) {
        if (err != nullptr) { *err = ERANGE; }
        return r.negative ? LONG_MIN : LONG_MAX;
    }
    return r.negative ? static_cast<long>(0UL - r.magnitude)              // 符号なしで否定してから変換
                      : static_cast<long>(r.magnitude);
}

constexpr unsigned long strtoul(const char *nptr, char **endptr, int base, int *err) noexcept {
    const detail::parse_result r = detail::parse_integer(nptr, base);
    if (endptr != nullptr) { *endptr = const_cast<char *>(r.end); }

    if (r.overflow) {
        if (err != nullptr) { *err = ERANGE; }
        return ULONG_MAX;
    }
    return r.negative ? 0UL - r.magnitude : r.magnitude;                  // 符号なしの否定（2^32を法とする）
}

constexpr long atol(const char *nptr, int *err) noexcept {
    return ilibc::strtol(nptr, nullptr, 10, err);
}

constexpr int atoi(const char *nptr, int *err) noexcept {
    return static_cast<int>(ilibc::strtol(nptr, nullptr, 10, err));      // ILP32: intとlongは同じ範囲
}

}  // namespace ilibc
```

`static_cast<long>(0UL - r.magnitude)`は、`magnitude`が2^31のとき`LONG_MIN`になる（C++20以降、符号なしから符号付きへの変換は2の補数で規定される）。

### 5.3 `std::`の関数

```cpp
namespace std {

[[nodiscard]] constexpr long labs(long j) noexcept { return j < 0 ? -j : j; }
[[nodiscard]] constexpr long abs(long j) noexcept  { return labs(j); }
[[nodiscard]] constexpr int  abs(int j) noexcept   { return j < 0 ? -j : j; }

[[nodiscard]] constexpr int atoi(const char *nptr) noexcept {
    return ilibc::atoi(nptr, &errno);
}
[[nodiscard]] constexpr long atol(const char *nptr) noexcept {
    return ilibc::atol(nptr, &errno);
}
constexpr long strtol(const char *nptr, char **endptr, int base) noexcept {
    return ilibc::strtol(nptr, endptr, base, &errno);
}
constexpr unsigned long strtoul(const char *nptr, char **endptr, int base) noexcept {
    return ilibc::strtoul(nptr, endptr, base, &errno);
}

}  // namespace std
```

- `errno`は、`<cerrno>`が定義する、単一のグローバル変数を指す左辺値マクロ。`&errno`は、その変数のアドレスになる。
- 定数式の評価中は、範囲外の経路で`errno`に書こうとして、定数式でなくなる（第3.4節）。

**正しさの要点**

- オーバーフロー判定は、`magnitude * b + dv`を計算する**前**に行う（`magnitude > limit`、または`magnitude == limit`かつ`dv > last`）。計算後に調べると、回り込みで検出を誤る。
- `strtol`の負の範囲は、絶対値が`|LONG_MIN|`（= `LONG_MAX + 1`）まで許す。`"-2147483648"`は範囲内。
- 範囲外のとき、`overflow`が立った後も数字を読み進めるため、`endptr`は有効な数字の終端を指す。

### 5.4 `ilibc::_itoa_s`

```cpp
namespace ilibc {

[[nodiscard]] constexpr int _itoa_s(int value, char *buffer, std::size_t size, int radix) noexcept {
    if (buffer == nullptr || size == 0) {                          // 1: 引数の誤り（bufferは触らない）
        return ILIBC_EINVAL;
    }
    if (radix < 2 || radix > 36) {                                 // 2: 基数の誤り
        buffer[0] = '\0';
        return ILIBC_EINVAL;
    }

    const bool negative = (radix == 10 && value < 0);
    unsigned magnitude = static_cast<unsigned>(value);             // 2の補数のビット列
    if (negative) { magnitude = 0U - magnitude; }                  // 絶対値（INT_MINも扱える）
    const unsigned base = static_cast<unsigned>(radix);

    std::size_t digits = 1;                                        // 桁数（値0も1桁）
    for (unsigned t = magnitude; t >= base; t /= base) { ++digits; }
    const std::size_t length = digits + (negative ? 1U : 0U);      // 終端を除く文字数

    if (size < length + 1) {                                       // 3: バッファ不足
        buffer[0] = '\0';
        return ILIBC_ERANGE;
    }

    buffer[length] = '\0';
    const std::size_t first = negative ? 1U : 0U;
    for (std::size_t i = length; i > first; ) {                    // 下位の桁から、後ろへ書く
        --i;
        const unsigned d = magnitude % base;
        buffer[i] = static_cast<char>(d < 10U ? '0' + d : 'a' + (d - 10U));
        magnitude /= base;
    }
    if (negative) { buffer[0] = '-'; }
    return 0;
}

template <std::size_t N>
[[nodiscard]] constexpr int _itoa_s(int value, char (&buffer)[N], int radix) noexcept {
    return ilibc::_itoa_s(value, buffer, N, radix);
}

}  // namespace ilibc
```

**正しさの要点**

- 必要な大きさを、**書き込む前に**計算する。バッファ不足のとき、途中まで書いた状態を残さない（`buffer[0]`の`'\0'`のみ）。
- 絶対値は、符号なしで求める（`0U - magnitude`）。`INT_MIN`の符号付きの否定（オーバーフロー）を避ける。
- 桁数の計算で、`t /= base`を、`t`が`base`未満になるまで繰り返す。値0でも、桁数は1。
- 書き込みは、**後ろから前へ**。`first`（符号がある場合は1）で止めて、符号を`buffer[0]`に書く。
- `errno`もグローバル変数も使わないため、全体が`constexpr`で、再入可能。
- 実装では、`ILIBC_EINVAL`、`ILIBC_ERANGE`を使う（ILIBC-HDR-001 第3.4節）。

---

## 6. 実装規則

### 6.1 ファイル構成

| ファイル | 内容 |
|---|---|
| `include/cstdlib` | `std::`の関数（第5.3節）、`EXIT_*`、`MB_CUR_MAX` |
| `include/stdlib.h` | 互換ラッパ（第2.2節） |
| `include/ilibc/stdlib.hpp` | `ilibc::detail`の解析関数、`ilibc::`のerr版（第5.1節、第5.2節）、`ilibc::_itoa_s`（第5.4節） |

`.cpp`ファイルは不要（ヘッダのみ）。

実装では、`errno`、`ERANGE`、`EINVAL`を、内部マクロ`ILIBC_ERRNO`、`ILIBC_ERANGE`、`ILIBC_EINVAL`と書く。`ILIBC_HOST_TEST`のとき、公開マクロ（`errno`、`ERANGE`、`EINVAL`、`EXIT_SUCCESS`、`EXIT_FAILURE`、`MB_CUR_MAX`）は定義されない（ホストlibcと衝突するため。ILIBC-HDR-001 第3.4節、第5.1節）。本書のコードブロックは、読みやすさのため公開マクロの名前で書いている。

### 6.2 依存関係

| 関数 | 依存先 |
|---|---|
| `abs`、`labs` | なし |
| `strtol`、`strtoul`（本体は`ilibc::`のerr版） | `std::isspace`（`<cctype>`。ヘッダ内の`constexpr`関数）、`<cerrno>`（`ERANGE`、`errno`）、`<climits>` |
| `atoi`、`atol` | `ilibc::strtol` |
| `ilibc::_itoa_s` | なし（整数の除算・剰余を使う） |

すべてヘッダ内のインライン関数どうしの依存で、リンク時の依存はない（概要仕様書 第11章）。

### 6.3 リソース

| 項目 | 値 |
|---|---|
| ROM定数データ | 0バイト |
| RAM | 0バイト（`errno`は`<cerrno>`が持つ） |
| スタック | 数ワード以下 |
| ヒープ | なし |

---

## 7. 規格との差異

| # | 内容 | 扱い |
|---|---|---|
| 1 | **全関数に`constexpr`を付ける** | 意図的な逸脱（概要仕様書 第8.2節、逸脱5） |
| 2 | **`atoi`／`atol`が、範囲外で`ERANGE`を設定する** | C89が許す範囲。範囲外の結果（規格では未定義）を定義する（概要仕様書 逸脱7） |
| 3 | `std::`の4関数に加えて、エラー出力先を引数で指定する`ilibc::`版を提供する | 拡張（概要仕様書 第8.5節A） |
| 4 | 無効な基数のとき、変換せずに0を返し、`*endptr = nptr`とする。エラーは報告しない（C99は`EINVAL`を許す） | 規格が未規定。D-L1 |
| 5 | `long long`版、`strtoll`、`strtoull`、`llabs`等を提供しない | 概要仕様書 D-12 |
| 6 | `div`、`ldiv`、`rand`、`srand`、`qsort`、`bsearch`、`malloc`系、`abort`／`exit`／`atexit`、`getenv`、`system`、`atof`、`strtod`、マルチバイト関数を提供しない | 概要仕様書 第7章 |
| 7 | `abs(INT_MIN)`等は未定義動作（規格どおり）。定数式評価中のみ診断される | 規格が許す範囲 |
| 8 | **`ilibc::_itoa_s`は、非標準の拡張関数**（C89にも、C++規格にもない） | 概要仕様書 第8.5節C |
| 9 | MSVCの`_itoa_s`との差異: (a)`ilibc::`にのみ置く（グローバルに導入しない）。(b)戻り値は`int`（`errno`と同じ型。MSVCの`errno_t`に相当）で、エラーコードを返す点は同じ。(c)無効なパラメータのハンドラは呼ばず、`errno`も変更しない（戻り値のみで報告する）。(d)バッファ不足は`ERANGE`（MSVCとの互換性は保証しない）。(e)`constexpr`。(f)`_ltoa_s`、`_ultoa_s`等は提供しない | 意図的な差異 |

---

## 8. 試験仕様（観点）

詳細な手順は、ILIBC-TST-001で定める。

| ID | 方式 | 観点 | 合否基準 |
|---|---|---|---|
| TC-SL-01 | コンパイル時（`static_assert`） | `abs`／`labs`を、0、±1、`INT_MAX`、`-INT_MAX`、`LONG_MAX`、`-LONG_MAX`で検証する。`abs(int)`と`abs(long)`のオーバーロード選択（戻り値型が`int`／`long`） | 全一致 |
| TC-SL-02 | コンパイル失敗試験 | (a)定数式で`abs(INT_MIN)`、`labs(LONG_MIN)`。(b)`abs(1LL)`、`abs(1U)`、`abs(1.0)`（曖昧）。いずれもコンパイルエラーになる | 全件エラー |
| TC-SL-03 | コンパイル時 | `ilibc::`のerr版で、第4.2節の変換の例の全行（`strtol`、`strtoul`）を検証する: 戻り値、`endptr`の位置、`*err` | 全一致 |
| TC-SL-04 | コンパイル時 | `std::`版を、範囲内の入力で検証する（`strtol`、`strtoul`、`atoi`、`atol`） | 全一致 |
| TC-SL-05 | コンパイル失敗試験 | 定数式で、`std::strtol`、`std::strtoul`、`std::atoi`、`std::atol`に範囲外の入力を与える。コンパイルエラーになる（`errno`への書き込み） | 全件エラー |
| TC-SL-06 | 実行時 | **err版の規則**: (a)成功時に`*err`を書かない（呼び出し前に番兵値を入れておき、不変）。(b)範囲外で`*err == ERANGE`。(c)`err`が`nullptr`でも動作する。(d)`errno`を変更しない（呼び出し前に番兵値を設定） | 失敗数0 |
| TC-SL-07 | 実行時 | **`std::`版の`errno`**: (a)成功時に`errno`が不変（呼び出し前に番兵値を設定）。(b)範囲外で`ERANGE`。(c)`atoi(s)`と`(int)strtol(s, nullptr, 10)`が、戻り値と`errno`の両方で一致（第4.2節の例と、乱数入力） | 失敗数0 |
| TC-SL-08 | コンパイル時／実行時 | **基数**: `base`が0と2〜36のそれぞれで、(a)`base`以上の数字で止まる。(b)英字の大文字・小文字を区別しない。(c)接頭辞の扱い（`0x`／`0X`、先頭の`0`、`"0x"`単独、`"0xg"`、`"0x1g"`）。(d)`"0x"`が`base`=8、10では接頭辞にならない | 全一致 |
| TC-SL-09 | コンパイル時／実行時 | **無効な基数**（−1、1、37、`INT_MIN`、`INT_MAX`）: 0を返し、`*endptr == nptr`、エラーなし | 全一致 |
| TC-SL-10 | コンパイル時／実行時 | **空白・符号**: 空白類6文字（0x20、0x09〜0x0D）が読み飛ばされ、0x1C〜0x1Fは読み飛ばされない。`"+"`、`"-"`、`"+-1"`、`"- 1"`は変換なし（`*endptr == nptr`）。符号の後に空白があっても変換しない | 全一致 |
| TC-SL-11 | 実行時 | **範囲の境界**: 各基数（0、2〜36）について、`LONG_MAX`、`LONG_MAX + 1`、`LONG_MIN`、`LONG_MIN − 1`、`ULONG_MAX`、`ULONG_MAX + 1`を表す文字列を、実装と独立な参照（64ビット整数による、基数変換の別実装）で生成して検証する。範囲外の後も、`endptr`が有効な数字の終端を指す | 全一致 |
| TC-SL-12 | ホスト差分 | `ILIBC_HOST_TEST`で別名前空間にした関数を、ホストのlibcと比較する。入力は、数字、英字、符号、空白、`x`、`X`からなる乱数文字列、基数は0、2〜36。戻り値、`endptr`のオフセット、`ERANGE`の有無を比較する（無効な基数は、ホストが`EINVAL`を設定するため、除外する） | 全一致 |
| TC-SL-13 | サニタイザ | ホスト差分試験を、ASan／UBSanで実行する（符号付きオーバーフロー、範囲外アクセスがない） | 検出0 |
| TC-SL-14 | ターゲット実行 | TC-SL-06〜TC-SL-11を、QEMU等のフリースタンディング環境で実行する。引数を`volatile`経由にして、畳み込みを避ける | 失敗数0 |
| TC-SL-15 | 最適化レベル | `-O0`、`-Os`、`-O2`でTC-SL-14が合格する | 全レベルで合格 |
| TC-SL-16 | ヘッダ | (a)`<cstdlib>`のみを含む翻訳単位で、`std::atoi`が使え、グローバルの`::atoi`が見つからない（コンパイル失敗試験）。(b)`<stdlib.h>`を含むと、両方が使える。(c)`EXIT_SUCCESS`、`EXIT_FAILURE`、`MB_CUR_MAX`が定義されている。(d)`std::div`、`std::ldiv`、`std::rand`、`std::srand`、`std::qsort`、`std::bsearch`、`std::malloc`、`std::exit`、`div_t`、`RAND_MAX`が存在しない（コンパイル失敗試験） | すべて満足 |
| TC-SL-17 | 成果物検査 | `<cstdlib>`を使うだけのプログラムをリンクして、(a)`<cstdlib>`由来の未解決シンボルがない（`errno`の変数を除く）。(b)C++ランタイム依存や静的初期化がない（概要仕様書 第12.2節） | すべて満足 |
| TC-SL-18 | コンパイル時（`static_assert`） | **`_itoa_s`の変換結果**: `ilibc::_itoa_s`で、第4.5節の変換の例の全行、および、値（0、1、−1、9、10、35、36、255、256、65535、`INT_MAX`、`INT_MIN`）×基数（2、8、10、16、36）の表を検証する。期待値は**手書き**。`size`が必要な大きさちょうどで、成功（戻り値0）すること | 全一致 |
| TC-SL-19 | コンパイル時 | **戻り値（エラーコード）とバッファの内容**: (a)`buffer`が`nullptr`（戻り値`EINVAL`）、(b)`size`が0（`EINVAL`。`buffer`に番兵値を入れておき、不変）、(c)基数が−1、0、1、37、`INT_MIN`、`INT_MAX`（`EINVAL`、`buffer[0]`が`'\0'`）、(d)`size`が、必要な大きさより1小さい（`ERANGE`、`buffer[0]`が`'\0'`で、他は不変）。(e)条件が重なる場合（`nullptr`と基数の誤り、`size`が0と基数の誤り）は、検査の順序（第4.5節）に従い`EINVAL`で、`buffer`が不変。**失敗する入力も、定数式のまま評価できる** | 全一致 |
| TC-SL-20 | コンパイル時 | **呼び出し形と型**: ポインタ版（#1）と配列版（#2）が、同じ入力で同じ戻り値・同じ`buffer`を返す。配列版で`N`が型から得られる。戻り値の型が`int`（`std::is_same_v`）。`noexcept`である | 全一致 |
| TC-SL-21 | コンパイル失敗 | **不正な呼び出し**: (a)`char *`のポインタに、`size`を省略した3引数形。(b)`unsigned char`、`signed char`の配列を、バッファに渡す。(c)**戻り値を無視する呼び出し**が、`-Werror=unused-result`でエラーになる（`[[nodiscard]]`）。(d)`<cstdlib>`と`<stdlib.h>`をインクルードしても、`::_itoa_s`と`std::_itoa_s`が存在しない。いずれもコンパイルエラーになる | 全件エラー |
| TC-SL-22 | 実行時 | **`errno`の不変**: 成功と、すべての失敗（`nullptr`、`size`が0、基数の誤り、バッファ不足）で、`errno`が変化しない（呼び出し前に番兵値を設定）。戻り値が、0、`ILIBC_EINVAL`、`ILIBC_ERANGE`のいずれかである | 失敗数0 |
| TC-SL-23 | 実行時、ホスト差分 | **参照と往復**: 基数2〜36のそれぞれで、(a)境界値、−2000〜2000の全値、乱数（基数ごとに10万個以上）の変換結果を、**実装から独立した参照**（`unsigned long long`で桁を取り出す別実装）と比較する。基数8、10、16は、ホストの`snprintf`（`%o`、`%d`、`%x`）とも比較する。(b)**往復**: 基数10は`ilibc::strtol`、それ以外は`ilibc::strtoul`で戻して、元の値（符号なしのビット列）に一致する。(c)文字数が33以下 | 全一致 |
| TC-SL-24 | 実行時、ホスト差分 | **書き込み範囲**: `size`が、必要な大きさちょうど、＋1、＋8のとき、前後のカナリア（`0xA5`）が壊れない。失敗時に書き込まれるのは、`buffer[0]`のみ（`nullptr`と`size`が0では、何も書かれない） | 失敗数0 |

---

## 9. 未決事項

| ID | 内容 | 候補 | 既定（暫定） |
|---|---|---|---|
| D-L1 | 無効な基数の報告 | 報告しない（既定。0を返し、`*endptr = nptr`）／`ERANGE`を流用する／`EINVAL`で報告する（`EINVAL`は`_itoa_s`のために定義済み） | 報告しない（`EINVAL`は`strtol`系では使わない。決定済みの「報告は`ERANGE`のみ」を維持） |
| — | `EXIT_SUCCESS`、`EXIT_FAILURE`、`MB_CUR_MAX`を定義し続けるか | 概要仕様書 D-17と同一 | 残す |
| — | `abort`等の扱い | 概要仕様書 D-11と同一 | 利用者が提供 |

---

## 改版履歴

| 版 | 日付 | 内容 |
|---|---|---|
| 0.1 | 2026-10-10 | 初版ドラフト（概要仕様書 v0.3に基づく新規作成） |
| 0.2 | 2026-10-10 | 名称をilibcに変更（`libc89`→`ilibc`、`LIBC89`→`ILIBC`、文書ID接頭辞）。過去の改版履歴中の名称も置き換えた。内容の変更なし |
| 0.2（訂正） | 2026-10-10 | ヘッダ仕様書の訂正に伴い、実装での内部マクロ（`ILIBC_ERRNO`、`ILIBC_ERANGE`）の使用を追記 |
| 0.3 | 2026-10-10 | 非標準の拡張関数`ilibc::_itoa_s`を追加（第2.5節、第4.5節、第5.4節）。`EINVAL`の使用を追記。試験TC-SL-18〜24を追加。未決事項D-L2を追加 |
| 0.3（訂正） | 2026-10-10 | `_itoa_s`の仕様を、MSVC準拠に改訂: `err`引数と`errno`版を廃止し、**`int`のエラーコードを戻り値で返す**（成功は0）。シグネチャは4つから2つ（ポインタ版、配列版）に。`errno`に依存せず、失敗の結果までコンパイル時に評価できる。試験TC-SL-18〜22を更新。未決事項D-L2を削除 |
