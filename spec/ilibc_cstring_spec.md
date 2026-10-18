# `<cstring>` 関数仕様書

| 項目 | 内容 |
|---|---|
| 文書ID | ILIBC-STR-001 |
| ライブラリ名 | ilibc（旧仮称: libc89） |
| 版 | 0.3（ドラフト） |
| 親文書 | ILIBC-OVR-001 全体概要仕様書 v0.5 |
| 対象関数 | ST-01〜ST-18（18個。`std::`のシグネチャは23個） |
| 対象ヘッダ | `<cstring>`（正）、`<string.h>`（互換ラッパ） |
| 動作の基準 | ISO/IEC 9899:1990 §7.11。インタフェースはISO/IEC 14882:2024（C++23）からの意図的な逸脱を含む（第8章） |
| 実装方針 | **ナイーブな基準実装**（第3章）。効率は目的としない |

---

## 1. 範囲と参照文書

- 本書は、`<cstring>`の18関数の宣言、動作、基準実装、実装規則、試験観点を定める。
- 共通の設計規則は、全体概要仕様書の第8章、第9章に従う。
- 実装は、見通しを優先した**基準実装（リファレンス実装）**とする。ワード単位のコピー等の最適化は、初版では行わない（概要仕様書 D-05）。

| 参照文書 | 参照箇所 |
|---|---|
| ILIBC-OVR-001 | 第5.2節（関数一覧）、第5.6節（オーバーロード）、第8章、第9章、D-05、D-15 |
| ILIBC-CTP-001 | 試験方式の流用元（「正本」と、実装と独立した参照の考え方） |
| ILIBC-TST-001 | 試験の共通方式 |

---

## 2. 提供物

### 2.1 標準名（`std::`）

`<cstring>`は、`namespace std`に次の関数を宣言する（`size_t`は`std::size_t`）。全関数に`noexcept`を付ける。

- **ND**: `[[nodiscard]]`を付ける（○）。
- **CE**: `constexpr`にする。○は付ける（ヘッダ内の`constexpr inline`関数）。「暫定×」は、`void *`を取るため、C++23では定数式で扱えない（第6.2節、概要仕様書 D-15）。
- **†**: const正しいオーバーロードがある（第2.2節）。

| ID | 関数 | シグネチャ | ND | CE |
|---|---|---|---|---|
| ST-01 | `memcpy` | `void *memcpy(void *s1, const void *s2, size_t n);` | － | 暫定× |
| ST-02 | `memmove` | `void *memmove(void *s1, const void *s2, size_t n);` | － | 暫定× |
| ST-03 | `memcmp` | `int memcmp(const void *s1, const void *s2, size_t n);` | ○ | 暫定× |
| ST-04 | `memchr` † | （第2.2節） | ○ | 暫定× |
| ST-05 | `memset` | `void *memset(void *s, int c, size_t n);` | － | 暫定× |
| ST-06 | `strcpy` | `char *strcpy(char *s1, const char *s2);` | － | ○ |
| ST-07 | `strncpy` | `char *strncpy(char *s1, const char *s2, size_t n);` | － | ○ |
| ST-08 | `strcat` | `char *strcat(char *s1, const char *s2);` | － | ○ |
| ST-09 | `strncat` | `char *strncat(char *s1, const char *s2, size_t n);` | － | ○ |
| ST-10 | `strcmp` | `int strcmp(const char *s1, const char *s2);` | ○ | ○ |
| ST-11 | `strncmp` | `int strncmp(const char *s1, const char *s2, size_t n);` | ○ | ○ |
| ST-12 | `strchr` † | （第2.2節） | ○ | ○ |
| ST-13 | `strrchr` † | （第2.2節） | ○ | ○ |
| ST-14 | `strstr` † | （第2.2節） | ○ | ○ |
| ST-15 | `strspn` | `size_t strspn(const char *s1, const char *s2);` | ○ | ○ |
| ST-16 | `strcspn` | `size_t strcspn(const char *s1, const char *s2);` | ○ | ○ |
| ST-17 | `strpbrk` † | （第2.2節） | ○ | ○ |
| ST-18 | `strlen` | `size_t strlen(const char *s);` | ○ | ○ |

- 関数形式マクロとしては定義しない。
- `memcpy`等に`[[nodiscard]]`を付けないのは、戻り値が通常無視されるため（概要仕様書 第8.4節）。
- ILP32では、`size_t`は`unsigned int`。

### 2.2 const正しいオーバーロード（†の5関数）

C++の`<cstring>`は、次の2つの宣言を持つ。入力がconst領域のとき、戻り値もconstになる。

| 関数 | const版 | 非const版 | CE |
|---|---|---|---|
| `memchr` | `const void *memchr(const void *s, int c, size_t n) noexcept;` | `void *memchr(void *s, int c, size_t n) noexcept;` | 暫定× |
| `strchr` | `const char *strchr(const char *s, int c) noexcept;` | `char *strchr(char *s, int c) noexcept;` | ○ |
| `strrchr` | `const char *strrchr(const char *s, int c) noexcept;` | `char *strrchr(char *s, int c) noexcept;` | ○ |
| `strstr` | `const char *strstr(const char *s1, const char *s2) noexcept;` | `char *strstr(char *s1, const char *s2) noexcept;` | ○ |
| `strpbrk` | `const char *strpbrk(const char *s1, const char *s2) noexcept;` | `char *strpbrk(char *s1, const char *s2) noexcept;` | ○ |

