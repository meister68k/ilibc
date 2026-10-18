# ヘッダ仕様書

| 項目 | 内容 |
|---|---|
| 文書ID | ILIBC-HDR-001 |
| ライブラリ名 | ilibc（旧仮称: libc89） |
| 版 | 0.5（ドラフト） |
| 親文書 | ILIBC-OVR-001 全体概要仕様書 v0.7 |
| 対象 | `<cassert>`、`<cerrno>`、互換ラッパ（`<xxx.h>`）、スタブ、設定ヘッダ、全ヘッダに共通の規則 |
| 対象外 | `<cctype>`、`<cstring>`、`<cstdlib>`の関数の仕様（ILIBC-CTP-001、ILIBC-STR-001、ILIBC-STL-001） |

---

## 1. 範囲と参照文書

- 本書は、ライブラリのヘッダファイル全体の構成、全ヘッダに共通の規則、関数を持たないヘッダ（`<cassert>`、`<cerrno>`）、互換ラッパ、スタブ、設定ヘッダ、`assert`失敗フックを定める。
- 関数を持つ3ヘッダの宣言内容は、各関数仕様書に従う。

| 参照文書 | 参照箇所 |
|---|---|
| ILIBC-OVR-001 | 第2.1節（前提）、第4章（ヘッダ一覧）、第6章（型・マクロ・変数）、第9.2節（`errno`）、第10章（移植層）、第12章（ビルドと検証）、D-11、D-14、D-16、D-17 |
| ILIBC-CTP-001、STR-001、STL-001 | 各ヘッダの関数 |
| ILIBC-TST-001 | 試験の共通方式 |

---

## 2. ヘッダ一覧とファイル構成

### 2.1 ファイル一覧（29ファイル）

| 区分 | ファイル | 数 | 内容 |
|---|---|---|---|
| 提供ヘッダ（C++） | `cctype`、`cstring`、`cstdlib`、`cassert`、`cerrno` | 5 | 関数・マクロの定義 |
| 互換ラッパ | `ctype.h`、`string.h`、`stdlib.h`、`assert.h`、`errno.h` | 5 | `<cxxx>`を取り込み、グローバルへ導入（第6章） |
| 条件付き提供（C++） | `cstdio` | 1 | バックエンドありで有効、それ以外`#error` |
| 公開拡張（C++） | `ilibc/format.hpp` | 1 | I/O非依存の書式化 |
| スタブ（C++） | `cmath`、`ctime`、`csignal`、`clocale`、`csetjmp` | 5 | `#error`（第7章） |
| スタブ（C） | `stdio.h`、`math.h`、`time.h`、`signal.h`、`locale.h`、`setjmp.h` | 6 | `#error`（第7章） |
| 内部・設定 | `ilibc/config.hpp` | 1 | 設定・前提検査・内部マクロ（第8章） |
| | `ilibc/assert_fail.hpp` | 1 | `assert`失敗フックの宣言（第4.3節） |
| | `ilibc/string_mem.hpp` | 1 | `mem*`のバイトポインタ核（ILIBC-STR-001） |
| | `ilibc/stdlib.hpp` | 1 | `ilibc::`のerr版と解析関数（ILIBC-STL-001） |
| | `ilibc/io_sink.hpp`、`ilibc/io_backend.hpp` | 2 | 共通シンクとI/O差替え |

- ファイル名は、拡張子なし（C++規格のヘッダ名）、`.h`（Cの互換名）、`.hpp`（内部）の3種類。
- 利用者が直接インクルードしてよいのは、提供ヘッダと互換ラッパのみ。`ilibc/`以下は原則内部ヘッダ。ただし`ilibc/config.hpp`（設定）と**`ilibc/format.hpp`（公開書式化）**は利用者が直接インクルードしてよい。

### 2.2 インクルード関係

矢印は「インクルードする」。標準ヘッダは、コンパイラ付属のもの。

| ヘッダ | インクルードするもの |
|---|---|
| `cctype` | `ilibc/config.hpp` |
| `cstring` | `ilibc/config.hpp`、`ilibc/string_mem.hpp`、`<cstddef>`、`<cstdint>` |
| `cstdlib` | `ilibc/config.hpp`、`ilibc/stdlib.hpp`、`<cstddef>` |
| `ilibc/stdlib.hpp` | `<cctype>`（`isspace`）、`<cerrno>`、`<climits>`、`<cstddef>` |
| `cerrno` | `ilibc/config.hpp` |
| `cassert` | `ilibc/assert_fail.hpp` |
| `ilibc/config.hpp` | `<climits>`、（`ILIBC_DEBUG`定義時）`ilibc/assert_fail.hpp` |
| 互換ラッパ | 対応する`<cxxx>`のみ |
| スタブ | なし（`ILIBC_STUB_PASSTHROUGH`定義時のみ、コンパイラ付属の同名ヘッダ） |

`<cstdlib>`は`<cctype>`を間接的に取り込むため、`<cstdlib>`をインクルードすると、`std::isalpha`等も見える。`std`名前空間に限った副作用であり、グローバル名前空間は変わらない（D-H4）。

### 2.3 コンパイラ付属ヘッダとの関係

| 区分 | 該当 | 扱い |
|---|---|---|
| 本ライブラリが**遮蔽する**名前（22個） | 提供ヘッダ5、互換ラッパ5、スタブ11、条件付き`<cstdio>` | ヘッダ検索パスの先頭に置き、コンパイラ付属の同名ヘッダより先に見つけさせる |
| コンパイラ付属のものを**そのまま使う**名前 | `cstddef`、`climits`、`cstdarg`、`cfloat`、`stddef.h`、`limits.h`、`stdarg.h`、`float.h`、`cstdint`ほか | 本ライブラリは遮蔽しない |

