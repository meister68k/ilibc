# `<cctype>` 関数仕様書

| 項目 | 内容 |
|---|---|
| 文書ID | ILIBC-CTP-001 |
| ライブラリ名 | ilibc（旧仮称: libc89） |
| 版 | 0.3（ドラフト） |
| 親文書 | ILIBC-OVR-001 全体概要仕様書 v0.5 |
| 対象関数 | CT-01〜CT-13（13個） |
| 対象ヘッダ | `<cctype>`（正）、`<ctype.h>`（互換ラッパ） |
| 動作の基準 | ISO/IEC 9899:1990 §7.3（`"C"`ロケール）。インタフェースはISO/IEC 14882:2024（C++23）からの意図的な逸脱を含む（第7章） |
| 実装方式 | **数式方式**（範囲比較）。テーブルを持たない |

---

## 1. 範囲と参照文書

- 本書は、`<cctype>`の13関数の宣言、動作、基準実装、試験観点を定める。
- 共通の設計規則は、全体概要仕様書の第8章、第9章に従う。

| 参照文書 | 参照箇所 |
|---|---|
| ILIBC-OVR-001 | 第3章（逸脱一覧）、第5.1節（関数一覧）、第8.2節（`constexpr`方針）、第8.3節（型安全方針）、第9.1節（ロケール） |
| ILIBC-TST-001 | 試験の共通方式 |

---

## 2. 提供物

### 2.1 標準名（`std::`）

`<cctype>`は、`namespace std`に次の13関数を宣言する。**すべて`constexpr`、`noexcept`、`[[nodiscard]]`**。

| ID | 関数 | シグネチャ |
|---|---|---|
| CT-01 | `isalnum` | `[[nodiscard]] constexpr bool isalnum(int c) noexcept;` |
| CT-02 | `isalpha` | `[[nodiscard]] constexpr bool isalpha(int c) noexcept;` |
| CT-03 | `iscntrl` | `[[nodiscard]] constexpr bool iscntrl(int c) noexcept;` |
| CT-04 | `isdigit` | `[[nodiscard]] constexpr bool isdigit(int c) noexcept;` |
| CT-05 | `isgraph` | `[[nodiscard]] constexpr bool isgraph(int c) noexcept;` |
| CT-06 | `islower` | `[[nodiscard]] constexpr bool islower(int c) noexcept;` |
| CT-07 | `isprint` | `[[nodiscard]] constexpr bool isprint(int c) noexcept;` |
| CT-08 | `ispunct` | `[[nodiscard]] constexpr bool ispunct(int c) noexcept;` |
| CT-09 | `isspace` | `[[nodiscard]] constexpr bool isspace(int c) noexcept;` |
| CT-10 | `isupper` | `[[nodiscard]] constexpr bool isupper(int c) noexcept;` |
| CT-11 | `isxdigit` | `[[nodiscard]] constexpr bool isxdigit(int c) noexcept;` |
| CT-12 | `tolower` | `[[nodiscard]] constexpr int tolower(int c) noexcept;` |
| CT-13 | `toupper` | `[[nodiscard]] constexpr int toupper(int c) noexcept;` |

- 分類関数（CT-01〜CT-11）は`bool`を返す。変換関数（CT-12、CT-13）は文字を返すため`int`のまま。
- 関数形式マクロとしては定義しない（C++規格が禁じる）。
- すべてヘッダ内の`constexpr inline`関数。**`.cpp`ファイルもリンクシンボルも持たない**。
- 関数のアドレスを取ることは許す。

### 2.2 互換ラッパ `<ctype.h>`

```cpp
#include <cctype>
using std::isalnum;  using std::isalpha;  using std::iscntrl;  using std::isdigit;
using std::isgraph;  using std::islower;  using std::isprint;  using std::ispunct;
using std::isspace;  using std::isupper;  using std::isxdigit;
using std::tolower;  using std::toupper;
```

独自の宣言・定義を持たない。

### 2.3 設定

なし。`<cctype>`に関する設定マクロは存在しない。

---