### 2.3 互換ラッパ `<string.h>`

```cpp
#include <cstring>
using std::memcpy;  using std::memmove; using std::memcmp;  using std::memchr;  using std::memset;
using std::strcpy;  using std::strncpy; using std::strcat;  using std::strncat;
using std::strcmp;  using std::strncmp;
using std::strchr;  using std::strrchr; using std::strstr;
using std::strspn;  using std::strcspn; using std::strpbrk;
using std::strlen;
using std::size_t;
```

独自の宣言・定義を持たない。`NULL`は`<cstddef>`（コンパイラ付属）から得る。

### 2.4 内部名（`ilibc::detail`）

| 名前 | 内容 | 詳細 |
|---|---|---|
| `ilibc::detail::mem_copy_fwd`、`mem_copy_bwd`、`mem_compare`、`mem_find`、`mem_fill` | バイトポインタ版の`constexpr`核。`void *`を取る5関数の本体 | 第5.1節、第6章 |
| `ilibc::detail::memchr_base` | `memchr`の基本実体（asmラベルでCシンボル名`memchr`を持つ） | 第7.2節 |

公開の拡張名（`ilibc::strlen`等）は提供しない。文字列系の関数は、`std::`名がそのまま`constexpr`である。

### 2.5 設定

| 設定 | 既定 | 効果 |
|---|---|---|
| `ILIBC_DEBUG` | 未定義 | 定義すると、ヌルポインタと領域の重なり（`memcpy`）を`assert`で検査する（第4.7節） |

---

## 3. 基準実装方針（ナイーブな実装）

見通しを良くするため、次の規則に従う。

| # | 規則 |
|---|---|
| N1 | **1バイトずつ**処理する単純なループのみ。ワード単位処理、アライメント処理、ループ展開、ビット演算による検索、テーブルを使わない |
| N2 | コンパイラ組込み関数（`__builtin_*`）、インラインアセンブリ、ベクトル命令を使わない |
| N3 | 規格の記述に近い構造にして、読んで正しさを確認できるようにする。本体は短く（目安: 15行以内）、状態を持ち込まない |
| N4 | 計算量は素朴なものとする。たとえば、`strstr`、`strspn`、`strcspn`、`strpbrk`は O(\|s1\|×\|s2\|)。改良しない |
| N5 | **他の関数を呼ばない**。18関数は、すべて互いに独立する |
| N6 | 添字（`std::size_t`）によるアクセスを基本とする。`while (*d++ = *s++);`のようなポインタ演算の連鎖は使わない |
| N7 | 戻り値は規格どおり。比較関数は、符号なしバイト値の差を返す（第4.4節） |
| N8 | 実行速度・コードサイズの最適化は目的としない。この実装は、将来の最適化版の**正解基準**として残す（第7.4節） |
| N9 | 文字列系の13関数は、ヘッダ内の`constexpr inline`関数とする。`.cpp`ファイルは、`mem*`の5関数だけが持つ |

**コンパイラによる関数呼び出しへの変換**

- `mem*`の実装ファイル（`memcpy.cpp`等）のループが、同名関数の呼び出しに置き換えられると、無限再帰になる。次を守る。

| コンパイラ | 対象ファイルへの指定 |
|---|---|
| GCC | `-fno-tree-loop-distribute-patterns` |
| Clang | `-fno-builtin-memcpy -fno-builtin-memmove -fno-builtin-memset -fno-builtin-memcmp` |

- 文字列系の関数は、ヘッダ内にあるため、利用者の翻訳単位に展開される。最適化で、ループが`memset`／`memcpy`の呼び出しに変換されることがあるが、これらは常に提供される。それ以外の関数への変換（たとえばClangのループ認識による`strlen`呼び出し）は、利用者のビルドで`-ffreestanding -fno-builtin`を指定して防ぐ（概要仕様書 第2.3節）。成果物検査（第9章 TC-ST-13）で、未解決シンボルが残らないことを確認する。

---

## 4. 共通規則

### 4.1 引数と領域

| 規則 | 内容 |
|---|---|
| 有効なポインタ | すべてのポインタ引数は、`n`が0の呼び出しでも**有効な値**でなければならない（ヌルポインタは未定義動作）。ただし、`n`が0のとき、領域にはアクセスしない |
| 文字列 | 文字列引数は、終端の`'\0'`を含む配列を指さなければならない。終端がないと、配列外を読む（未定義動作） |
| 領域の重なり | `memcpy`、`strcpy`、`strncpy`、`strcat`、`strncat`で、コピー元とコピー先が重なる場合は未定義動作。`memmove`のみ、重なりを許す |
| 書き込み先の大きさ | 呼び出し側が保証する（ライブラリは検査しない）。`strcat`は`strlen(s1) + strlen(s2) + 1`、`strncat`は`strlen(s1) + min(n, strlen(s2)) + 1`、`strncpy`は`n`文字分が必要 |
| アドレス空間 | **平坦なアドレス空間**を前提とする（`memmove`が、アドレスを`std::uintptr_t`に変換して大小比較するため） |

### 4.2 文字の扱い

| 関数 | `int c`の変換 | 比較の型 |
|---|---|---|
| `memchr`、`memset` | `unsigned char`に変換 | `unsigned char` |
| `strchr`、`strrchr` | `char`に変換 | `char`（同じ変換後の値どうしの等価比較） |
| `memcmp`、`strcmp`、`strncmp` | （引数なし） | **`unsigned char`**として比較（`char`の符号に依存しない） |