**検索パスの規則**

- 本ライブラリの`include/`は、**`-isystem`**で指定し、コンパイラ付属の標準ヘッダのパスより**前**に置く。`-I`は使わない（警告の抑止と、探索順序のため）。
- `-nostdinc++`は使わない（コンパイラ付属の`<type_traits>`等が必要なため）。

---

## 3. 共通規則

### 3.1 全ヘッダ共通の規則

| # | 規則 |
|---|---|
| H1 | **包含ガード**: `#pragma once`を使う。**例外**: `<cassert>`と`<assert.h>`は、インクルードのたびに`NDEBUG`の状態で`assert`を再定義する必要があるため、包含ガードを持たない（第4.1節）。ただし、そこから取り込む`ilibc/assert_fail.hpp`は`#pragma once`を持つ |
| H2 | **自己完結**: すべてのヘッダは、他のヘッダを先にインクルードしなくても、単独でコンパイルできる。必要なものは、自分でインクルードする |
| H3 | **順序非依存**: 複数のヘッダのインクルード順序で、結果が変わらない |
| H4 | **宣言する名前を限定する**: 仕様書で定めた名前以外を、`std`、グローバル、マクロの各名前空間に追加しない（`<cstdlib>`が`<cctype>`を取り込む副作用を除く。D-H4） |
| H5 | **マクロ**: 公開マクロは第3.3節の一覧のもののみ。内部マクロは、`ILIBC_`の接頭辞を付ける |
| H6 | **`extern "C"`**: `assert`失敗フック（第4.3節）にのみ使う。他の関数は、C++のリンケージ（`mem*`の5関数は、asmラベルでCのシンボル名を与える） |
| H7 | **文字コード**: ヘッダは、**ASCIIのみ**で書く（コメントも英語）。`#error`のメッセージも、ASCII |
| H8 | **警告**: `-pedantic -Wall -Wextra -Wconversion`で、警告を出さない。`-isystem`で指定するため、利用者の警告オプションの影響は受けにくいが、ライブラリ自身のビルドでは検査する |
| H9 | **設定マクロの一貫性**: 設定マクロ（第8.4節）は、コマンドラインの`-D`で、**すべての翻訳単位に同じ値**を与える。翻訳単位ごとに値が違うと、ヘッダ内の`inline`関数の定義が食い違い、ODR違反になる。設定ヘッダを書き換えてはならない |

### 3.2 言語水準とコンパイラ

- 本ライブラリのヘッダが**実際に必要とする言語機能は、C++20まで**（`constinit`、`constexpr`の拡張、2の補数の保証等）。`if consteval`等のC++23の機能は、現時点で使っていない（最適化版の導入時に使う予定。概要仕様書 D-05）。
- そのため、ヘッダは、`__cplusplus >= 202002L`を要求する。ビルドは、概要仕様書 第2.3節のとおり`-std=c++23`で行う（試験コードはC++23を使う）。
- コンパイラは、GCC／Clang系を要求する（`__GNUC__`の定義。概要仕様書 D-14）。

### 3.3 マクロ一覧

**公開マクロ（利用者が使うもの）**

| マクロ | 定義するヘッダ | 備考 |
|---|---|---|
| `assert` | `<cassert>`、`<assert.h>` | 第4章。`ILIBC_HOST_TEST`では、これらのヘッダを使わない |
| `errno`、`EDOM`、`ERANGE`、`EINVAL` | `<cerrno>`、`<errno.h>` | 第5章。`ILIBC_HOST_TEST`では定義しない（ホストlibcのものと衝突するため。ライブラリ内部は`ILIBC_ERRNO`等を使う） |
| `EXIT_SUCCESS`、`EXIT_FAILURE`、`MB_CUR_MAX` | `<cstdlib>`、`<stdlib.h>` | ILIBC-STL-001。`ILIBC_HOST_TEST`では定義しない（同上） |

**設定マクロ（利用者が`-D`で指定するもの）**: `ILIBC_DEBUG`、`ILIBC_NONNULL`、`ILIBC_HOST_TEST`、`ILIBC_STUB_PASSTHROUGH`（第8.4節）。

**識別マクロ**: `ILIBC_VERSION_MAJOR`、`ILIBC_VERSION_MINOR`、`ILIBC_VERSION_PATCH`、`ILIBC_VERSION`（第8.3節。D-H3）。

**内部マクロ（利用者は使わない）**: `ILIBC_NS`、`ILIBC_NS_BEGIN`、`ILIBC_NS_END`、`ILIBC_ASM_LABEL`、`ILIBC_ASSERT`、`ILIBC_ERRNO`、`ILIBC_EDOM`、`ILIBC_ERANGE`、`ILIBC_EINVAL`。

これら以外のマクロを、ヘッダが定義してはならない（第10章 TC-HD-08）。

### 3.4 実装における名前空間の記述

関数仕様書のコードブロックは、読みやすさのため`namespace std { ... }`と`std::`を直接書いている。**実装では、次の規則に従う**（ホスト差分試験のため。概要仕様書 第10.2節）。