## 3. 分類の定義

### 3.1 前提

- `"C"`ロケールのみ。実行文字集合はASCII（概要仕様書 第2.1節の`static_assert`で検証）。
- 引数は任意の`int`値を受け付ける（第4.1節）。

### 3.2 区間表（正本）

次の区間表が、13関数の定義の**唯一の正本**である。実装、試験は、すべてこの表から導く。○は、その区間のすべての文字で関数が真になることを示す。区間表にない値（0x80以上、負の値など）は、すべて偽。

| 区間 | 内容 | 個数 | iscntrl | isprint | isgraph | isspace | isdigit | isxdigit | isupper | islower | isalpha | isalnum | ispunct |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 0x00–0x08 | 制御文字 | 9 | ○ | | | | | | | | | | |
| 0x09–0x0D | `\t \n \v \f \r` | 5 | ○ | | | ○ | | | | | | | |
| 0x0E–0x1F | 制御文字 | 18 | ○ | | | | | | | | | | |
| 0x20 | 空白 | 1 | | ○ | | ○ | | | | | | | |
| 0x21–0x2F | `! " # $ % & ' ( ) * + , - . /` | 15 | | ○ | ○ | | | | | | | | ○ |
| 0x30–0x39 | `0`–`9` | 10 | | ○ | ○ | | ○ | ○ | | | | ○ | |
| 0x3A–0x40 | `: ; < = > ? @` | 7 | | ○ | ○ | | | | | | | | ○ |
| 0x41–0x46 | `A`–`F` | 6 | | ○ | ○ | | | ○ | ○ | | ○ | ○ | |
| 0x47–0x5A | `G`–`Z` | 20 | | ○ | ○ | | | | ○ | | ○ | ○ | |
| 0x5B–0x60 | `` [ \ ] ^ _ ` `` | 6 | | ○ | ○ | | | | | | | | ○ |
| 0x61–0x66 | `a`–`f` | 6 | | ○ | ○ | | | ○ | | ○ | ○ | ○ | |
| 0x67–0x7A | `g`–`z` | 20 | | ○ | ○ | | | | | ○ | ○ | ○ | |
| 0x7B–0x7E | `{ \| } ~` | 4 | | ○ | ○ | | | | | | | | ○ |
| 0x7F | DEL | 1 | ○ | | | | | | | | | | |
| 上記以外のすべての`int`（0x80以上、`EOF`、負の値、256以上） | | — | | | | | | | | | | | |

**真となる文字の個数**（0〜127の128値のうち）

| 関数 | 個数 | 関数 | 個数 |
|---|---|---|---|
| `iscntrl` | 33 | `isupper` | 26 |
| `isprint` | 95 | `islower` | 26 |
| `isgraph` | 94 | `isalpha` | 52 |
| `isspace` | 6 | `isalnum` | 62 |
| `isdigit` | 10 | `ispunct` | 32 |
| `isxdigit` | 22 | | |

### 3.3 関係式（不変条件）

**任意の`int`値**`c`について、次が成り立つこと（試験に使う）。

| # | 関係式 |
|---|---|
| R1 | `isalpha(c)` ⇔ `isupper(c)` または `islower(c)` |
| R2 | `isalnum(c)` ⇔ `isalpha(c)` または `isdigit(c)` |
| R3 | `isgraph(c)` ⇔ `isalnum(c)` または `ispunct(c)` |
| R4 | `isprint(c)` ⇔ `isgraph(c)` または `c == ' '` |
| R5 | `isupper`、`islower`、`isdigit`、`ispunct`は、互いに排他（同時に真にならない） |
| R6 | `isxdigit(c)` ならば `isdigit(c)` または `isalpha(c)` |
| R7 | `iscntrl(c)` ならば `isprint(c)` は偽 |
| R8 | `isspace(c)` かつ `c != ' '` ならば `iscntrl(c)` は真 |
| R9 | `c < 0` または `c > 127` ならば、13の分類関数すべてが偽 |

---

## 4. 共通規則

### 4.1 入力

- 引数は、**任意の`int`値**を受け付ける。規格の「`unsigned char`で表現できる値または`EOF`」という制約は、本ライブラリでは課さない。
- 領域外の値（負の値、`EOF`、128以上、256以上）は、**分類関数で`false`、変換関数で入力値そのもの**を返す。未定義動作は生じない。
- 結果は、`c`の下位8ビットだけを見るような別名化（たとえば`c`と`c + 256`が同じ結果になる）を起こさない。
- したがって、符号付き`char`の負の値（0x80以上の文字）を渡しても安全。`EOF`（-1）との衝突も、結果の差を生まない（どちらも偽、または不変）。

### 4.2 戻り値

| 関数群 | 戻り値 |
|---|---|
| 分類関数（CT-01〜CT-11） | `bool`。真なら`true`、偽なら`false` |
| 変換関数（CT-12、CT-13） | `int`。変換後の値、または変換対象外のとき入力値そのもの |

### 4.3 `errno`・状態・再入・資源

| 項目 | 内容 |
|---|---|
| `errno` | 変更しない |
| 内部状態 | なし。グローバル変数、静的変数、定数テーブルを持たない |
| 再入可能性 | 完全に再入可能。割り込みハンドラ、複数コンテキストから、制約なく呼べる |
| スタック | 追加のスタックを要しない |
| ヒープ | 使わない |
| ROM定数データ／RAM | 0バイト／0バイト |
| 依存する他関数 | なし |

### 4.4 `constexpr`

- 全13関数が`constexpr`。定数式の中で、`static_assert`、テンプレート引数、`constexpr`変数の初期化に使える。
- 領域外の入力も定義済みなので、定数式の評価が失敗する入力はない。
- 引数が定数のとき、`constexpr`でなくても、`inline`により最適化でコンパイル時に畳み込まれる。

### 4.5 属性

`noexcept`、`[[nodiscard]]`: 第2.1節のとおり。全13関数に付ける。

---

## 5. 関数仕様

### 5.1 分類関数（CT-01〜CT-11）

| ID | 関数 | 真を返す文字 | 個数 | 区間表の定義 |
|---|---|---|---|---|
| CT-01 | `isalnum` | 英数字 | 62 | `A–Z`、`a–z`、`0–9` |
| CT-02 | `isalpha` | 英字 | 52 | `A–Z`、`a–z` |
| CT-03 | `iscntrl` | 制御文字 | 33 | 0x00〜0x1F、0x7F |
| CT-04 | `isdigit` | 10進数字 | 10 | `0–9` |
| CT-05 | `isgraph` | 空白以外の表示可能文字 | 94 | 0x21〜0x7E |
| CT-06 | `islower` | 小文字 | 26 | `a–z` |
| CT-07 | `isprint` | 空白を含む表示可能文字 | 95 | 0x20〜0x7E |
| CT-08 | `ispunct` | 区切り文字（記号） | 32 | 0x21〜0x7Eのうち、英数字を除く |
| CT-09 | `isspace` | 空白類 | 6 | 0x20、0x09〜0x0D |
| CT-10 | `isupper` | 大文字 | 26 | `A–Z` |
| CT-11 | `isxdigit` | 16進数字 | 22 | `0–9`、`A–F`、`a–f` |

**補足**

- **`isspace`**: 6文字は、スペース、水平タブ（`\t`）、改行（`\n`）、垂直タブ（`\v`）、改ページ（`\f`）、復帰（`\r`）。0x1C〜0x1F（FS、GS、RS、US）は含めない（`"C"`ロケールの規定）。
- **`isprint`と`isgraph`**: 違いはスペース（0x20）のみ。スペースは`isprint`が真、`isgraph`が偽。
- **`ispunct`**: 表示可能文字（`isgraph`）のうち、`isalnum`が偽のもの。スペースは含まない。
- **`isxdigit`**: 大文字・小文字の区別なし。
- **`isalpha`／`isalnum`**: ASCIIの英字のみ。アクセント付き文字等（0x80以上）は偽。

### 5.2 変換関数（CT-12、CT-13）

| ID | 関数 | 変換する範囲 | 変換内容 |
|---|---|---|---|
| CT-12 | `tolower` | `A–Z`（0x41〜0x5A） | 0x20を加える（`a–z`） |
| CT-13 | `toupper` | `a–z`（0x61〜0x7A） | 0x20を引く（`A–Z`） |

- 変換対象外（上記以外の**すべての`int`**。`EOF`、負の値、128以上を含む）は、入力値をそのまま返す。
- 性質（試験に使う）:

| # | 性質 |
|---|---|
| V1 | `isupper(c)` ならば `tolower(c) == c + 32` かつ `islower(tolower(c))` |
| V2 | `islower(c)` ならば `toupper(c) == c - 32` かつ `isupper(toupper(c))` |
| V3 | `isupper(c)` でも`islower(c)`でもないとき、`tolower(c) == c` かつ `toupper(c) == c`（任意の`int`で成り立つ） |
| V4 | `isalpha(c)`のとき、`tolower(toupper(c)) == tolower(c)`、`toupper(tolower(c)) == toupper(c)` |
| V5 | 変換される文字は、それぞれ26個 |

### 5.3 基準実装

次の実装を、そのまま使う。符号なし整数の剰余演算により範囲判定を1回の比較で行う。**任意の`int`で、符号付きオーバーフローを起こさない**。

```cpp
namespace ilibc::detail {

// lo <= c <= hi を判定する。cが負でも、大きくても安全（符号なしに変換して比較）。
constexpr bool in_range(int c, int lo, int hi) noexcept {
    return static_cast<unsigned>(c) - static_cast<unsigned>(lo)
           <= static_cast<unsigned>(hi - lo);
}

}  // namespace ilibc::detail