- `strchr`、`strrchr`では、終端の`'\0'`も検索対象に含める（`c`が0のとき、終端を指すポインタを返す）。

### 4.3 戻り値

| 関数 | 戻り値 |
|---|---|
| `memcpy`、`memmove`、`memset`、`strcpy`、`strncpy`、`strcat`、`strncat` | 第1引数（コピー先／設定先） |
| `memchr`、`strchr`、`strrchr`、`strstr`、`strpbrk` | 見つかった位置へのポインタ。見つからなければ`nullptr` |
| `strspn`、`strcspn`、`strlen` | 長さ（`size_t`） |
| `memcmp`、`strcmp`、`strncmp` | 第4.4節 |

### 4.4 比較関数の戻り値

- 規格は、符号のみを規定する（負、0、正）。**利用者は、符号以外の値に依存してはならない**。
- 本実装は、最初に異なる位置の2バイトを`unsigned char`として、**その差**（−255〜255）を返す。この差は、基準実装の素朴な帰結であり、保証ではない。
- 試験は、符号のみを検査する（ホストlibcは、符号以外の値が異なる）。

### 4.5 `errno`・状態・再入・資源

| 項目 | 内容 |
|---|---|
| `errno` | 変更しない |
| 内部状態 | **なし**（グローバル変数、静的変数を持たない） |
| 再入可能性 | **全18関数が完全に再入可能**（割り込みハンドラから呼べる）。ただし、複数のコンテキストが同じ領域を同時に操作する場合の排他は、呼び出し側の責任 |
| スタック | 各関数とも、局所変数のみ（再帰なし、局所配列なし）。追加のスタックは数ワード以下 |
| ヒープ | 使わない |
| ROM定数データ／RAM | 0バイト／0バイト |

### 4.6 属性・例外・定数式

- `noexcept`、`[[nodiscard]]`、`constexpr`: 第2.1節のとおり。例外を投げない。
- **文字列系の13関数は`constexpr`**。定数式の評価中に未定義動作（範囲外アクセス、未終端の文字列、ヌルポインタの参照）があると、**コンパイルエラーとして診断される**（C++の定数式の性質）。

### 4.7 `ILIBC_DEBUG`

定義時、次を`assert`する。`constexpr`関数内の`assert`は、条件が真のとき定数式の評価に影響しない。

| 検査 | 対象 |
|---|---|
| ポインタ引数が`nullptr`でない | 全18関数のすべてのポインタ引数 |
| `n`が0より大きいとき、コピー元とコピー先の領域が重ならない | `memcpy` |

文字列の終端の有無、書き込み先の大きさは、検査できない。

---

## 5. 関数仕様

各関数に、動作、領域と未定義動作、基準実装を示す。基準実装は、第3章の規則に従う**そのまま使える実装**である。

### 5.1 メモリ関数（ST-01〜ST-05）

`void *`を取るため、C++23の定数式では`void *`からの型変換ができない。そこで、本体は`unsigned char *`を取る`ilibc::detail`の`constexpr`核とし、`std::`関数が`void *`から変換して呼ぶ（`std::`関数自体は非`constexpr`。D-15）。

**バイトポインタ核（共通。`ilibc::detail`）**

```cpp
namespace ilibc::detail {

constexpr void mem_copy_fwd(unsigned char *d, const unsigned char *s, std::size_t n) noexcept {
    for (std::size_t i = 0; i < n; ++i) { d[i] = s[i]; }              // 先頭から
}

constexpr void mem_copy_bwd(unsigned char *d, const unsigned char *s, std::size_t n) noexcept {
    while (n > 0) { --n; d[n] = s[n]; }                                // 末尾から
}

constexpr int mem_compare(const unsigned char *a, const unsigned char *b, std::size_t n) noexcept {
    for (std::size_t i = 0; i < n; ++i) {
        if (a[i] != b[i]) { return static_cast<int>(a[i]) - static_cast<int>(b[i]); }
    }
    return 0;
}

constexpr const unsigned char *mem_find(const unsigned char *s, unsigned char c, std::size_t n) noexcept {
    for (std::size_t i = 0; i < n; ++i) {
        if (s[i] == c) { return s + i; }
    }
    return nullptr;
}

constexpr void mem_fill(unsigned char *d, unsigned char c, std::size_t n) noexcept {
    for (std::size_t i = 0; i < n; ++i) { d[i] = c; }
}

}  // namespace ilibc::detail
```

#### ST-01 `memcpy`

| 項目 | 内容 |
|---|---|
| 動作 | `s2`が指す先頭`n`バイトを、`s1`が指す領域にコピーする |
| 戻り値 | `s1` |
| 未定義動作 | 領域が重なる。ヌルポインタ（`n`が0でも） |
| 基準実装 | `mem_copy_fwd`を呼ぶ（先頭から1バイトずつ）。重なり時の結果は不定（依存しない） |
| 試験の重点 | `n`=0、1、大きい値。重ならない任意のオフセット。書き込み範囲の前後（カナリア） |

```cpp
void *memcpy(void *s1, const void *s2, std::size_t n) noexcept {
    ilibc::detail::mem_copy_fwd(static_cast<unsigned char *>(s1),
                                 static_cast<const unsigned char *>(s2), n);
    return s1;
}
```