| 仕様書の記述 | 実装での記述 |
|---|---|
| `namespace std {` … `}`（本ライブラリの関数を宣言・定義する範囲） | `ILIBC_NS_BEGIN` … `ILIBC_NS_END` |
| 本ライブラリの関数への修飾（`std::isspace`等） | `ILIBC_NS::isspace` |
| コンパイラ付属の型（`std::size_t`、`std::uintptr_t`） | `std::size_t`のまま（本ライブラリの関数ではないため） |
| 関数の宣言の`__asm__("memcpy")` | `ILIBC_ASM_LABEL("memcpy")` |
| ライブラリ内部での`errno`、`ERANGE`、`EDOM`、`EINVAL`の使用 | `ILIBC_ERRNO`、`ILIBC_ERANGE`、`ILIBC_EDOM`、`ILIBC_EINVAL`（`ILIBC_HOST_TEST`でホストlibcと衝突しないため） |

`ILIBC_HOST_TEST`未定義のとき、`ILIBC_NS`は`std`、`ILIBC_ASM_LABEL(x)`は`__asm__(x)`。定義時は、`ILIBC_NS`が`ilibc_under_test`、`ILIBC_ASM_LABEL(x)`は空（ホストlibcとの衝突を避ける）。

---

## 4. `<cassert>`

### 4.1 仕様

| 項目 | 内容 |
|---|---|
| 定義するもの | マクロ`assert`のみ（`std`名前空間の名前はない） |
| 包含ガード | **持たない**（H1）。インクルードのたびに、`NDEBUG`の状態に応じて再定義する |
| `NDEBUG`未定義 | `assert(expr)`は、`expr`を評価し、偽ならフック（第4.3節）を呼ぶ。式の型は`void` |
| `NDEBUG`定義 | `assert(expr)`は、`expr`を**評価しない**。`static_cast<void>(0)`に展開される |
| 引数 | 可変長マクロ。式の中にコンマがあってもよい（`assert(std::is_same_v<A, B>)`等）。空の引数は不可 |
| 評価 | 式は、`static_cast<bool>`による文脈的変換で評価する |
| 定数式 | 式が真のとき、定数式の中で使える。**偽のとき、フックの呼び出し（非`constexpr`）により、定数式でなくなり、コンパイルエラーになる** |

### 4.2 定義

```cpp
// <cassert>  (no include guard: assert is redefined on every inclusion)
#include <ilibc/assert_fail.hpp>

#undef assert
#ifdef NDEBUG
#  define assert(...) (static_cast<void>(0))
#else
#  define assert(...)                                                           \
     (static_cast<bool>(__VA_ARGS__) ? static_cast<void>(0)                     \
                                     : ::__ilibc_assert_fail(#__VA_ARGS__, __FILE__, __LINE__))
#endif
```

- 引数が`#__VA_ARGS__`で文字列化され、式のテキストがフックに渡される。
- ROMの消費: `assert`1個ごとに、式の文字列と`__FILE__`の文字列が、定数データとして残る。製品ビルドでは`NDEBUG`を定義する。`-ffile-prefix-map`でパスを短くすることを推奨する。

### 4.3 失敗フック `__ilibc_assert_fail`

`ilibc/assert_fail.hpp`が、次の宣言を持つ（包含ガードあり）。

```cpp
// ilibc/assert_fail.hpp
#pragma once
extern "C" [[noreturn]] void __ilibc_assert_fail(const char *expr, const char *file, int line) noexcept;
```

| 項目 | 内容 |
|---|---|
| 提供者 | **利用者**。自分のプログラムで定義する。定義しない場合は、既定の弱シンボル実装（第9章）が使われる |
| 引数 | `expr`: 式の文字列、`file`: `__FILE__`、`line`: `__LINE__` |
| 戻り | **戻ってはならない**（無限ループ、リセット、デバッガ停止等） |
| 実行コンテキスト | 割り込み内、任意のタスクから呼ばれうる。ライブラリの機能（`assert`を含む）に依存しないこと（再帰を避ける） |
| リンケージ | `extern "C"`（Cのシンボル名`__ilibc_assert_fail`。アセンブリ等から上書きできる） |

### 4.4 `ILIBC_ASSERT`（ライブラリ内部のアサーション）

ライブラリの関数が、`ILIBC_DEBUG`ビルドで引数を検査するために使う内部マクロ。`NDEBUG`に**依存しない**（利用者の`NDEBUG`の設定と無関係に、`ILIBC_DEBUG`だけで有効になる）。

```cpp
// ilibc/config.hpp
#if defined(ILIBC_DEBUG)
#  include <ilibc/assert_fail.hpp>
#  define ILIBC_ASSERT(...)                                                    \
     (static_cast<bool>(__VA_ARGS__) ? static_cast<void>(0)                     \
                                     : ::__ilibc_assert_fail(#__VA_ARGS__, __FILE__, __LINE__))
#else
#  define ILIBC_ASSERT(...) (static_cast<void>(0))
#endif
```

### 4.5 互換ラッパ `<assert.h>`

`#include <cassert>`のみ。**包含ガードを持たない**（H1）。`using`宣言はない（`assert`はマクロのため）。

---

## 5. `<cerrno>`

### 5.1 仕様