namespace std {

[[nodiscard]] constexpr bool isupper(int c) noexcept { return ilibc::detail::in_range(c, 'A', 'Z'); }
[[nodiscard]] constexpr bool islower(int c) noexcept { return ilibc::detail::in_range(c, 'a', 'z'); }
[[nodiscard]] constexpr bool isdigit(int c) noexcept { return ilibc::detail::in_range(c, '0', '9'); }

[[nodiscard]] constexpr bool isalpha(int c) noexcept { return isupper(c) || islower(c); }
[[nodiscard]] constexpr bool isalnum(int c) noexcept { return isalpha(c) || isdigit(c); }

[[nodiscard]] constexpr bool isxdigit(int c) noexcept {
    return isdigit(c) || ilibc::detail::in_range(c, 'A', 'F') || ilibc::detail::in_range(c, 'a', 'f');
}

[[nodiscard]] constexpr bool isspace(int c) noexcept {
    return c == ' ' || ilibc::detail::in_range(c, 0x09, 0x0D);
}
[[nodiscard]] constexpr bool iscntrl(int c) noexcept {
    return ilibc::detail::in_range(c, 0x00, 0x1F) || c == 0x7F;
}
[[nodiscard]] constexpr bool isprint(int c) noexcept { return ilibc::detail::in_range(c, 0x20, 0x7E); }
[[nodiscard]] constexpr bool isgraph(int c) noexcept { return ilibc::detail::in_range(c, 0x21, 0x7E); }
[[nodiscard]] constexpr bool ispunct(int c) noexcept { return isgraph(c) && !isalnum(c); }

[[nodiscard]] constexpr int tolower(int c) noexcept { return isupper(c) ? c + ('a' - 'A') : c; }
[[nodiscard]] constexpr int toupper(int c) noexcept { return islower(c) ? c - ('a' - 'A') : c; }

}  // namespace std
```

**正しさの要点**

- `c`が`lo`未満のとき、`static_cast<unsigned>(c) - lo`は大きな値に回り込み、上限を超えるため偽になる。
- `c`が負のとき、`static_cast<unsigned>(c)`は2^31以上となり、`lo`〜`hi`の範囲（0〜127）に入らない。したがって、負の値が範囲内と判定されることはない。
- 文字リテラル（`'A'`等）は、`int`に昇格して使う。ASCIIは第2.1節の`static_assert`で保証する。

---

## 6. 実装規則

### 6.1 構成

| ファイル | 内容 |
|---|---|
| `include/cctype` | `ilibc::detail::in_range`と、`std::`の13関数の`constexpr`定義（第5.3節） |
| `include/ctype.h` | 互換ラッパ（第2.2節） |

`.cpp`ファイルは不要。依存するヘッダは`ilibc/config.hpp`のみ（名前空間マクロ`ILIBC_NS_BEGIN`／`ILIBC_NS_END`のため。ILIBC-HDR-001 第3.4節）。実装では、本書のコードブロックの`namespace std { ... }`を、このマクロで書く。

### 6.2 リソース

| 項目 | 値 |
|---|---|
| ROM（定数データ） | 0バイト |
| RAM | 0バイト |
| スタック | 追加なし |
| 実行時の判定 | 数回の比較（`isalpha`は2回、`isdigit`は1回、`ispunct`は数回） |
| 定数引数 | コンパイル時に畳み込まれる |

### 6.3 依存関係

- 他のライブラリ関数に依存しない（概要仕様書 第11章のL0）。
- `<cstdlib>`の`strtol`、`strtoul`が、`isspace`に依存する（ILIBC-STL-001）。

---

## 7. 規格との差異

| # | 内容 | 扱い |
|---|---|---|
| 1 | **分類関数11個が`bool`を返す**（規格は`int`）。関数の型が`int(int)`から`bool(int)`に変わる。`int (*f)(int) = std::isalpha;`のような関数ポインタの代入はコンパイルエラーになる。`if (isdigit(c))`、`isdigit(c) != 0`、`int r = isdigit(c);`は変わらず動く | 意図的な逸脱（概要仕様書 第3章 逸脱6） |
| 2 | **引数に任意の`int`を受け付け**、領域外で結果を定義する（規格は未定義動作） | 規格が許す範囲の拡張。組込みでは安全側に働く |
| 3 | **`constexpr`を付ける** | 意図的な逸脱（概要仕様書 第8.2節、逸脱5） |
| 4 | `isblank`（C++の`<cctype>`が要求するC99由来の関数）を提供しない | 概要仕様書 D-12 |
| 5 | ロケール（`std::setlocale`、`<locale>`の`std::isalpha`テンプレート等）に対応しない | `"C"`ロケール固定 |

---

## 8. 試験仕様（観点）

詳細な手順は、ILIBC-TST-001で定める。コンパイル時テストの評価回数が、コンパイラの上限（Clangの`-fconstexpr-steps`、GCCの`-fconstexpr-ops-limit`）を超える場合は、関数ごとに`static_assert`を分割する。

| ID | 方式 | 観点 | 合否基準 |
|---|---|---|---|
| TC-CT-01 | コンパイル時（`static_assert`） | 13関数を、**−1から255までの全257値**で、区間表（第3.2節）から独立に書いた参照（区間の列挙）と比較する | 全一致 |
| TC-CT-02 | コンパイル時 | **領域外・極端値**: `INT_MIN`、`INT_MIN + 1`、`-256`、`-2`、`256`、`INT_MAX`。さらに、0〜255の全値`c`について、`c + 256`、`c + 0x10000`、`c + 0x01000000`、`c \| 0x80000000`（`unsigned`経由で`int`に変換）を引数にして、分類関数がすべて偽、変換関数が入力値そのものであること（下位ビットだけを見る実装の誤りを検出する） | 全一致 |
| TC-CT-03 | コンパイル時 | 0〜127で真となる文字の個数（第3.2節の表）を数える | 全関数で一致 |
| TC-CT-04 | コンパイル時 | 関係式R1〜R9（第3.3節）を、−1〜255、および TC-CT-02の値で検証する | 全成立 |
| TC-CT-05 | コンパイル時 | 変換関数の性質V1〜V5（第5.2節）、領域外の入力値がそのまま返ること | 全成立 |
| TC-CT-06 | コンパイル時 | 型の検査: 分類11関数の戻り値型が`bool`、変換2関数が`int`であること（`std::is_same_v<decltype(std::isalpha(0)), bool>`等）。全13関数が`noexcept`であること。定数式で呼べること | 全成立 |
| TC-CT-07 | ホスト差分 | `ILIBC_HOST_TEST`で別名前空間にした自作関数を、ホストのlibc（`setlocale(LC_ALL, "C")`）と、**−1〜255の全257値**で比較する。分類関数は**真偽**（`bool`と、ホストの0／非0）、変換関数は値を比較する。領域外はホストlibcが未定義動作のため、比較しない | 全一致 |
| TC-CT-08 | ターゲット実行 | TC-CT-01、TC-CT-02を、実行時にも行う（`constexpr`の評価と、実行時のコード生成の食い違いを検出する）。引数を`volatile`経由にして、畳み込みを避ける | 失敗数0 |
| TC-CT-09 | 最適化レベル | `-O0`、`-Os`、`-O2`でTC-CT-08が合格する | 全レベルで合格 |
| TC-CT-10 | ヘッダ | (a)`<cctype>`のみを含む翻訳単位で、`std::isalpha`が使え、グローバルの`::isalpha`が見つからない（コンパイル失敗試験）。(b)`<ctype.h>`を含むと、`::isalpha`と`std::isalpha`の両方が使える | すべて満足 |
| TC-CT-11 | 成果物検査 | `<cctype>`を使うだけのプログラムをリンクして、(a)`<cctype>`由来のシンボル、(b)`<cctype>`由来の定数データ（`.rodata`）、(c)C++ランタイム依存や静的初期化（概要仕様書 第12.2節）が、いずれもないこと | すべて満足 |

**TC-CT-01の参照は、実装と独立に書く**こと。たとえば、「大文字は`A`から`Z`の26文字」という列挙を、実装の演算式と別の書き方（文字列リテラルの走査等）で作る。同じ式を2回書くだけでは、誤りを検出できない。

---

## 9. 未決事項

なし。旧版（v0.1）の未決事項（D-02、D-C1〜C3）は、すべて解決した。

| 旧ID | 内容 | 決定 |
|---|---|---|
| D-02 | テーブル方式／演算方式 | **数式方式のみ** |
| D-C1 | `ilibc::`の分類関数の戻り値型 | `ilibc::`版を廃止。`std::`版が`bool` |
| D-C2 | `ilibc::`版が受け付ける型 | `ilibc::`版を廃止。任意の`int`を受け付ける |
| D-C3 | 領域外入力の実行時検査 | 領域外入力を定義済みにしたため不要 |

---

## 改版履歴

| 版 | 日付 | 内容 |
|---|---|---|
| 0.1 | 2026-10-09 | 初版ドラフト |
| 0.2 | 2026-10-10 | 数式方式のみに変更（テーブル、フラグ、`ILIBC_CTYPE_NO_TABLE`、テーブルと演算の一致試験を削除）。分類関数の戻り値を`bool`に変更。任意の`int`で挙動を定義（領域外は偽／不変）し、`ILIBC_DEBUG`による領域検査、定数式での領域外診断を削除。`std::`版に`constexpr`を付け、`ilibc::`の`constexpr`核、`char`型別版、型を削除するオーバーロードを廃止。`.cpp`なし、ヘッダのみに。試験に、領域外・極端値、ビット別名化、型の検査を追加 |
| 0.2（訂正） | 2026-10-10 | ヘッダ仕様書（ILIBC-HDR-001）との整合: `ilibc/config.hpp`への依存と、名前空間マクロの使用を記載 |
| 0.3 | 2026-10-10 | 名称をilibcに変更（`libc89`→`ilibc`、`LIBC89`→`ILIBC`、文書ID接頭辞）。過去の改版履歴中の名称も置き換えた。内容の変更なし |
| 0.3（訂正） | 2026-10-10 | 親文書の版の更新（v0.5）。内容の変更なし |