#### ST-02 `memmove`

| 項目 | 内容 |
|---|---|
| 動作 | `memcpy`と同じ。ただし、領域が重なっていても、コピー元の内容が正しくコピー先に移る |
| 戻り値 | `s1` |
| 基準実装 | コピー先がコピー元以下のアドレスなら先頭から、そうでなければ末尾からコピーする（アドレスは`std::uintptr_t`にして比較） |
| 試験の重点 | 重なりの全パターン（コピー先がコピー元の前／後ろ、1バイトずれ、完全一致、重ならない） |

```cpp
void *memmove(void *s1, const void *s2, std::size_t n) noexcept {
    unsigned char *d = static_cast<unsigned char *>(s1);
    const unsigned char *s = static_cast<const unsigned char *>(s2);
    if (reinterpret_cast<std::uintptr_t>(d) <= reinterpret_cast<std::uintptr_t>(s)) {
        ilibc::detail::mem_copy_fwd(d, s, n);
    } else {
        ilibc::detail::mem_copy_bwd(d, s, n);
    }
    return s1;
}
```

先頭から／末尾からの2つの核は、アドレス比較を含まないため、コンパイル時に（同じ配列の部分範囲どうしで）試験できる。

#### ST-03 `memcmp`

| 項目 | 内容 |
|---|---|
| 動作 | 先頭`n`バイトを、`unsigned char`として先頭から比較する |
| 戻り値 | 最初に異なるバイトの差（第4.4節）。すべて等しいか`n`が0なら0 |
| 試験の重点 | 0x80以上のバイト（符号付き比較の誤りを検出）、最初／最後のバイトだけが異なる場合、`n`=0 |

```cpp
int memcmp(const void *s1, const void *s2, std::size_t n) noexcept {
    return ilibc::detail::mem_compare(static_cast<const unsigned char *>(s1),
                                       static_cast<const unsigned char *>(s2), n);
}
```

#### ST-04 `memchr`

| 項目 | 内容 |
|---|---|
| 動作 | 先頭`n`バイトのうち、最初に`(unsigned char)c`と等しいバイトを探す |
| 戻り値 | 見つかった位置のポインタ。なければ`nullptr`（`n`が0のときも`nullptr`） |
| 備考 | 見つかった時点で探索をやめる（以降のバイトは読まない） |
| 基準実装 | `std::`の2つのオーバーロード（第2.2節）は、同じ基本実体を呼ぶ`inline`ラッパ |
| 試験の重点 | 先頭／末尾／途中の一致、不一致、`c`が256以上や負（変換後の値で比較）、`n`の範囲外にある一致を見つけないこと |

```cpp
// 基本実体（ilibc::detail。リンク名は"memchr"）
void *memchr_base(const void *s, int c, std::size_t n) noexcept {
    const unsigned char *p = ilibc::detail::mem_find(static_cast<const unsigned char *>(s),
                                                      static_cast<unsigned char>(c), n);
    return const_cast<unsigned char *>(p);
}
```

#### ST-05 `memset`

| 項目 | 内容 |
|---|---|
| 動作 | `s`が指す先頭`n`バイトに、`(unsigned char)c`を設定する |
| 戻り値 | `s` |
| 試験の重点 | `c`が256以上、負、0xFF。`n`=0。書き込み範囲の前後（カナリア） |

```cpp
void *memset(void *s, int c, std::size_t n) noexcept {
    ilibc::detail::mem_fill(static_cast<unsigned char *>(s), static_cast<unsigned char>(c), n);
    return s;
}
```

### 5.2 コピー・連結（ST-06〜ST-09）

以下は、`namespace std`内の`constexpr`関数（`inline`は`constexpr`に含意される）。

#### ST-06 `strcpy`

| 項目 | 内容 |
|---|---|
| 動作 | `s2`が指す文字列（終端`'\0'`を含む）を、`s1`が指す配列にコピーする |
| 戻り値 | `s1` |
| 未定義動作 | 領域が重なる。`s1`の配列が小さい |

```cpp
constexpr char *strcpy(char *s1, const char *s2) noexcept {
    std::size_t i = 0;
    while ((s1[i] = s2[i]) != '\0') { ++i; }
    return s1;
}
```

#### ST-07 `strncpy`

| 項目 | 内容 |
|---|---|
| 動作 | `s2`から最大`n`文字を`s1`にコピーする。`s2`が`n`文字より短ければ、残りを`'\0'`で埋める |
| 戻り値 | `s1` |
| 注意 | `s2`の長さが`n`以上のとき、**`s1`は終端されない**。常に`n`バイト書き込む |
| 試験の重点 | 長さが`n`未満／ちょうど`n`／`n`超、`n`=0、残りの`'\0'`埋め |

```cpp
constexpr char *strncpy(char *s1, const char *s2, std::size_t n) noexcept {
    std::size_t i = 0;
    while (i < n && s2[i] != '\0') { s1[i] = s2[i]; ++i; }   // 文字をコピー
    while (i < n) { s1[i] = '\0'; ++i; }                     // 残りを'\0'で埋める
    return s1;
}
```

#### ST-08 `strcat`

| 項目 | 内容 |
|---|---|
| 動作 | `s2`の文字列（終端を含む）を、`s1`の文字列の末尾に連結する |
| 戻り値 | `s1` |
| 未定義動作 | 領域が重なる。`s1`の配列が小さい |