| 項目 | 内容 |
|---|---|
| 定義するもの | 公開マクロ`errno`、`EDOM`、`ERANGE`、`EINVAL`。内部マクロ`ILIBC_ERRNO`、`ILIBC_EDOM`、`ILIBC_ERANGE`、`ILIBC_EINVAL`。変数`ilibc::detail::g_errno` |
| `EDOM`、`ERANGE`、`EINVAL` | 正の整数定数式で、互いに異なる値。`#if`で使える。値は33、34、22（D-H2） |
| `errno` | `int`型の、変更可能な左辺値に展開されるマクロ。単一のグローバル変数（`g_errno`）を指す |
| 変数の性質 | `inline constinit int`。**静的初期化**（値0）のみ。動的初期化、ガード変数、静的コンストラクタを生まない。0初期化されるため、`.bss`に置かれる |
| リンケージ | C++のリンケージ（名前修飾される）。Cのシンボル名は提供しない（C言語からの利用は対象外。概要仕様書 第4.4節） |
| スレッド | なし。単一のコンテキスト前提。割り込みと主処理で共有される（概要仕様書 第9.4節） |
| 初期値 | 0 |
| ライブラリが書く関数 | `std::strtol`、`std::strtoul`、`std::atoi`、`std::atol`のみ（範囲外のとき`ERANGE`）。`ilibc::`のerr版は書かない |
| `std`名前空間 | `errno`は`std`名前空間に属さない（マクロ） |

### 5.2 定義

```cpp
// <cerrno>
#pragma once
#include <ilibc/config.hpp>

namespace ilibc::detail {
inline constinit int g_errno = 0;
}

// internal names: always defined, used inside the library
#define ILIBC_EDOM   33
#define ILIBC_ERANGE 34
#define ILIBC_EINVAL 22
#define ILIBC_ERRNO  (::ilibc::detail::g_errno)

static_assert(ILIBC_EDOM > 0 && ILIBC_ERANGE > 0 && ILIBC_EINVAL > 0);
static_assert(ILIBC_EDOM != ILIBC_ERANGE && ILIBC_EDOM != ILIBC_EINVAL && ILIBC_ERANGE != ILIBC_EINVAL);

// public names: not defined in host-test mode (they would collide with the host libc)
#if !defined(ILIBC_HOST_TEST)
#  ifdef errno
#    error "ilibc: errno is already defined by another header."
#  endif
#  define EDOM   ILIBC_EDOM
#  define ERANGE ILIBC_ERANGE
#  define EINVAL ILIBC_EINVAL
#  define errno  ILIBC_ERRNO
#endif
```

- 他のヘッダが先に`errno`を定義していた場合は、`#error`で止める。黙って上書きすると、他のヘッダのコードと、`errno`の実体が食い違うため。
- `ILIBC_HOST_TEST`のとき、公開マクロ（`errno`、`EDOM`、`ERANGE`、`EINVAL`）は定義しない。ホスト差分試験では、ホストのlibcの`<cerrno>`が同時に使われるため。ライブラリの関数は、常に内部マクロ（`ILIBC_ERRNO`等）を使うので、動作は変わらない。試験コードも、`ILIBC_ERRNO`、`ILIBC_ERANGE`を使う（ILIBC-TST-001）。
- `errno`という名前の変数、メンバ、関数を、利用者が宣言すると、マクロにより壊れる（規格どおり）。

### 5.3 互換ラッパ `<errno.h>`

`#include <cerrno>`のみ。`using`宣言はない（マクロと変数のみのため）。

---

## 6. 互換ラッパ

### 6.1 規則

| # | 規則 |
|---|---|
| W1 | `#include <cxxx>`の後に、`using std::名前;`を並べるだけ。独自の宣言・定義、マクロ定義を持たない |
| W2 | 包含ガードは`#pragma once`（H1）。**例外**: `<assert.h>`は持たない |
| W3 | オーバーロードを持つ名前（`abs`、`memchr`等）は、`using std::名前;`で、すべてのオーバーロードが導入される |
| W4 | `<cxxx>`（C++のヘッダ）は、グローバル名前空間に**何も追加しない**。`<xxx.h>`のみが追加する |
| W5 | `ILIBC_HOST_TEST`のとき、ラッパは使わない（`std::`の名前を導入する前提のため） |
| W6 | 拡張API（`ilibc::`の名前。`_itoa_s`、err版等）は、ラッパでグローバルに**導入しない** |

### 6.2 各ラッパが導入する名前

| ラッパ | グローバルに導入する名前 |
|---|---|
| `<ctype.h>` | `isalnum`、`isalpha`、`iscntrl`、`isdigit`、`isgraph`、`islower`、`isprint`、`ispunct`、`isspace`、`isupper`、`isxdigit`、`tolower`、`toupper`（13個） |
| `<string.h>` | `memcpy`、`memmove`、`memcmp`、`memchr`、`memset`、`strcpy`、`strncpy`、`strcat`、`strncat`、`strcmp`、`strncmp`、`strchr`、`strrchr`、`strstr`、`strspn`、`strcspn`、`strpbrk`、`strlen`、`size_t`（19個） |
| `<stdlib.h>` | `abs`、`labs`、`atoi`、`atol`、`strtol`、`strtoul`、`size_t`（7個） |
| `<assert.h>` | なし（マクロ`assert`のみ） |
| `<errno.h>` | なし（マクロのみ） |

具体的な`using`宣言の並びは、ILIBC-CTP-001（第2.2節）、ILIBC-STR-001（第2.3節）、ILIBC-STL-001（第2.2節）に従う。

---

## 7. スタブ

### 7.1 対象

提供しないヘッダ11個（C++形式5、C形式6）を、`#error`を持つスタブとする（概要仕様書 第4.3節、第4.5節）。

| C++ヘッダ | C形式 | メッセージ中の理由 |
|---|---|---|
| （C++の`cstdio`は条件付き公開） | `stdio.h` | `no C stdio ABI` |
| `cmath` | `math.h` | `no floating point` |
| `ctime` | `time.h` | `no clock` |
| `csignal` | `signal.h` | `no OS` |
| `clocale` | `locale.h` | `only the C locale` |
| `csetjmp` | `setjmp.h` | `architecture dependent; not provided` |

### 7.2 雛形

11ファイルは、次の雛形から機械的に生成してよい。内容は、雛形と一致すること。

```cpp
// <stdio.h> (stub)
#pragma once
#if defined(ILIBC_STUB_PASSTHROUGH)
#  include_next <stdio.h>
#else
#  error "ilibc: <stdio.h> is not provided"
#endif
```

C形式のスタブ（`stdio.h`等）は、**自分自身の名前**で、同じ雛形を使う（`#include_next <stdio.h>`、メッセージは`<stdio.h>`）。C形式のスタブが、C++形式のスタブをインクルードしてはならない（エラーメッセージが、利用者が書いていないヘッダを指して混乱するため）。

### 7.3 規則

| # | 規則 |
|---|---|
| S1 | 既定では、インクルードした時点で、コンパイルエラーになる。メッセージに、`ilibc:`、ヘッダ名、`is not provided`、理由を含める |
| S2 | スタブは、宣言も、マクロも、定義しない |
| S3 | `ILIBC_STUB_PASSTHROUGH`が定義されているとき、**全スタブが一斉に**素通りする（スタブごとに個別には指定できない）。コンパイラ付属の同名ヘッダが、`#include_next`により、本ライブラリの`include/`の次の検索パスから探される |
| S4 | `ILIBC_STUB_PASSTHROUGH`は、統合試験での原因の切り分け用であり、通常のビルドでは定義しない（D-16） |
| S5 | `#include_next`はGCC／Clangの拡張。`-isystem`で指定した場所のヘッダでは、警告が抑止される。スタブが`#error`で止まるとき、この行は読み飛ばされる |
| S6 | `<cfloat>`、`<float.h>`、および C99 以降で追加された`<cwchar>`等は、**スタブにしない**（概要仕様書 第4.2節、第4.3節） |

---

## 8. 設定ヘッダ `ilibc/config.hpp`

### 8.1 位置づけ

- 前提の検査、名前空間・属性・アサーションの内部マクロ、識別マクロを定義する。
- 仮称時代の概要仕様書での名称`libc89_config.hpp`は、本書で`ilibc/config.hpp`と定める。
- 利用者は、直接書き換えない（H9）。設定は、`-D`で与える。
- 関数を持つ3ヘッダ（`<cctype>`、`<cstring>`、`<cstdlib>`）と`<cerrno>`が、インクルードする。

### 8.2 内容（骨格）

```cpp
// ilibc/config.hpp
#pragma once

// (a) language level and compiler
#if __cplusplus < 202002L
#  error "ilibc: C++20 or later is required (C++23 recommended)."
#endif
#if !defined(__GNUC__)
#  error "ilibc: GCC or Clang is required (see ILIBC-OVR-001 D-14)."
#endif

// (b) platform assumptions (ILIBC-OVR-001 section 2.1)
#include <climits>
static_assert(CHAR_BIT == 8, "ilibc: 8-bit char is required");
static_assert(sizeof(int) == 4 && sizeof(long) == 4 && sizeof(void *) == 4,
              "ilibc: ILP32 is required");
static_assert(sizeof(int) == sizeof(long), "ilibc: int and long must have the same size");
static_assert('A' == 0x41 && 'a' == 0x61 && '0' == 0x30, "ilibc: ASCII is required");

// (c) namespace and asm label
#if defined(ILIBC_HOST_TEST)
#  define ILIBC_NS ilibc_under_test
#  define ILIBC_ASM_LABEL(name)
#else
#  define ILIBC_NS std
#  define ILIBC_ASM_LABEL(name) __asm__(name)
#endif
#define ILIBC_NS_BEGIN namespace ILIBC_NS {
#define ILIBC_NS_END   }

// (d) attribute: nullable by default, user may predefine it (e.g. as [[gnu::nonnull]])
#ifndef ILIBC_NONNULL
#  define ILIBC_NONNULL
#endif

// (e) internal assertion
//     (see ILIBC-HDR-001 section 4.4)

// (f) identification (see ILIBC-HDR-001 section 8.3)
#define ILIBC_VERSION_MAJOR 0
#define ILIBC_VERSION_MINOR 1
#define ILIBC_VERSION_PATCH 0
#define ILIBC_VERSION (ILIBC_VERSION_MAJOR * 10000 + ILIBC_VERSION_MINOR * 100 + ILIBC_VERSION_PATCH)
```

- `ILIBC_ASSERT`の定義は、第4.4節。
- 前提の検査（b）は、不成立のときビルドエラーになる（概要仕様書 第2.1節）。ILP32以外への対応は、範囲外。

### 8.3 識別マクロ

| マクロ | 値 |
|---|---|
| `ILIBC_VERSION_MAJOR`、`MINOR`、`PATCH` | 本ライブラリの版（現時点: 0、1、0） |
| `ILIBC_VERSION` | `MAJOR * 10000 + MINOR * 100 + PATCH`（比較用の整数） |

利用者が、版による分岐をするための最小限の識別子。仕様書の版（文書の版）とは、別の番号体系（D-H3）。

### 8.4 設定マクロ

利用者が`-D`で与える。**すべての翻訳単位で同じ値**にする（H9）。