```cpp
constexpr char *strcat(char *s1, const char *s2) noexcept {
    std::size_t i = 0;
    while (s1[i] != '\0') { ++i; }                           // s1の終端を探す
    std::size_t j = 0;
    while ((s1[i + j] = s2[j]) != '\0') { ++j; }             // 終端を含めてコピー
    return s1;
}
```

#### ST-09 `strncat`

| 項目 | 内容 |
|---|---|
| 動作 | `s2`から最大`n`文字を、`s1`の文字列の末尾に連結し、**必ず`'\0'`で終端する** |
| 戻り値 | `s1` |
| 注意 | `n`は、`s1`の配列全体の大きさではなく、連結する文字数の上限。最大`n + 1`バイト書き込む |
| 試験の重点 | `s2`の長さが`n`未満／ちょうど`n`／`n`超、`n`=0（`s1`は変化せず終端のみ） |

```cpp
constexpr char *strncat(char *s1, const char *s2, std::size_t n) noexcept {
    std::size_t i = 0;
    while (s1[i] != '\0') { ++i; }
    std::size_t j = 0;
    while (j < n && s2[j] != '\0') { s1[i + j] = s2[j]; ++j; }
    s1[i + j] = '\0';
    return s1;
}
```

### 5.3 比較（ST-10、ST-11）

#### ST-10 `strcmp`

| 項目 | 内容 |
|---|---|
| 動作 | 2つの文字列を、`unsigned char`として先頭から比較する |
| 戻り値 | 第4.4節 |
| 試験の重点 | 空文字列、一方が他方の接頭辞、0x80以上の文字（例: `"\xff"`と`"a"`で、前者が大きい） |

```cpp
constexpr int strcmp(const char *s1, const char *s2) noexcept {
    std::size_t i = 0;
    while (s1[i] != '\0' && s1[i] == s2[i]) { ++i; }
    return static_cast<int>(static_cast<unsigned char>(s1[i]))
         - static_cast<int>(static_cast<unsigned char>(s2[i]));
}
```

#### ST-11 `strncmp`

| 項目 | 内容 |
|---|---|
| 動作 | 最大`n`文字を比較する。どちらかの`'\0'`に達したら、それ以降は比較しない |
| 戻り値 | 第4.4節。`n`が0なら0 |
| 試験の重点 | `n`が文字列長の前後、`'\0'`の後ろに異なる文字がある場合（比較されないこと）、`n`=0 |

```cpp
constexpr int strncmp(const char *s1, const char *s2, std::size_t n) noexcept {
    for (std::size_t i = 0; i < n; ++i) {
        const unsigned char a = static_cast<unsigned char>(s1[i]);
        const unsigned char b = static_cast<unsigned char>(s2[i]);
        if (a != b) { return static_cast<int>(a) - static_cast<int>(b); }
        if (a == '\0') { return 0; }
    }
    return 0;
}
```

### 5.4 検索（ST-12〜ST-17）

const／非constの2版を持つ関数（`strchr`、`strrchr`、`strstr`、`strpbrk`）は、**const版を本体**とし、非const版はconstを外して呼ぶ。両方とも`constexpr`。以降は、const版の本体のみを示す。非const版は次の形（`strchr`の例）。

```cpp
constexpr char *strchr(char *s, int c) noexcept {
    return const_cast<char *>(strchr(static_cast<const char *>(s), c));
}
```

#### ST-12 `strchr`

| 項目 | 内容 |
|---|---|
| 動作 | `s`の中で、最初に`(char)c`と等しい文字を探す。終端`'\0'`も対象 |
| 戻り値 | 見つかった位置のポインタ。なければ`nullptr` |
| 試験の重点 | `c`=0（終端を指す）、先頭／末尾の一致、空文字列、`c`が256以上や負 |

```cpp
constexpr const char *strchr(const char *s, int c) noexcept {
    const char ch = static_cast<char>(c);
    for (;; ++s) {
        if (*s == ch) { return s; }            // 一致の判定を、終端の判定より先に行う
        if (*s == '\0') { return nullptr; }
    }
}
```

#### ST-13 `strrchr`

| 項目 | 内容 |
|---|---|
| 動作 | `s`の中で、**最後に**`(char)c`と等しい文字を探す。終端`'\0'`も対象 |
| 戻り値 | 見つかった位置のポインタ。なければ`nullptr` |
| 試験の重点 | `c`=0（終端を指す）、複数一致（最後を返す）、一致が1つ、空文字列 |

```cpp
constexpr const char *strrchr(const char *s, int c) noexcept {
    const char ch = static_cast<char>(c);
    const char *last = nullptr;
    for (;; ++s) {
        if (*s == ch) { last = s; }
        if (*s == '\0') { return last; }
    }
}
```

#### ST-14 `strstr`

| 項目 | 内容 |
|---|---|
| 動作 | `s1`の中で、文字列`s2`（終端を除く）と一致する最初の位置を探す |
| 戻り値 | 見つかった位置のポインタ。なければ`nullptr`。**`s2`が空文字列なら`s1`** |
| 試験の重点 | 空の`s2`、`s2`が`s1`より長い、先頭／末尾／途中の一致、部分一致の後に失敗して再開する場合（例: `s1`=`"aaab"`、`s2`=`"aab"`） |

```cpp
constexpr const char *strstr(const char *s1, const char *s2) noexcept {
    if (*s2 == '\0') { return s1; }
    for (; *s1 != '\0'; ++s1) {
        std::size_t i = 0;
        while (s2[i] != '\0' && s1[i] == s2[i]) { ++i; }
        if (s2[i] == '\0') { return s1; }       // s2を最後まで照合できた
    }
    return nullptr;
}
```

`s1[i]`が`'\0'`に達すると、`s2[i]`が`'\0'`でない限り、`s1[i] != s2[i]`となって照合が止まるため、`s1`の終端を越えて読むことはない。

#### ST-15 `strspn`

| 項目 | 内容 |
|---|---|
| 動作 | `s1`の先頭から、**すべて`s2`に含まれる文字**が続く長さを返す |
| 戻り値 | 長さ。`s2`が空なら0 |
| 試験の重点 | 空の`s1`／`s2`、すべて一致、先頭が不一致、`s2`に重複文字がある場合 |

```cpp
constexpr std::size_t strspn(const char *s1, const char *s2) noexcept {
    std::size_t n = 0;
    while (s1[n] != '\0') {
        const char *p = s2;
        while (*p != '\0' && *p != s1[n]) { ++p; }     // s1[n]がs2にあるか
        if (*p == '\0') { break; }                      // なければ終了
        ++n;
    }
    return n;
}
```

#### ST-16 `strcspn`

| 項目 | 内容 |
|---|---|
| 動作 | `s1`の先頭から、**`s2`に含まれない文字**が続く長さを返す |
| 戻り値 | 長さ。`s2`が空なら`strlen(s1)` |
| 試験の重点 | 空の`s1`／`s2`、先頭が一致、まったく一致しない |

```cpp
constexpr std::size_t strcspn(const char *s1, const char *s2) noexcept {
    std::size_t n = 0;
    while (s1[n] != '\0') {
        const char *p = s2;
        while (*p != '\0' && *p != s1[n]) { ++p; }
        if (*p != '\0') { break; }                      // s2にあれば終了
        ++n;
    }
    return n;
}
```

#### ST-17 `strpbrk`

| 項目 | 内容 |
|---|---|
| 動作 | `s1`の中で、`s2`のいずれかの文字と等しい最初の文字を探す。終端`'\0'`は対象外 |
| 戻り値 | 見つかった位置のポインタ。なければ`nullptr` |
| 試験の重点 | 空の`s1`／`s2`、先頭／末尾の一致、`s2`の文字の順序に依存しないこと |

```cpp
constexpr const char *strpbrk(const char *s1, const char *s2) noexcept {
    for (; *s1 != '\0'; ++s1) {
        for (const char *p = s2; *p != '\0'; ++p) {
            if (*p == *s1) { return s1; }
        }
    }
    return nullptr;
}
```

### 5.5 `strlen`（ST-18）

| 項目 | 内容 |
|---|---|
| 動作 | 終端`'\0'`を除く文字数を返す |
| 試験の重点 | 空文字列、1文字、`'\0'`の手前に0x80以上の文字がある場合 |

```cpp
constexpr std::size_t strlen(const char *s) noexcept {
    std::size_t n = 0;
    while (s[n] != '\0') { ++n; }
    return n;
}
```

---

## 6. `mem*`の`constexpr`化

### 6.1 内部核

第5.1節の`ilibc::detail::mem_*`の5つは`constexpr`であり、`unsigned char`配列に対して、**コンパイル時に検証できる**。`memmove`は、先頭から／末尾からの2つの核を、それぞれ検証する。

### 6.2 `std::mem*`を`constexpr`にできない理由と、検討中の対応

- C++23では、定数式の中で、`void *`から他のポインタ型への変換ができない（C++26で解禁）。`std::memcpy(void *, const void *, size_t)`の本体は、純粋なC++23の範囲では`constexpr`にできない。
- 概要仕様書 第8.2節の方針（技術的に可能な限り`constexpr`）に従い、次の対応を検討する（概要仕様書 D-15）。

| 案 | 内容 | 課題 |
|---|---|---|
| (a) | 非`constexpr`のまま（暫定） | 定数式で使えない |
| (b) | `if consteval`で、コンパイラ組込み関数（`__builtin_memcpy`等）に委譲する | GCC／Clangでの定数評価の動作を検証する必要がある。規則N2（組込み関数の不使用）の例外になる（実行時の分岐は、従来のナイーブな実装のまま）。`constexpr`は`inline`を意味するため、コンパイラが暗黙に呼ぶ外部シンボルの確実な生成（`.cpp`での強制出力）が課題 |
| (c) | C++26の`void *`変換の解禁を待つ | 時期が未定 |

---

## 7. 実装規則

### 7.1 ファイル構成

```
include/
  cstring                 # 文字列系13関数のconstexpr定義、memchrのinlineラッパ、mem*の宣言
  string.h                # 互換ラッパ（第2.3節）
  ilibc/
    string_mem.hpp        # ilibc::detail::mem_*の5核、memchr_baseの宣言
src/string/
  memcpy.cpp   memmove.cpp   memcmp.cpp   memchr.cpp   memset.cpp
```

`.cpp`は、`mem*`の5ファイルのみ（1関数1ファイル）。各ファイルは、対応する核を呼ぶ数行のラッパ。

### 7.2 宣言とリンク名

概要仕様書 第8.1節に従う。`mem*`の5関数の実体のみ、Cのシンボル名（マングルなし）で提供する。`std::`の宣言に、アセンブラ名ラベルを付けて、リンク名を固定する。例（GCC/Clang）:

```cpp
namespace std {
void *memcpy(void *s1, const void *s2, size_t n) noexcept __asm__("memcpy");
void *memmove(void *s1, const void *s2, size_t n) noexcept __asm__("memmove");
[[nodiscard]] int memcmp(const void *s1, const void *s2, size_t n) noexcept __asm__("memcmp");
void *memset(void *s, int c, size_t n) noexcept __asm__("memset");
}
```

`memchr`は、const版オーバーロードを持つため、基本実体（`void *`を返す）を`ilibc::detail`に置き、asmラベルでCシンボル名を与える。`std::`の2つのオーバーロードは、これを呼ぶ`inline`ラッパとする。

```cpp
namespace ilibc::detail {
void *memchr_base(const void *s, int c, std::size_t n) noexcept __asm__("memchr");
}
namespace std {
[[nodiscard]] inline const void *memchr(const void *s, int c, size_t n) noexcept {
    return ilibc::detail::memchr_base(s, c, n);
}
[[nodiscard]] inline void *memchr(void *s, int c, size_t n) noexcept {
    return ilibc::detail::memchr_base(s, c, n);
}
}
```

### 7.3 リソース

| 項目 | 値 |
|---|---|
| ROM定数データ | 0バイト |
| RAM | 0バイト |
| スタック | 数ワード以下（全関数。再帰なし） |
| ヒープ | なし |
| コードサイズ | 規定しない（基準値を記録する。第9章 TC-ST-14） |

### 7.4 依存関係と将来の最適化

- リンク時の依存: **なし**。`ilibc.a`は`mem*`の5オブジェクトのみで、互いに独立。
- 基準実装（第5章）は、最適化版を導入した後も、**削除しない**。
- 最適化版は、`if consteval`で基準実装と分岐するか、ビルド設定で切り替える（D-S1）。コンパイル時評価、および差分試験の**比較基準**には、基準実装を使う。
- 最適化版は、第9章の全試験に合格しなければならない。

---

## 8. 規格との差異

| # | 内容 | 扱い |
|---|---|---|
| 1 | **文字列系13関数（`std::`）に`constexpr`を付ける** | 意図的な逸脱（概要仕様書 第8.2節、逸脱5） |
| 2 | 比較関数の戻り値は、バイト値の差（符号以外は保証しない） | 規格が許す範囲 |
| 3 | `strcoll`、`strxfrm`、`strerror`、`strtok`を提供しない | 概要仕様書 第7章 |
| 4 | `mem*`系は、暫定的に`constexpr`でない | 概要仕様書 D-15 |
| 5 | 領域外アクセス、重なり、未終端文字列などは未定義動作（規格どおり）。定数式評価中のみ診断される | 規格が許す範囲 |

---

## 9. 試験仕様（観点）

詳細な手順は、ILIBC-TST-001で定める。

| ID | 方式 | 観点 | 合否基準 |
|---|---|---|---|
| TC-ST-01 | コンパイル時（`static_assert`） | `std::`の文字列系13関数（const／非constの両オーバーロードを含む）を、関数ごとの「試験の重点」（第5章）の入力で検証する。期待値は、**実装と独立に手書きした値**とする | 全一致 |
| TC-ST-02 | コンパイル時 | `ilibc::detail`の`mem_*`核を、`unsigned char`配列で検証する。`mem_copy_fwd`と`mem_copy_bwd`は、重なる部分範囲（同じ配列内）でも検証する | 全一致 |
| TC-ST-03 | コンパイル時 | 0x80以上のバイトを含む入力で、比較関数が`unsigned char`として動作する（符号付き比較で誤る入力を含める） | 全一致 |
| TC-ST-04 | コンパイル失敗試験 | 定数式で、未定義動作が診断される: (a)終端のない配列に`strlen`、(b)小さい配列に`strcpy`、(c)`nullptr`に`strcmp`。いずれもコンパイルエラーになる | 全件エラー |
| TC-ST-05 | コンパイル時 | const／非constオーバーロードの戻り値型（`const char *`／`char *`、`const void *`／`void *`）が、入力のconst性に従う（`std::is_same_v`） | 全成立 |
| TC-ST-06 | ホスト差分 | `ILIBC_HOST_TEST`で別名前空間にした18関数を、ホストlibcと比較する。長さ0〜64、バイト値は0〜255（0と0x80以上を高頻度で含める）、乱数入力。**比較関数は符号のみ**、検索関数は**先頭からのオフセット**で比較する | 全一致 |
| TC-ST-07 | 書き込み範囲 | 書き込み先の前後に、カナリア（`0xA5`）を置き、書き込み後に壊れていない。`strncpy`は`n`バイト、`strncat`は最大`n+1`バイト、`strcat`は`strlen(s1)+strlen(s2)+1`バイトだけが書き込まれる | 失敗数0 |
| TC-ST-08 | アライメント | `memcpy`、`memmove`、`memset`、`memcmp`、`memchr`を、先頭アドレスのオフセット0〜7の全組み合わせで検証する | 全一致 |
| TC-ST-09 | `memmove`重なり | `n`=0〜32、コピー元とコピー先のオフセット差を−32〜+32の全値で、一時バッファ経由のコピーと比較する | 全一致 |
| TC-ST-10 | サニタイザ | ホスト差分試験を、ASan／UBSanで実行する（範囲外アクセス、未初期化読み出しがない） | 検出0 |
| TC-ST-11 | ターゲット実行 | TC-ST-07〜TC-ST-09を、QEMU等のフリースタンディング環境で実行する。引数を`volatile`経由にして、畳み込みを避ける | 失敗数0 |
| TC-ST-12 | 成果物検査（`ilibc.a`） | (a)定義するグローバルシンボルが、`memcpy`、`memmove`、`memcmp`、`memchr`、`memset`の5個だけ。(b)5個のオブジェクトに、**未解決シンボルがない**（`nm -u`が空。`ILIBC_DEBUG`時の`__ilibc_assert_fail`を除く）。かつ、`mem*`の関数本体に、自分自身や他の`mem*`への呼び出し命令がない（`nm -u`では自己再帰を検出できないため、逆アセンブルと再配置で検査する。ILIBC-TST-001 第5.5節）。(c)概要仕様書 第12.2節のC++ランタイム・静的初期化の検査 | すべて満足 |
| TC-ST-13 | 利用者側のリンク検査 | 18関数をすべて使うサンプルを、`-ffreestanding -fno-builtin -nostdlib -O0/-Os/-O2/-O3`でコンパイルし、`ilibc.a`なしでリンクしたときの未解決シンボルが、`mem*`の5個の部分集合に限られる。`ilibc.a`ありで、未解決シンボルが0 | 満足 |
| TC-ST-14 | 基準値の記録 | 5個の`mem*`オブジェクトのコードサイズ、各関数の最大スタック使用量（`-fstack-usage`）を、`-Os`と`-O2`で記録する。文字列系は、非インライン化したときのサイズを記録する | 記録（最適化版の比較基準） |
| TC-ST-15 | 最適化レベル | `-O0`、`-Os`、`-O2`、`-O3`でTC-ST-06〜TC-ST-11が合格する | 全レベルで合格 |
| TC-ST-16 | ヘッダ | (a)`<cstring>`のみを含む翻訳単位で、`std::strlen`が使え、グローバルの`::strlen`が見つからない（コンパイル失敗試験）。(b)`<string.h>`を含むと、`::strlen`と`std::strlen`の両方が使える | すべて満足 |
| TC-ST-17 | `ILIBC_DEBUG` | ヌルポインタ、`memcpy`の重なりで、`__ilibc_assert_fail`が呼ばれる | 呼ばれる |