| 設定 | 既定 | 内容 | 影響を受ける範囲 |
|---|---|---|---|
| `ILIBC_DEBUG` | 未定義 | 引数検査用`ILIBC_ASSERT`を有効にする（`<cctype>`は領域外入力が定義済みのため対象外） | `<cstring>`、`<cstdlib>`の関数の本体（ヘッダ内の`inline`関数）。`mem*`の`.cpp` |
| `ILIBC_NONNULL` | 空 | ポインタ引数を持つ関数の宣言の先頭に付ける属性。利用者が事前に定義した場合（例: `-DILIBC_NONNULL="[[gnu::nonnull]]"`）はそれを使う。付けると、`nullptr`の検査（`ILIBC_DEBUG`）がコンパイラに除去されることがある | `<cstring>`、`<cstdlib>`のポインタ引数の関数 |
| `ILIBC_HOST_TEST` | 未定義 | ホスト差分試験用（第3.4節）。`std`の代わりに`ilibc_under_test`に宣言・定義し、asmラベルを付けない。公開マクロ（`errno`、`EDOM`、`ERANGE`、`EINVAL`、`EXIT_SUCCESS`、`EXIT_FAILURE`、`MB_CUR_MAX`）は定義しない。`<cassert>`と互換ラッパは使わない | 全ヘッダ（互換ラッパを除く） |
| `ILIBC_STUB_PASSTHROUGH` | 未定義 | スタブを一斉に素通りさせる（第7.3節） | スタブ11個+条件付き`<cstdio>`（計12個） |

**組み合わせの規則**

- `ILIBC_HOST_TEST`と互換ラッパの併用は不可（W5）。
- `ILIBC_DEBUG`と`NDEBUG`は独立（第4.4節）。

---

## 9. 既定のフック実装（`ilibc_hooks.a`）

### 9.1 目的と配置

- `assert`失敗フックの**既定の実装（弱シンボル）**を、別のアーカイブ`ilibc_hooks.a`として提供する。
- **`ilibc.a`には入れない**。`ilibc.a`が定義するグローバルシンボルを、`mem*`の5個だけに保ち、成果物検査（概要仕様書 第12.2節）を単純にするため。
- 利用者が、フックを自分で定義した場合（強シンボル）は、そちらが使われる。`ilibc_hooks.a`は、リンクコマンドの**最後**に置く。

### 9.2 内容

```cpp
// src/hooks/assert_fail.cpp  ->  ilibc_hooks.a
#include <ilibc/assert_fail.hpp>

extern "C" [[noreturn]] __attribute__((weak))
void __ilibc_assert_fail(const char *, const char *, int) noexcept {
    for (;;) {
        asm volatile("");          // keep the loop observable; stop here for the debugger
    }
}
```

- 副作用のない空の無限ループは、最適化で除去されるおそれがあるため、`asm volatile`を入れる。
- 引数は使わない（出力手段がない）。デバッガで、スタック上の引数を見られるように、最適化後も呼び出し元のフレームが残る。

### 9.3 将来の拡張（D-11）

`abort`等を、コンパイラ付属の部品が参照する場合の既定実装（弱シンボル）は、概要仕様書 D-11の決定後に、`ilibc_hooks.a`へ追加する。

---

## 10. 試験仕様（観点）

詳細な手順は、ILIBC-TST-001で定める。

| ID | 方式 | 観点 | 合否基準 |
|---|---|---|---|
| TC-HD-01 | コンパイル | **自己完結**: 提供ヘッダ5、互換ラッパ5、スタブ11（`ILIBC_STUB_PASSTHROUGH`なしではエラーを期待）、条件付き`<cstdio>`（バックエンド未選択ではエラー、`ILIBC_IO_BACKEND_POSIX`指定では成功）、内部ヘッダ（`ilibc/io_backend.hpp`・`ilibc/io_sink.hpp`、バックエンド指定）を、それぞれ**単独**でコンパイルする（`-fsyntax-only`）。`-std=c++20`と`-std=c++23`の両方 | 全件成功（スタブと未選択の`<cstdio>`は、期待どおりエラー） |
| TC-HD-02 | コンパイル | **順序非依存・重複**: 提供ヘッダ5と互換ラッパ5の、全順列（または十分な数のランダム順列）で、重複インクルードも含めて、コンパイルが通り、結果が同じ（`-E -dM`で出力されるマクロ集合が一致） | 全件一致 |
| TC-HD-03 | コンパイル | **警告ゼロ**: 全ヘッダを、`-pedantic -Wall -Wextra -Wconversion -Werror`（`-isystem`ではなく`-I`で指定して、警告を出させる）でコンパイルする | 警告0 |
| TC-HD-04 | コンパイル失敗 | **言語水準**: `-std=c++17`で、設定ヘッダを含むヘッダがコンパイルエラーになる。`-std=c++20`で通る | 期待どおり |
| TC-HD-05 | コンパイル | **互換ラッパ**: 各ラッパをインクルードしたとき、第6.2節の名前が、グローバルと`std`の両方で使える。`<cxxx>`のみをインクルードしたとき、グローバルに現れない（コンパイル失敗試験）。`ilibc::`の拡張関数（`_itoa_s`等）が、ラッパをインクルードしても、グローバルと`std`に現れない（コンパイル失敗試験） | 全成立 |
| TC-HD-06 | コンパイル | **`<assert.h>`と`<cassert>`**: (a)`NDEBUG`未定義→定義→未定義の順に、同じ翻訳単位で再インクルードして、各時点の`assert`の定義が`NDEBUG`に従う。(b)`NDEBUG`定義時に、式が評価されない（副作用が起きない）。(c)コンマを含む式（`assert(std::is_same_v<int, int>)`）。(d)定数式で、真の`assert`が通り、偽の`assert`がコンパイルエラー | 全成立 |
| TC-HD-07 | 実行時 | **失敗フック**: 試験用のフックを定義して、(a)偽の`assert`で呼ばれる。(b)引数の`expr`が式のテキストと一致、`file`と`line`が正しい。(c)`NDEBUG`定義時には呼ばれない。(d)`ILIBC_DEBUG`の`ILIBC_ASSERT`が、`NDEBUG`の設定に関係なく働く | 全成立 |
| TC-HD-08 | プリプロセッサ | **マクロの漏れ**: 全ヘッダをインクルードした状態の`-E -dM`の出力から、コンパイラが定義するマクロを除いた集合が、第3.3節の一覧（公開、設定、識別、内部）と一致する | 一致 |
| TC-HD-09 | コンパイル／実行時 | **`<cerrno>`**: (a)`errno`が`int`の左辺値で、代入、`++`、アドレス取得ができる。(b)`EDOM`、`ERANGE`、`EINVAL`が`#if`で使え、正で、互いに異なる。(c)初期値が0。(d)2つの翻訳単位が`errno`を使ってリンクしたとき、実体が1つ（`nm`で、`g_errno`が弱シンボル1つに統合される）。(e)他のヘッダが先に`errno`を定義していると`#error` | 全成立 |
| TC-HD-10 | 成果物検査 | **静的初期化**: `<cerrno>`を使うプログラムに、静的コンストラクタ、`__cxa_guard_*`、`.init_array`がない（概要仕様書 第12.2節） | すべて満足 |
| TC-HD-11 | コンパイル失敗 | **スタブ**: スタブ11個と、条件付き`<cstdio>`（`ILIBC_STUB_PASSTHROUGH`なし・バックエンド未選択）が、コンパイルエラーになり、メッセージに、`ilibc:`、そのヘッダ名、`is not provided`、理由（スタブは第7.1節、`<cstdio>`はILIBC-IO-001第5.1節）を含む。C形式のスタブのメッセージが、C形式の名前を指す | 全件成立 |
| TC-HD-12 | コンパイル | **スタブの素通り**: `ILIBC_STUB_PASSTHROUGH`を定義すると、スタブ11個と`<cstdio>`の計12個が、コンパイラ付属の同名ヘッダを取り込む（ホスト環境のヘッダで確認する） | 全件成立 |
| TC-HD-13 | コンパイル | **遮蔽の確認**: `-isystem`の順序で、本ライブラリの`<cstring>`等が、コンパイラ付属のものより先に選ばれる（`#include_next`や`__FILE__`で確認）。コンパイラ付属のままの`<cstddef>`、`<climits>`、`<cstdarg>`、`<cfloat>`が、遮蔽されない | 全成立 |
| TC-HD-14 | コンパイル失敗 | **前提の検査**: ILP32でない環境、`CHAR_BIT`が8でない環境、非GCC系で、ビルドエラーになる（模擬: `-D`で`CHAR_BIT`相当を差し替えられる試験用の設定、またはクロスコンパイラ） | 期待どおり |
| TC-HD-15 | リンク | **設定の一貫性**: `ILIBC_DEBUG`の有無が異なる2つの翻訳単位をリンクしたとき、ODR違反を検出する手段（`-Wodr`、LTO）で、警告が出る。または、この組み合わせが禁止であることを、ビルドシステムが検査する | 検出または禁止 |
| TC-HD-16 | リンク | **フック**: (a)利用者がフックを定義 → 利用者の定義が使われる。(b)定義せず、`ilibc_hooks.a`をリンク → 既定の弱シンボルが使われる。(c)定義せず、`ilibc_hooks.a`もなし、かつ`assert`が非`NDEBUG`で使われる → 未解決シンボルのリンクエラー | 全成立 |
| TC-HD-17 | 成果物検査 | `ilibc.a`が、`__ilibc_assert_fail`を**定義しない**（`mem*`の5個のみ。概要仕様書 第12.2節） | 満足 |

---

## 11. 未決事項

| ID | 内容 | 候補 | 既定（暫定） |
|---|---|---|---|
| D-H1 | 包含ガード | `#pragma once`／従来の`#ifndef`ガード（`#pragma once`が使えない処理系のため） | `#pragma once`（GCC／Clang前提） |
| D-H2 | `EDOM`、`ERANGE`、`EINVAL`の値 | 33、34、22（Linuxの慣用値）／他 | 33、34、22 |
| D-H3 | 識別マクロ（`ILIBC_VERSION*`）を提供するか | 提供する／提供しない | 提供する（暫定） |
| D-H4 | `<cstdlib>`が`<cctype>`を間接的に取り込み、`std::isalpha`等が見える副作用 | 許容する／`isspace`の核を別の内部ヘッダに分離して防ぐ | 許容する |
| D-H5 | `assert`を可変長マクロにすること（C++26で規格化された形） | 可変長（既定）／1引数マクロ（C++23の規定どおり） | 可変長 |
| D-11 | `abort`等の既定の弱シンボルの追加先 | 概要仕様書 D-11の決定後、`ilibc_hooks.a`に追加する | 未定 |
| D-14 | 対象コンパイラ | 概要仕様書 D-14と同一（GCC／Clang系を要求するチェックを、設定ヘッダに入れた） | GCC／Clang系 |