---

## 10. 未決事項

| ID | 内容 | 候補 | 既定（暫定） |
|---|---|---|---|
| D-15 | `mem*`の`constexpr`化 | 概要仕様書 D-15と同一。第6.2節の案(a)〜(c) | (a)。検証結果が良ければ(b) |
| D-S1 | 最適化版の導入方式 | `if consteval`で分岐／ビルド設定で切り替え／別名前空間 | 未定（初版の後） |
| D-S3 | 比較関数の戻り値を、差ではなく{−1, 0, 1}に正規化するか | 差（素朴、既定）／{−1, 0, 1}（分岐が1つ増える） | 差 |
| D-S4 | `ILIBC_DEBUG`で、ヌルポインタ以外の検査（`strcpy`系の重なり）も行うか | ヌルポインタと`memcpy`の重なりのみ（既定）／全関数 | 既定 |
| D-S5 | `memmove`の大小比較を、平坦なアドレス空間の前提（`uintptr_t`）とすることの是非 | 平坦（既定）／`std::less<void *>`（全順序。フリースタンディングでの利用可否を確認） | 平坦 |

### 10.1 解決済み

| 旧ID | 内容 | 決定 |
|---|---|---|
| D-01 | `strtok`を含めるか | **削除**（隠れた状態、再入不可）。再入可能な代替は、拡張APIの将来課題（概要仕様書 D-13） |
| D-05 | ワード単位等の最適化 | 初版はバイト単位（確定） |
| D-S2 | 再入可能なトークン分割 | 概要仕様書 D-13に統合 |

---

## 改版履歴

| 版 | 日付 | 内容 |
|---|---|---|
| 0.1 | 2026-10-09 | 初版ドラフト |
| 0.2 | 2026-10-10 | `strtok`を削除（19→18関数、シグネチャ24→23。ID: `strlen`はST-19→ST-18）。内部状態をなしに（RAM 4→0バイト、全関数が完全に再入可能）。**文字列系13関数を、ヘッダ内の`constexpr`関数に一本化**（`ilibc::`の公開核と、`.cpp`を廃止。`.cpp`は`mem*`の5ファイルのみ）。規則N5の例外をなくし、全18関数が独立。`mem*`の`constexpr`化の検討（D-15）を追加。試験に、定数式での未定義動作の診断、利用者側のリンク検査、`ilibc.a`のシンボル検査を追加し、`strtok`関連を削除 |
| 0.3 | 2026-10-10 | 名称をilibcに変更（`libc89`→`ilibc`、`LIBC89`→`ILIBC`、文書ID接頭辞、`assert`失敗フック`__c89_assert_fail`→`__ilibc_assert_fail`）。過去の改版履歴中の名称も置き換えた。内容の変更なし |
| 0.3（訂正） | 2026-10-10 | TC-ST-12の検査方法を訂正（`nm -u`では自己再帰を検出できないため、逆アセンブルによる検査を追加） |
| 0.3（訂正2） | 2026-10-10 | 親文書の版の更新（v0.5）。内容の変更なし |