---

## 改版履歴

| 版 | 日付 | 内容 |
|---|---|---|
| 0.1 | 2026-10-10 | 初版ドラフト |
| 0.2 | 2026-10-10 | 名称をilibcに変更（`libc89`→`ilibc`、`LIBC89`→`ILIBC`、文書ID接頭辞、`assert`失敗フック`__c89_assert_fail`→`__ilibc_assert_fail`）。過去の改版履歴中の名称も置き換えた。内容の変更なし |
| 0.2（訂正） | 2026-10-10 | 試験仕様書（ILIBC-TST-001）の作成に伴う整合: `ILIBC_HOST_TEST`でホストlibcと衝突する公開マクロ（`errno`、`EDOM`、`ERANGE`、`EXIT_*`、`MB_CUR_MAX`）を定義しないことを規定し、ライブラリ内部用に`ILIBC_ERRNO`、`ILIBC_EDOM`、`ILIBC_ERANGE`を追加 |
| 0.3 | 2026-10-10 | `_itoa_s`の追加に伴う変更: `EINVAL`（内部`ILIBC_EINVAL`、値22）を追加。互換ラッパが`ilibc::`の拡張API（`_itoa_s`等）をグローバルに導入しないことを規定（W6） |
| 0.31 | 2026-10-12 | ILIBC-IO-001 v0.4に伴う追加公開ヘッダと条件付きスタブの節（第12章）を追加。`<cstdio>`を条件付き公開ヘッダへ分類変更（旧第2章のスタブ分類と第7.2節のサンプルは旧仕様扱い） |
| 0.32 | 2026-10-12 | TC-HD-01・11・12をスタブ11個+条件付き`<cstdio>`（計12個）に更新。`ILIBC_STUB_PASSTHROUGH`の影響範囲表記を同じ分類に合わせて修正 |
| 0.5 | 2026-10-12 | 入力APIと`clear_errno`の追加を反映（第13章） |

---


## 12. 追加公開ヘッダと条件付きスタブ（ILIBC-IO-001 v0.4）

本節は第2章のヘッダ一覧と第7章の`<cstdio>`スタブ記述を更新する。`<cstdio>`は無条件スタブではなく、明示的なI/Oバックエンド選択時だけ有効な公開ヘッダである。`<stdio.h>`は常時`#error`スタブを維持する。

| ヘッダ | 区分 | インクルード関係 |
|---|---|---|
| `<ilibc/format.hpp>` | 公開・I/Oなし | `ilibc/config.hpp`とI/O非依存のフォーマット処理 |
| `<cstdio>` | 条件付き公開 | `ilibc/format.hpp`、`ilibc/io_backend.hpp` |
| `<ilibc/io_backend.hpp>` | 内部 | 未選択/未実装では`#error`、POSIXは`<unistd.h>` |
| `<ilibc/io_sink.hpp>` | 内部 | Count/Buffer/FDシンク。ただし書式化のみではFDを実体化しない |
| `<stdio.h>` | スタブ | 無条件`#error`（従来の診断用passthroughは別） |

旧ファイル一覧26件を基準に、公開`ilibc/format.hpp`、内部`io_backend.hpp`、`io_sink.hpp`の**3件を追加**して合計29件。既存の`<cstdio>`は分類を変更するだけなので件数は増えない。C++の`#error`スタブは5件、C形式のスタブは6件。

**新規設定マクロ:** `ILIBC_IO_BACKEND_POSIX`、`ILIBC_HOST_IO_TEST`（POSIXの別名）、`ILIBC_IO_BACKEND_CUSTOM`（将来予約・現時点`#error`）、`ILIBC_ENABLE_PRINTF_MACRO`。POSIXとCUSTOMの同時指定はエラー、バックエンド未指定の`<cstdio>`もエラー。設定マクロは初回インクルード前に確定。`ILIBC_HOST_TEST`とは独立。

`<cstdio>`の`printf`互換マクロは`ILIBC_ENABLE_PRINTF_MACRO`が定義された場合だけ生成する。`<ilibc/format.hpp>`には互換`printf`マクロを定義しない。I/Oなしの`<ilibc/format.hpp>`単独インクルード・リンクと、`<cstdio>`未選択時のコンパイル失敗を必須試験に加える。

個別API仕様の正本はILIBC-IO-001 v0.5。本節に反する第2章のスタブ分類と第7.2節の`<cstdio>`サンプルは旧仕様扱いとする。

---
## 13. 入力機能・`clear_errno`の追補（ILIBC-IO-001 v0.5）

- `<cstdio>`は既存の条件付き公開ヘッダのまま。`getchar`、`fgetc`、`fgets`、安全な`gets`、`fread`を`ilibc::`に追加。
- `<ilibc/io_backend.hpp>`に`detail::read_some(int,void*,unsigned int,int*) noexcept`を追加。POSIXではホスト`<unistd.h>`と`::read()`を使用する。
- `<stdio.h>`は従来どおり`#error`スタブ。`<ilibc/format.hpp>`はI/O非依存を維持。
- `<cerrno>`へ`ilibc::clear_errno()`と`ilibc::clear_errno(int&)`を追加。公開マクロ`errno`の実体や数値定義は変更しない。
- 入力エラーの既定報告先はilibc側`detail::g_errno`。`ILIBC_HOST_TEST`時にホストの`errno`へ誤って書かない。
- ヘッダの名前空間、リンク方式、包含ガードなどは従来の規則に従う。
