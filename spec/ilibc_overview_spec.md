# ilibc 全体概要仕様書（フリースタンディング C89 サブセット標準ライブラリ、C++23実装）

| 項目 | 内容 |
|---|---|
| 文書ID | ILIBC-OVR-001 |
| 名称 | **ilibc**（旧仮称: libc89） |
| 版 | 0.7（ドラフト） |
| 対象 | 32bitマイコン／OSなし／入出力なし／浮動小数点なし／ヒープなし |
| 機能の範囲 | ISO/IEC 9899:1990（C89／C90）標準ライブラリのサブセット |
| 実装言語・インタフェース規定 | ISO/IEC 14882:2024（C++23）。ヘッダは`<cctype>`等のC++形式を正とし、`<ctype.h>`等は互換ラッパとする |
| 位置づけ | 全体概要。個々の関数の仕様は別文書（第13章）に譲る |

---

## 1. 目的と範囲

### 1.1 目的

OS・ファイルシステム・入出力・浮動小数点ユニット・ヒープを持たない32bitマイコン向けに、C89標準ライブラリのうち実用上意味のある関数だけを抜き出し、**C++23で実装**した小型で決定的なライブラリを提供する。

### 1.2 範囲

- 本書は、ライブラリが提供するヘッダ、関数、マクロ、型、C++としての設計規則、共通動作規則、依存関係、ビルド構成を定める。
- 個々の関数の厳密な振る舞い（境界条件、戻り値、エラー時動作）は個別仕様書で定める。
- コンパイラ実行時ライブラリ、コンパイラ付属の標準ヘッダ、スタートアップコード、リンカスクリプト、ドライバ類は範囲外とする。
- C++20のモジュール（`import std;`）は範囲外とする（ヘッダのみ提供）。

### 1.3 設計方針

1. **小さく**: 規格にあっても、この環境で意味を持たないものは実装しない（第1.4節）。
2. **決定的に**: 動的メモリ、ロケール、時刻、環境などの外部状態に依存しない。
3. **状態を持たない**: 書き換え可能なグローバル状態は`errno`のみとする。
4. **`constexpr`を最優先**: 技術的に可能な限り、**規格を逸脱してでも**、すべての関数を`constexpr`にする（第8.2節）。
5. **C++の型システムで安全に**: 型検査で誤用を検出する（第8.3節）。
6. **規格の面を保つ**: 規格からの逸脱は、第3章に一覧化したものに限る。独自の拡張は別名前空間（`ilibc`）に分離する。
7. **アーキテクチャに依存しない**: アセンブリ、特定CPUの機能に依存しない。純粋なC++23で書く。
8. **検証可能に**: コンパイル時テスト（`static_assert`）、ホスト差分テスト、ターゲット実行の3段で検証する。

### 1.4 機能の選定基準

次のいずれかに該当する関数は、提供しない。

| 基準 | 該当する例 |
|---|---|
| 使用頻度が低い、または使用予定がない | `qsort`、`bsearch` |
| CPUアーキテクチャに強く依存する | `setjmp`、`longjmp` |
| 隠れた内部状態を持つ、または、より良い代替がある | `rand`、`srand`（より良い乱数生成器がある）、`strtok`（再入不可） |
| C++では冗長 | `div`、`ldiv`（コンパイラが`/`と`%`を1つの除算にまとめる） |

次の条件をすべて満たす**非標準の拡張関数**に限り、追加を認める。

| 基準 | 内容 |
|---|---|
| 欠落の補完 | ilibcが除外した標準機能（`<cstdio>`）の代替として、実用上必要である（例: 整数から文字列への変換） |
| 状態を持たない | 隠れた内部状態、グローバル変数（`errno`を除く）を持たない |
| 名前の隔離 | `ilibc::`に置き、`std::`にも、グローバルにも導入しない |
| 既存の方針に従う | `constexpr`、`noexcept`、`[[nodiscard]]`。エラーは、戻り値、または出力先の引数で返し、`errno`に頼らない |

### 1.5 名称と命名規則

正式名称は**ilibc**（旧仮称: libc89）。次の規則で、名称を使う。

| 対象 | 規則 | 例 |
|---|---|---|
| 名前空間 | 拡張機能は`ilibc::`、内部は`ilibc::detail`。標準の名前は`std::` | `ilibc::strtol`、`ilibc::detail::mem_copy_fwd` |
| マクロ | 接頭辞`ILIBC_`（設定、内部、識別マクロ） | `ILIBC_DEBUG`、`ILIBC_NS`、`ILIBC_VERSION` |
| 文書ID | `ILIBC-xxx-nnn` | `ILIBC-OVR-001` |
| 内部ヘッダのディレクトリ | `ilibc/` | `<ilibc/config.hpp>` |
| アーカイブ | `ilibc.a`、`ilibc_hooks.a` | |
| `#error`、`static_assert`のメッセージ | 接頭辞`ilibc:` | `ilibc: <cstdio> is not provided` |
| `assert`失敗フック | `__ilibc_assert_fail`（旧: `__c89_assert_fail`） | |

「C89」は、機能の範囲を示す規格（ISO/IEC 9899:1990）の名称であり、ライブラリ名ではない。

---

## 2. 前提条件

### 2.1 実行環境

| 項目 | 前提 |
|---|---|
| ワード幅 | 32bit |
| データモデル | ILP32（`int`=`long`=ポインタ=32bit）。`char`=8bit |
| 整数表現 | 2の補数（C++20以降は規格が保証） |
| 文字コード | ASCII（実行文字集合） |
| `char`の符号 | 処理系依存。どちらでも正しく動作すること |
| エンディアン | 依存しない |
| 浮動小数点 | 使用しない。ライブラリ内に`float`／`double`／`long double`を一切含めない |
| 整数除算 | `/`と`%`はハードウェア命令またはコンパイラ実行時ライブラリを使ってよい |
| ロケール | `"C"`ロケール固定 |
| OS／ヒープ／入出力 | なし |
| 例外／RTTI | 無効（`-fno-exceptions -fno-rtti`）。ライブラリは使わない |
| 実行モデル | 単一スレッド。割り込みからの呼び出し可否は各関数仕様に記載（第9.4節） |

これらの前提は、`ilibc/config.hpp`で**コンパイル時に検査**する（不成立ならビルドエラー）。

```cpp
static_assert(CHAR_BIT == 8);
static_assert(sizeof(int) == 4 && sizeof(long) == 4 && sizeof(void *) == 4);
static_assert(sizeof(int) == sizeof(long));                  // atoi／atolが、strtolの結果をそのまま返すため
static_assert('A' == 0x41 && 'a' == 0x61 && '0' == 0x30);   // ASCII
```

### 2.2 実装言語

- C++23（`-std=c++23`）で記述する。対応する最新に近い処理系（GCC／Clang系）を対象とする（D-14）。
- アセンブリは使わない。

| 使用する機能 | 使用しない機能 |
|---|---|
| `constexpr`／`consteval`／`constinit`、`if consteval` | 例外（`throw`／`try`） |
| テンプレート、`concepts`、`requires` | RTTI（`typeid`、`dynamic_cast`） |
| `static_assert`、`[[nodiscard]]`、`[[noreturn]]`、`noexcept` | 仮想関数（vtableを生まない） |
| `std::bit_cast`、`std::unreachable`、`std::is_constant_evaluated` | 動的メモリ（`operator new`／`delete`） |
| `inline`変数、`enum class`、名前空間 | 動的初期化を要するグローバル／静的ローカル変数（ガード変数を生む） |
| | `thread_local`、コルーチン、`std::function`、ストリーム、`<string>`等のホスト向け標準ライブラリ |

ライブラリ内部で参照する標準ヘッダは、コンパイラ付属のフリースタンディング対応ヘッダ（`<cstddef>`、`<cstdint>`、`<climits>`、`<limits>`、`<type_traits>`、`<concepts>`、`<bit>`、`<utility>`、`<version>`）に限る。使用する処理系での利用可否は、導入時に確認する。

利用者のコードが、上記以外の標準ヘッダを使うことは妨げない。ただし、その利用可否は、処理系のフリースタンディング構成に依存する。使う場合は、本ライブラリとの併用でビルドできることを、利用者が確認する（第10.3節）。

### 2.3 推奨コンパイルオプション（GCC/Clang系）

**ライブラリのビルドと、利用者のビルドの両方**に、次を指定する。ほとんどの関数がヘッダ内の`constexpr inline`関数であり、利用者の翻訳単位に展開されるため。

```
-std=c++23 -pedantic -Wall -Wextra -Wconversion
-ffreestanding -fno-builtin
-fno-exceptions -fno-rtti -fno-threadsafe-statics
（リンク時）-nostdlib
```

`mem*`の実装ファイル（`src/string/mem*.cpp`）には、さらに次を指定する（ループが同名関数の呼び出しに変換されて、自己再帰になることを防ぐ）。

| コンパイラ | 指定 |
|---|---|
| GCC | `-fno-tree-loop-distribute-patterns` |
| Clang | `-fno-builtin-memcpy -fno-builtin-memmove -fno-builtin-memset -fno-builtin-memcmp` |

---

## 3. 規格適合性

- **機能**: 提供する関数・マクロ・型の動作は、C89の規定と同一とする。
- **インタフェース**: ヘッダ名、名前空間、オーバーロード、`<xxx.h>`ヘッダの意味は、C++23規格（ISO/IEC 14882:2024）に従う。
- **フリースタンディングとの関係**: C++23規格では、フリースタンディング処理系が提供するヘッダ集合は処理系定義であり、必須とされるCライブラリ系ヘッダは`<cstdlib>`（部分）などに限られる。`<cstring>`と`<cerrno>`は、C++26で部分的に必須化される方向にある。したがって、本ライブラリが`<cctype>`等を追加で提供することは、規格の枠内である。

本ライブラリは、次の点でC++23規格から**意図的に逸脱**する。

| # | 逸脱 | 参照 |
|---|---|---|
| 1 | 提供するのはC89のサブセットのみ | 第4章、第7章 |
| 2 | C++規格が要求するC99以降の追加（`isblank`、`llabs`、`strtoll`、`long long`版`abs`等）を提供しない | 第7章、D-12 |
| 3 | フリースタンディング`<cstdlib>`が要求する`abort`／`atexit`／`exit`等を提供しない | 第7章、D-11 |
| 4 | `<cerrno>`が定義するのは`EDOM`、`ERANGE`、`EINVAL`のみ | 第6章 |
| 5 | **`std::`の関数に`constexpr`を付ける**（C++規格は、規格が明示した場合を除いて禁じている）。技術的に可能な限り、全関数に付ける | 第8.2節 |
| 6 | **`<cctype>`の分類関数11個が`bool`を返す**（規格では`int`）。関数の型が変わる。引数は任意の`int`で挙動を定義する（規格では領域外は未定義） | 第8.3節、ILIBC-CTP-001 |
| 7 | **`atoi`／`atol`が、範囲外で`errno`に`ERANGE`を設定する**（C89は設定を禁じていないため、許される範囲。範囲外の結果の定義を追加する） | 第9.2節 |
| 8 | `<csetjmp>`、`<cstdio>`等、提供しないヘッダを、`#error`を含むスタブとして存在させる（素通りさせない） | 第4.3節、第4.5節 |

逸脱5の実害は小さい。`constexpr`の有無を、適合プログラムが検出する手段は、ほぼない。

---

## 4. ヘッダ一覧

C89の15ヘッダに対応するC++ヘッダのうち、**5個を本ライブラリが提供**し、4個はコンパイラ付属のものを使い、5個は`#error`のスタブとし、`<cstdio>`は条件付きで提供する（第15章）。

### 4.1 本ライブラリが提供するヘッダ（5個＋互換ラッパ5個）

| # | C++ヘッダ | 互換ラッパ | 区分 | 関数数 | 概要 |
|---|---|---|---|---|---|
| 1 | `<cctype>` | `<ctype.h>` | 関数 | 13 | 文字分類・大小文字変換 |
| 2 | `<cstring>` | `<string.h>` | 関数 | 18 | メモリ操作、文字列操作 |
| 3 | `<cstdlib>` | `<stdlib.h>` | 関数 | 6 | 絶対値、整数変換 |
| 4 | `<cassert>` | `<assert.h>` | マクロ | 0 | `assert`（失敗時はフックを呼ぶ） |
| 5 | `<cerrno>` | `<errno.h>` | マクロ・変数 | 0 | `errno`、`EDOM`、`ERANGE`、`EINVAL` |

- ライブラリ関数は**名前単位で37個**。C++規格のオーバーロードを数えると、`std::`のシグネチャは**43個**になる（第5.6節）。

### 4.2 コンパイラ付属のものを使うヘッダ（4個）

| C++ヘッダ | 互換ラッパ | 理由 |
|---|---|---|
| `<cstddef>` | `<stddef.h>` | `size_t`、`NULL`、`offsetof`等はコンパイラ依存。C++のフリースタンディング処理系が必ず提供する |
| `<climits>` | `<limits.h>` | 同上。第2.1節の`static_assert`で値を検証する |
| `<cstdarg>` | `<stdarg.h>` | `va_list`等はコンパイラ組込み機能に依存。ライブラリ内部では使用しない |
| `<cfloat>` | `<float.h>` | 浮動小数点は使わないが、フリースタンディング必須のヘッダであり、コンパイラ付属の部品が内部で参照しうるため、スタブにしない。ライブラリ内では参照しない |

### 4.3 条件付き提供ヘッダ（1個）と`#error`スタブ（5個＋C形式6個）

| C++ヘッダ | 互換ラッパ | 提供しない理由 |
|---|---|---|
| `<cstdio>` | `<stdio.h>`（常時スタブ） | 条件付き公開ヘッダ。バックエンド未指定時は`#error`（第15章） |
| `<cmath>` | `<math.h>` | 浮動小数点なし |
| `<ctime>` | `<time.h>` | 時計・RTCなし |
| `<csignal>` | `<signal.h>` | OSなし |
| `<clocale>` | `<locale.h>` | `"C"`ロケール固定 |
| `<csetjmp>` | `<setjmp.h>` | CPUアーキテクチャに強く依存し、使用頻度が低い（第1.4節） |

C99以降でC++に取り込まれた`<cwchar>`、`<cwctype>`、`<cuchar>`、`<cinttypes>`、`<cfenv>`は、提供もスタブもしない（コンパイラ付属の部品が内部で参照しうるため）。`<cstdint>`はコンパイラ付属のものを利用してよい。

### 4.4 互換ラッパ（`<xxx.h>`）の規則

| 項目 | 規則 |
|---|---|
| `<cxxx>` | 宣言を`std`名前空間にのみ置く。グローバル名前空間には追加しない |
| `<xxx.h>` | `#include <cxxx>`した後、`using std::名前;`でグローバル名前空間に導入するだけの薄いラッパ。独自の宣言・定義を持たない |
| 対象 | 第4.1節の5組。第4.2節の4組はコンパイラ付属のものをそのまま使う。第4.3節のC++5件とC形式6件はスタブ。`<cstdio>`のみ条件付き公開 |
| C言語からの利用 | **対象外（確定）**。ほとんどの関数がヘッダ内の`constexpr`関数であり、C言語からは使えない |

### 4.5 スタブの規則

提供しないヘッダを素通りさせると、コンパイラ付属の同名ヘッダが見つかり、宣言だけが見えて実体がなく、リンクエラーになる。または、誤った実装が混入する。これを防ぐため、**ヘッダ検索パスの先頭**に、次の形のスタブを置く。

```cpp
// include/stdio.h (C-compatible name remains a stub)
#if defined(ILIBC_STUB_PASSTHROUGH)
#  include_next <stdio.h>
#else
#  error "ilibc: <stdio.h> is not provided"
#endif
```

- 既定では、インクルードした時点でコンパイルエラーになる。
- コンパイラ付属の部品が、内部でスタブのヘッダを参照して失敗する場合（第10.3節）の切り分け用に、`ILIBC_STUB_PASSTHROUGH`で素通りさせられる（D-16）。通常のビルドでは定義しない。

---

## 5. 関数一覧（名前単位で37個）

以下はすべて`namespace std`内の宣言で、全関数に`noexcept`を付ける。`size_t`は`std::size_t`。

- **ND**: `[[nodiscard]]`を付ける（○）。方針は第8.4節。
- **CE**: `constexpr`にする。○は付ける。「暫定×」は、D-15の検証結果で決める。
- **†**: C++規格によるオーバーロードがある（第5.6節）。

関数IDは、個別仕様書の参照キーとして使う（形式: ヘッダ略号-連番）。

### 5.1 `<cctype>`（13個）

| ID | 関数 | シグネチャ | ND | CE | 概要 |
|---|---|---|---|---|---|
| CT-01 | `isalnum` | `bool isalnum(int c)` | ○ | ○ | 英数字か |
| CT-02 | `isalpha` | `bool isalpha(int c)` | ○ | ○ | 英字か |
| CT-03 | `iscntrl` | `bool iscntrl(int c)` | ○ | ○ | 制御文字か |
| CT-04 | `isdigit` | `bool isdigit(int c)` | ○ | ○ | 10進数字か |
| CT-05 | `isgraph` | `bool isgraph(int c)` | ○ | ○ | 空白以外の表示可能文字か |
| CT-06 | `islower` | `bool islower(int c)` | ○ | ○ | 小文字か |
| CT-07 | `isprint` | `bool isprint(int c)` | ○ | ○ | 空白を含む表示可能文字か |
| CT-08 | `ispunct` | `bool ispunct(int c)` | ○ | ○ | 区切り文字（記号）か |
| CT-09 | `isspace` | `bool isspace(int c)` | ○ | ○ | 空白類か |
| CT-10 | `isupper` | `bool isupper(int c)` | ○ | ○ | 大文字か |
| CT-11 | `isxdigit` | `bool isxdigit(int c)` | ○ | ○ | 16進数字か |
| CT-12 | `tolower` | `int tolower(int c)` | ○ | ○ | 小文字に変換 |
| CT-13 | `toupper` | `int toupper(int c)` | ○ | ○ | 大文字に変換 |

### 5.2 `<cstring>`（18個）

| ID | 関数 | シグネチャ | ND | CE | 概要 |
|---|---|---|---|---|---|
| ST-01 | `memcpy` | `void *memcpy(void *s1, const void *s2, size_t n)` | － | 暫定× | コピー（重複不可） |
| ST-02 | `memmove` | `void *memmove(void *s1, const void *s2, size_t n)` | － | 暫定× | コピー（重複可） |
| ST-03 | `memcmp` | `int memcmp(const void *s1, const void *s2, size_t n)` | ○ | 暫定× | 比較 |
| ST-04 | `memchr` † | `void *memchr(const void *s, int c, size_t n)` | ○ | 暫定× | 文字検索 |
| ST-05 | `memset` | `void *memset(void *s, int c, size_t n)` | － | 暫定× | 指定値で埋める |
| ST-06 | `strcpy` | `char *strcpy(char *s1, const char *s2)` | － | ○ | 文字列コピー |
| ST-07 | `strncpy` | `char *strncpy(char *s1, const char *s2, size_t n)` | － | ○ | 長さ指定コピー |
| ST-08 | `strcat` | `char *strcat(char *s1, const char *s2)` | － | ○ | 連結 |
| ST-09 | `strncat` | `char *strncat(char *s1, const char *s2, size_t n)` | － | ○ | 長さ指定連結 |
| ST-10 | `strcmp` | `int strcmp(const char *s1, const char *s2)` | ○ | ○ | 比較 |
| ST-11 | `strncmp` | `int strncmp(const char *s1, const char *s2, size_t n)` | ○ | ○ | 長さ指定比較 |
| ST-12 | `strchr` † | `char *strchr(const char *s, int c)` | ○ | ○ | 文字を先頭から検索 |
| ST-13 | `strrchr` † | `char *strrchr(const char *s, int c)` | ○ | ○ | 文字を末尾から検索 |
| ST-14 | `strstr` † | `char *strstr(const char *s1, const char *s2)` | ○ | ○ | 部分文字列検索 |
| ST-15 | `strspn` | `size_t strspn(const char *s1, const char *s2)` | ○ | ○ | 集合内文字が続く長さ |
| ST-16 | `strcspn` | `size_t strcspn(const char *s1, const char *s2)` | ○ | ○ | 集合外文字が続く長さ |
| ST-17 | `strpbrk` † | `char *strpbrk(const char *s1, const char *s2)` | ○ | ○ | 集合内のいずれかを検索 |
| ST-18 | `strlen` | `size_t strlen(const char *s)` | ○ | ○ | 長さ |

### 5.3 `<cstdlib>`（6個）

| ID | 関数 | シグネチャ | ND | CE | 概要 |
|---|---|---|---|---|---|
| SL-01 | `abs` † | `int abs(int j)` | ○ | ○ | 絶対値 |
| SL-02 | `labs` | `long labs(long j)` | ○ | ○ | 絶対値（long） |
| SL-03 | `atoi` | `int atoi(const char *nptr)` | ○ | ○ | 文字列→int（範囲外で`ERANGE`） |
| SL-04 | `atol` | `long atol(const char *nptr)` | ○ | ○ | 文字列→long（範囲外で`ERANGE`） |
| SL-05 | `strtol` | `long strtol(const char *nptr, char **endptr, int base)` | － | ○ | 文字列→long（基数・エラー検出付き） |
| SL-06 | `strtoul` | `unsigned long strtoul(const char *nptr, char **endptr, int base)` | － | ○ | 文字列→unsigned long |

`strtol`、`strtoul`、`atoi`、`atol`は、`errno`に`ERANGE`を設定しうる。それでも`constexpr`にできる（第8.2節）。

### 5.4 内訳

| ヘッダ | 関数数（名前単位） | `std::`シグネチャ数 |
|---|---|---|
| `<cctype>` | 13 | 13 |
| `<cstring>` | 18 | 23 |
| `<cstdlib>` | 6 | 7 |
| **合計** | **37** | **43** |

### 5.5 `ilibc::`の関数（拡張API）

| 関数 | シグネチャ | 概要 |
|---|---|---|
| `ilibc::strtol` | `long strtol(const char *nptr, char **endptr, int base, int *err)` | エラーの出力先を引数で指定する版 |
| `ilibc::strtoul` | `unsigned long strtoul(const char *nptr, char **endptr, int base, int *err)` | 同上 |
| `ilibc::atoi` | `int atoi(const char *nptr, int *err)` | 同上 |
| `ilibc::atol` | `long atol(const char *nptr, int *err)` | 同上 |
| `ilibc::_itoa_s` | `int _itoa_s(int value, char *buffer, size_t size, int radix)` | 整数を文字列に変換（非標準。第8.5節C）。エラーコードを戻り値で返す（0は成功） |
| `ilibc::_itoa_s` | `template <size_t N> int _itoa_s(int value, char (&buffer)[N], int radix)` | 配列版（バッファ長を省略） |

詳細は第8.5節。`std::`の4関数は、`&errno`を渡してerr版を呼ぶラッパである。`_itoa_s`は非標準の拡張関数で、`std::`にも、グローバルにも導入しない（互換ラッパの対象外）。

### 5.6 C++規格によるオーバーロード

**const正しいオーバーロード（5関数）**: C++の`<cstring>`は、Cの`void *`／`char *`を返す宣言を、次の2つに置き換える。const領域へのポインタを、書き込み可能なポインタとして受け取る誤用を、型検査で防げる。

| 関数 | const版 | 非const版 |
|---|---|---|
| `memchr` | `const void *memchr(const void *, int, size_t)` | `void *memchr(void *, int, size_t)` |
| `strchr` | `const char *strchr(const char *, int)` | `char *strchr(char *, int)` |
| `strrchr` | `const char *strrchr(const char *, int)` | `char *strrchr(char *, int)` |
| `strstr` | `const char *strstr(const char *, const char *)` | `char *strstr(char *, const char *)` |
| `strpbrk` | `const char *strpbrk(const char *, const char *)` | `char *strpbrk(char *, const char *)` |

**その他のオーバーロード（1個）**: `long abs(long j)`（`labs`と同じ処理）。

`long long`版・浮動小数点版の`abs`は提供しない。たとえば`std::abs(1LL)`や`std::abs(1.0)`は、`int`版と`long`版の間で曖昧になり、**コンパイルエラー**になる。暗黙の型変換で誤った結果が出ることはない。

---

## 6. 型・マクロ・変数一覧

### 6.1 型

| 型 | 定義元 | 内容 |
|---|---|---|
| `std::size_t`, `std::ptrdiff_t` | `<cstddef>`（コンパイラ付属） | `unsigned int`、`int`（ILP32） |
| `wchar_t` | （C++組込み型） | ヘッダでの定義なし。関連関数は非提供 |
| `va_list` | `<cstdarg>`（コンパイラ付属） | コンパイラ依存 |

`div_t`、`ldiv_t`、`jmp_buf`は、関数の削除とともに提供しない。

### 6.2 マクロ

| マクロ | 定義元 | 備考 |
|---|---|---|
| `NULL`, `offsetof` | `<cstddef>`（コンパイラ付属） | C++では`((void *)0)`ではない |
| `CHAR_BIT`, `INT_MAX`, `LONG_MAX`, `ULONG_MAX`等 | `<climits>`（コンパイラ付属） | 第2.1節の`static_assert`で検証 |
| `va_start`, `va_arg`, `va_end`, `va_copy` | `<cstdarg>`（コンパイラ付属） | |
| `EXIT_SUCCESS`, `EXIT_FAILURE` | `<cstdlib>` | 宣言互換のためのみ定義（`exit`は非提供）。D-17 |
| `MB_CUR_MAX` | `<cstdlib>` | 1。D-17 |
| `EDOM`, `ERANGE`, `EINVAL` | `<cerrno>` | `EDOM`は本ライブラリ内では未使用。`EINVAL`は`_itoa_s`の引数エラー用（値22） |
| `errno` | `<cerrno>` | 単一のグローバル変数を指す左辺値マクロ（第9.2節） |
| `assert` | `<cassert>` | `NDEBUG`定義時は無効化（第10.4節） |

関数形式マクロとして`isalpha`等を定義することは、C++規格が禁じるため行わない。

### 6.3 変数

| 変数 | ヘッダ | 備考 |
|---|---|---|
| `errno` | `<cerrno>` | `inline constinit int`（C++のリンケージ。Cのシンボルは提供しない）。**ライブラリ内の、唯一の書き換え可能なグローバル状態** |

---

## 7. 提供しない関数・機能の一覧

### 7.1 選定基準による削除（第1.4節）

| 関数 | 基準 | 備考 |
|---|---|---|
| `setjmp`, `longjmp` | CPUアーキテクチャに強く依存し、使用頻度が低い | 必要な利用者は、コンパイラ組込み（`__builtin_setjmp`等）またはツールチェーン付属のものを使う |
| `qsort`, `bsearch` | 使用予定がない | 将来、必要になった場合は、`<algorithm>`の使用を検討する（現時点では使用しない）。`void *`と関数ポインタのため、インライン化も`constexpr`化もできない |
| `rand`, `srand` | 隠れた状態を持ち、より良い代替がある | 乱数生成器は、状態を利用者が持つ`constexpr`対応のクラスとして、別途検討（範囲外） |
| `strtok` | 隠れた状態を持ち、再入不可 | 再入可能な代替は、将来の拡張API（D-13） |
| `div`, `ldiv`, `div_t`, `ldiv_t` | C++では冗長 | `a / b`と`a % b`を使う |

### 7.2 環境による除外

| 区分 | 関数・機能 | 除外理由 |
|---|---|---|
| `<cstdlib>` メモリ | `malloc`, `calloc`, `realloc`, `free` | ヒープなし |
| `<cstdlib>` 終了 | `abort`, `exit`, `atexit` | 終了先となる環境がない（C++23のフリースタンディング`<cstdlib>`は`abort`、`atexit`、`exit`、`at_quick_exit`、`quick_exit`を要求するが、D-11で扱う） |
| `<cstdlib>` 環境 | `getenv`, `system` | OSなし |
| `<cstdlib>` 浮動小数点 | `atof`, `strtod` | 浮動小数点なし |
| `<cstdlib>` マルチバイト | `mblen`, `mbtowc`, `wctomb`, `mbstowcs`, `wcstombs` | `"C"`ロケール固定。1バイト=1文字 |
| `<cstring>` | `strcoll`, `strxfrm` | ロケール依存。`"C"`ロケールでは不要 |
| `<cstring>` | `strerror` | メッセージ表がROMを消費。`errno`値の解釈は利用者側 |
| C99以降の追加 | `isblank`, `llabs`, `lldiv`, `atoll`, `strtoll`, `strtoull`, `strtof`, `strtold`, `_Exit`, `quick_exit`, `at_quick_exit`, `aligned_alloc` および`long long`版`abs` | C89の範囲外（C++規格は要求するが、D-12で扱う） |
| `<cerrno>` | `EDOM`、`ERANGE`、`EINVAL`以外の`E*`マクロ | C89の範囲外（`EINVAL`は拡張関数のために追加） |
| `<cstdio>`の未対応機能 | 入力、FILE*、可変引数など | I/O拡張の対象外。限定出力は第15章 |
| `<cmath>` 全体 | 22関数 | 浮動小数点なし |
| `<ctime>` 全体 | `time`, `clock`, `difftime`, `mktime`, `asctime`, `ctime`, `gmtime`, `localtime`, `strftime` | 時計なし |
| `<csignal>` | `signal`, `raise` | OSなし |
| `<clocale>` | `setlocale`, `localeconv` | `"C"`ロケール固定 |

---

## 8. C++設計規則

### 8.1 名前空間とリンケージ

| 名前空間 | 用途 |
|---|---|
| `std` | 規格の名前。シグネチャは規格どおり（第3章の逸脱、`noexcept`、`[[nodiscard]]`、`constexpr`を除く） |
| `ilibc` | 拡張API（第8.5節） |
| `ilibc::detail` | 内部補助。外部公開しない |

**実装形態**

- ほとんどの関数は、**ヘッダ内の`constexpr inline`関数**とする。`.cpp`ファイル、リンクシンボルは不要。
- **`.cpp`が必要なのは、`mem*`の5関数のみ**（`memcpy`、`memmove`、`memcmp`、`memchr`、`memset`）。コンパイラが暗黙に呼び出すため、Cのシンボル名（マングルなし）で提供する。実装は、`std::`の宣言にアセンブラ名ラベル（GCC/Clang: `__asm__("memcpy")`）を付けて、リンク名を固定する。
- `memchr`のconstオーバーロードは、基本実体（`void *`を返す）を`ilibc::detail`に置いてasmラベルでCシンボル名を与え、`std::`の2つのオーバーロードは`inline`ラッパとする（戻り値型だけが違う2つの関数は共存できないため）。
- C言語から参照できるシンボルは、`mem*`の5つのみ。

### 8.2 `constexpr`方針

**方針**: **技術的に可能な限り、規格を逸脱してでも、すべての関数に`constexpr`を付ける。** 今後追加する関数にも適用する。

**規格との関係**: C++規格は、標準ライブラリの関数に、規格が明示した場合を除いて`constexpr`を付けることを禁じている。本ライブラリは、これを意図的に破る（第3章の逸脱5）。

| 区分 | 対象 | `constexpr` | 備考 |
|---|---|---|---|
| `<cctype>` | 13関数 | ○ | |
| `<cstring>`（文字列系） | `strlen`、`strcpy`、`strncpy`、`strcat`、`strncat`、`strcmp`、`strncmp`、`strchr`、`strrchr`、`strstr`、`strspn`、`strcspn`、`strpbrk`（オーバーロード含む） | ○ | |
| `<cstring>`（メモリ系） | `memcpy`、`memmove`、`memcmp`、`memchr`、`memset` | **暫定×** | C++23では、定数式で`void *`から他のポインタ型への変換ができない（C++26で解禁）。コンパイラ組込み関数による定数評価の検証後に決める（D-15） |
| `<cstdlib>` | `abs`、`labs`、`atoi`、`atol`、`strtol`、`strtoul` | ○ | `errno`を書く経路は、定数式でなくなる（下記） |
| `ilibc::` | 第5.5節の4関数 | ○ | |

**`errno`を書く関数の扱い**: `strtol`等は、範囲外のときだけ`errno`に書く。定数式の評価中に、`errno`のようなグローバル変数へ書く経路に入ると、その評価は定数式でなくなる（コンパイルエラー）。したがって、次の性質が得られる。

- 範囲内の入力は、コンパイル時に評価できる。
- **範囲外の入力を、コンパイル時に評価すると、コンパイルエラーで検出される**（利点）。

**定数式での診断**: `constexpr`の評価中の未定義動作（範囲外アクセス、未終端文字列、符号付きオーバーフロー、`abs(INT_MIN)`等）は、コンパイルエラーとして診断される。

**将来の最適化**: ワード単位処理等の最適化版を導入する場合は、`if consteval`で、コンパイル時の基準実装と実行時の最適版を分岐する（D-05）。

### 8.3 型安全方針

1. **`std::`名の範囲**: 規格どおりのシグネチャを保つ。この範囲での安全化は、(a)const正しいオーバーロード（第5.6節）、(b)`[[nodiscard]]`、(c)`[[noreturn]]`、(d)未提供オーバーロードによる曖昧エラー（第5.6節）、(e)`<cctype>`の分類関数が`bool`を返すこと、に限る。
2. **前提のコンパイル時検査**: 第2.1節の`static_assert`。
3. それ以外の型安全は、拡張API（第8.5節）で提供し、`std::`名の動作・型を変えない。

### 8.4 属性・例外指定

| 項目 | 規則 |
|---|---|
| `noexcept` | 全関数に付ける。C++規格は、例外指定の強化を実装に許している |
| `[[nodiscard]]` | 第5章の表の方針。戻り値の無視が確実にバグとなる関数（判定、比較、検索、長さ、絶対値、変換）に付ける。`memcpy`のように戻り値が通常無視される関数や、副作用が主目的の関数（`strtol`は`endptr`と`errno`が副作用）には付けない |
| `[[noreturn]]` | `assert`失敗フックのみ |
| コンパイラ拡張属性 | `[[gnu::nonnull]]`等は、マクロ`ILIBC_NONNULL`経由で任意に付ける（既定は無効。付けるとNULL検査がコンパイラに除去される場合があるため） |
| 例外・RTTI | 使わない |

### 8.5 拡張API（`ilibc::`）

個別仕様は、各ヘッダの仕様書（ILIBC-STL-001等）と、ILIBC-EXT-001で定める。

#### A. エラー出力先を指定する版（確定）

`strtol`、`strtoul`、`atoi`、`atol`の4関数について、グローバルな`errno`を使わず、エラーの出力先を**引数で渡す版**を提供する。

```cpp
namespace ilibc {
constexpr long          strtol (const char *nptr, char **endptr, int base, int *err) noexcept;
constexpr unsigned long strtoul(const char *nptr, char **endptr, int base, int *err) noexcept;
constexpr int           atoi   (const char *nptr, int *err) noexcept;
constexpr long          atol   (const char *nptr, int *err) noexcept;
}
```

| # | 規則 |
|---|---|
| 1 | 範囲外のとき、`err`が`nullptr`でなければ、`*err`に`ERANGE`を設定する。**成功時は、`*err`を変更しない**（書かない） |
| 2 | `err`が`nullptr`のとき、報告を捨てる |
| 3 | `errno`は、まったく変更しない |
| 4 | 報告するエラーは`ERANGE`のみ |
| 5 | `std::`版は、`&errno`を渡してこれらを呼ぶラッパ（実体の本体は`ilibc::`版） |

利点: 完全に再入可能（割り込みハンドラ用の変数を渡せる）、`constexpr`の中でエラー報告まで検証できる、ライブラリが`errno`に触れる箇所が4個のラッパだけになる。

#### B. 範囲・型付き版（将来）

`void *`を使わない、範囲と型を取る版（`trivially_copyable`制約、サイズは型から算出）。`mem*`の`constexpr`による代替になる。初版の範囲外（D-13）。

#### C. 整数から文字列への変換 `_itoa_s`（確定）

非標準の拡張関数として、`ilibc::_itoa_s`（MSVCの`_itoa_s`に由来）を提供する。`stdio`を持たないilibcで、整数を文字列にする手段として必要なため（第1.4節の追加基準）。

| # | 規則 |
|---|---|
| 1 | `ilibc::`にだけ置く。`std::`にも、グローバルにも導入しない（互換ラッパの対象外） |
| 2 | **エラーコードを戻り値で返す**（`int`。`errno`と同じ型）。0は成功。`errno`は変更しない（MSVCの`_itoa_s`に準拠） |
| 3 | バッファ長を省略できる、配列を受けるテンプレート版を持つ |
| 4 | エラーは、引数の誤り（`nullptr`、`size`が0、基数が範囲外）が`EINVAL`、バッファ不足が`ERANGE` |
| 5 | 失敗の報告まで含めて`constexpr`（グローバル状態を使わない） |

詳細は、ILIBC-STL-001 第4.5節。

### 8.6 静的初期化

- グローバル状態は、`errno`の1個のみ。`inline constinit int`とし、**静的初期化のみ**を行う。
- 静的コンストラクタ・デストラクタ、`__cxa_guard_*`、`__cxa_atexit`、`.init_array`の要素を、成果物に含めない。
- この条件は、CIでシンボルとセクションを検査する（第12.2節）。

---

## 9. 共通動作規則

### 9.1 ロケールと文字集合

- `"C"`ロケールのみ。実行時に変更できない。
- `<cctype>`の判定はASCIIに基づく。0x80以上の値や、`EOF`を含む領域外の値は、分類関数で偽、変換関数で入力値そのもの、とする（ILIBC-CTP-001）。

### 9.2 `errno`の扱い

| 関数 | `errno`と、エラーの報告 |
|---|---|
| `std::strtol`, `std::strtoul`, `std::atoi`, `std::atol` | 範囲外のとき`errno`に`ERANGE`を設定。成功時は`errno`を変更しない |
| `ilibc::`の同4関数（err版） | `errno`を変更しない。範囲外のとき`*err`に`ERANGE`（`err`が非`nullptr`のとき）。成功時は`*err`を変更しない |
| `ilibc::_itoa_s` | `errno`を変更しない。エラーは、戻り値（`EINVAL`、`ERANGE`）で返す。成功は0 |
| その他の全関数 | 変更しない |

- `errno`は、単一のグローバル変数（`inline constinit int`）を指す左辺値マクロとする。
- 利用者が呼び出し前に0にする。ライブラリは0に戻さない（規格どおり）。
- `atoi`／`atol`は、`strtol`と`errno`も含めて等価とする。

### 9.3 引数検証と未定義動作

- 規格が未定義動作とする入力（`NULL`ポインタ、領域の重なる`memcpy`、終端のない文字列等）に対して、**ライブラリは検査をしない**。ただし、`<cctype>`は、任意の`int`に対して挙動を定義する。
- 開発時は、`ILIBC_DEBUG`ビルドで`assert`による検査を有効にできる（個別仕様書で定義）。

### 9.4 再入可能性・割り込みからの利用

| 区分 | 該当関数 | 備考 |
|---|---|---|
| **完全に再入可能** | `<cctype>`全関数、`<cstring>`全関数、`abs`、`labs`、`ilibc::`のerr版4関数、`ilibc::_itoa_s` | 割り込みハンドラから呼べる。コピー等で、複数のコンテキストが同じ領域を同時に操作する場合の排他は、呼び出し側の責任 |
| **`errno`を書く** | `std::strtol`、`std::strtoul`、`std::atoi`、`std::atol` | 割り込みと主処理で`errno`を共有する場合は注意。割り込みからは、err版を使う |

内部状態を持つ再入不可の関数は、ない。

### 9.5 リソース使用

| 項目 | 規則 |
|---|---|
| ヒープ | 使用しない |
| スタック | 再帰を使わない。各関数とも、数ワード以下 |
| グローバル状態 | **`errno`の1個のみ（4バイトRAM）**。`constinit` |
| 定数テーブル | なし（ROM 0バイト） |

### 9.6 `<cctype>`の実装形態

- **数式方式のみ**（範囲比較による判定。テーブルを持たない）。
- `inline constexpr`関数のみ。関数形式マクロ版は、C++規格が禁じるため作らない。

---

## 10. 依存関係と移植層

### 10.1 コンパイラが暗黙に要求する関数

フリースタンディング環境でも、コンパイラは`memcpy`、`memmove`、`memset`、`memcmp`の呼び出しを生成する（構造体コピー、配列初期化等）。また、ヘッダ内の関数のループが、`memset`／`memcpy`の呼び出しに変換されることもある。本ライブラリは、これらをCシンボル名（第8.1節）で提供する。

### 10.2 設定ヘッダ `ilibc/config.hpp`

| 設定 | 既定値 | 内容 |
|---|---|---|
| `ILIBC_DEBUG` | 未定義 | 定義すると引数検査用`assert`を有効化（`<cctype>`は領域外入力が定義済みのため対象外） |
| `ILIBC_NONNULL` | 空 | `nonnull`属性を付与するマクロ |
| `ILIBC_HOST_TEST` | 未定義 | ホスト差分テスト用（第12.1節）。`std`ではなく`ilibc_under_test`名前空間に宣言・定義し、asmラベルを付けない |
| `ILIBC_STUB_PASSTHROUGH` | 未定義 | スタブのヘッダを素通りさせる（第4.5節）。通常は定義しない |

### 10.3 コンパイラ付属ヘッダ・標準ライブラリ部品との統合

- ヘッダ検索パスは、本ライブラリを、コンパイラ付属の標準ヘッダより**前**に置く（`-isystem`の順序）。
- コンパイラ付属のフリースタンディング対応ヘッダ（`<type_traits>`、`<bit>`等、第2.2節でライブラリ自身が使うもの）が内部で`<cstdlib>`、`<cstring>`、`<cwchar>`、`<cstdio>`等を取り込む場合がある。本ライブラリのヘッダと衝突、またはスタブの`#error`が発動して、ビルドが失敗しないかを、導入時に確認する（統合試験。第12.2節）。失敗する場合は、`ILIBC_STUB_PASSTHROUGH`で原因を切り分ける。
- 統合試験の対象は、ライブラリ自身が使うヘッダ（第2.2節の一覧）とする。利用者が追加のヘッダ（`<algorithm>`、`<span>`等）を使う場合は、同様の確認を、利用者が行う。
- コンパイラ付属の部品が`abort`等を参照する場合の扱いは、D-11で定める。

### 10.4 `assert`失敗時のフック

`abort`と`stderr`がないため、`assert`失敗時は利用者が提供する次の関数を呼ぶ。

```cpp
extern "C" [[noreturn]] void __ilibc_assert_fail(const char *expr, const char *file, int line) noexcept;
```

- 戻らないことを必須とする（無限ループ、リセット、デバッガ停止等）。
- 既定の弱シンボル実装（副作用のある無限ループ）は、`ilibc.a`ではなく、**別アーカイブ`ilibc_hooks.a`**で提供する（ILIBC-HDR-001 第9章）。`ilibc.a`が定義するシンボルを、`mem*`の5個だけに保つため。

### 10.5 コンパイラ実行時ライブラリ

32bit整数の`/`、`%`（ハードウェア除算命令のないコアの場合）は、`__aeabi_idiv`等のコンパイラ実行時ライブラリに依存する。本ライブラリの範囲外とし、リンク時に別途供給する。

---

## 11. 関数間の内部依存

| 層 | 内容 | 依存先 |
|---|---|---|
| L0 | `<cctype>`全13関数、`<cstring>`全18関数、`abs`、`labs`、`ilibc::_itoa_s` | なし（すべて互いに独立） |
| L1 | `strtol`、`strtoul`（本体は`ilibc::`のerr版） | `isspace`（`<cctype>`。ヘッダ内の`constexpr inline`） |
| L2 | `atoi`、`atol` | `ilibc::strtol`（ヘッダ内） |

- 依存は、すべて**ヘッダ内のインライン関数どうし**であり、リンク時の依存を生まない。
- リンク時に必要なオブジェクトは、`mem*`の5個のみ。互いに独立。

---

## 12. ビルドと検証の方針

### 12.1 成果物とディレクトリ構成

```
ilibc/
  include/
    cctype  cstring  cstdlib  cassert  cerrno             # C++ヘッダ（正）
    ctype.h string.h stdlib.h assert.h errno.h            # 互換ラッパ
    cstdio  cmath  ctime  csignal  clocale  csetjmp       # スタブ（#error）
    stdio.h math.h time.h signal.h locale.h setjmp.h      # スタブ（#error）
    ilibc/                                               # err版、mem核、設定ヘッダ、内部ヘッダ
  src/string/                                             # mem*の5ファイルのみ
    memcpy.cpp  memmove.cpp  memcmp.cpp  memchr.cpp  memset.cpp
  test/
    constexpr/    # コンパイル時テスト（static_assert）
    host/         # PC上の差分テスト
    target/       # QEMU／実機用ハーネス
  doc/
  ilibc.a        # 5オブジェクト（mem*のみ）
  ilibc_hooks.a  # 弱シンボルの既定フック（assert失敗フック。別アーカイブ）
```

### 12.2 検証方針（概要）

1. **コンパイル時テスト**: `std::`名の`constexpr`関数を、直接`static_assert`で検証する（本ライブラリのヘッダを、コンパイラ付属のものより先に見つける設定で）。コンパイルが通れば合格で、**実行環境も出力手段も要らない**。
2. **ホスト差分テスト**: `ILIBC_HOST_TEST`で別名前空間に宣言した関数を、ホストのlibcと比較する（ランダム入力、境界値、ASan／UBSan併用）。ホストは、`-m32`でILP32にビルドする（ILIBC-TST-001 第3.2節）。
3. **ターゲット実行**: 同一テストコードを、QEMU等のフリースタンディング環境で実行する。結果は、セミホスティングまたはUART直書きの最小出力関数だけで出す（テスト対象に依存しない）。引数を`volatile`経由にして、コンパイル時の畳み込みを避ける。
4. **成果物検査（CI）**
   - `ilibc.a`が定義するグローバルシンボルが、`memcpy`、`memmove`、`memcmp`、`memchr`、`memset`の5個**だけ**であること。5個のオブジェクトに未解決シンボルがない（互いを呼ばない、自己再帰しない）こと。
   - C++ランタイム依存（`__cxa_*`、`_Unwind_*`、`__gxx_personality_*`、`operator new`／`delete`、`typeinfo`、`vtable`）がないこと。
   - 空でない`.init_array`／`.ctors`がないこと。
5. **利用者側のリンク検査**: 全37関数と拡張関数`ilibc::_itoa_s`を使うサンプルを、`-ffreestanding -fno-builtin -nostdlib`でリンクし、未解決シンボルが0であること。
6. **スタブ検査**: スタブの6ヘッダ（と`.h`）が、コンパイルエラーになること（コンパイル失敗試験）。`ILIBC_STUB_PASSTHROUGH`で素通りすること。
7. **最適化レベル確認**: `-O0`、`-Os`、`-O2`で同じ結果になること。
8. **統合試験**: ライブラリ自身が使うコンパイラ付属のヘッダ（第2.2節の一覧）と併用して、ビルドできること（スタブの`#error`が発動しないこと）。
9. 詳細は別文書（ILIBC-TST-001）で定める。

コンパイル時テストの例:

```cpp
static_assert([] {
    for (int c = -1; c < 256; ++c)
        if (std::isdigit(c) != (c >= '0' && c <= '9')) return false;
    return true;
}());
```

---

## 13. 文書体系

| 文書ID | 文書名 | 版 | 内容 |
|---|---|---|---|
| ILIBC-OVR-001 | 全体概要仕様書（本書） | 0.5 | 構成・方針・一覧 |
| ILIBC-CTP-001 | `<cctype>`関数仕様書 | 0.3 | CT-01〜CT-13 |
| ILIBC-STR-001 | `<cstring>`関数仕様書 | 0.3 | ST-01〜ST-18 |
| ILIBC-STL-001 | `<cstdlib>`関数仕様書 | 0.3 | SL-01〜SL-06、`ilibc::`のerr版、`_itoa_s` |
| ILIBC-AST-001 | `<cassert>`実装仕様書 | 0.1 | `assert`、失敗フック、`ILIBC_ASSERT` |
| ILIBC-ERR-001 | `<cerrno>`実装仕様書 | 0.1 | `errno`、エラー定数、ホスト試験モード |
| ILIBC-HDR-001 | ヘッダ仕様書 | 0.3 | ヘッダ共通規則、互換ラッパ、スタブ、設定ヘッダ、既定フック。`<cassert>`・`<cerrno>`の詳細は個別仕様書を参照 |
| ILIBC-EXT-001 | 拡張API仕様書 | 0.1 | 既存のerr引数版4関数と`_itoa_s`の2シグネチャ、共通規則 |
| ILIBC-PRT-001 | 移植ガイド | 0.1 | ツールチェーン、移植層、フック、リンク名、ヘッダ検索順序、導入・検証手順 |
| ILIBC-TST-001 | 試験仕様書 | 0.2 | 試験方式・基盤・環境、全体試験（TC-GN）、ツールチェーン試験（TC-TL）、トレーサビリティ |

各関数仕様書には、関数IDごとに次の項目を記載する。

- 宣言（`std::`のシグネチャ、オーバーロード、属性）、機能、引数、戻り値
- 境界条件、エラー時動作、`errno`
- 未定義動作／処理系定義事項の選択
- 内部状態、再入可能性、最大スタック使用量
- 依存する他関数
- 試験観点（コンパイル時テストの可否を含む）

---

## 14. 未決事項

| ID | 内容 | 候補 | 既定（暫定） |
|---|---|---|---|
| D-05 | ワード単位等の最適化 | 初版はバイト単位。将来は`if consteval`で基準実装と分岐 | **初版はバイト単位（確定）**。導入方式は別途 |
| D-08 | `sprintf`相当の最小実装を提供するか | 範囲外／別モジュールで追加 | 範囲外 |
| D-11 | `abort`等の扱い | 提供しない／利用者が移植層で提供する前提とし、コンパイラ付属部品からの参照を満たす弱シンボルを用意する | 利用者が提供（弱シンボルの既定実装を用意） |
| D-12 | C++規格が要求するC99以降の追加（`isblank`、`long long`版`abs`等）を提供するか | 提供しない／一部のみ提供 | 提供しない |
| D-13 | 拡張APIの将来範囲 | 範囲・型付き版（`mem*`の代替）、再入可能なトークン分割 | **初版はerr引数版4関数と`ilibc::_itoa_s`の2シグネチャのみ（確定）**。その他は将来候補（ILIBC-EXT-001） |
| D-14 | 対象コンパイラ | GCC／Clang系のみ（asmラベル、`include_next`、`nonnull`属性に依存）／MSVC等も対象 | GCC／Clang系。最低バージョンは導入時に決める |
| D-15 | `mem*`の`constexpr`化 | (a)非`constexpr`のまま／(b)`if consteval`で、コンパイラ組込み関数（`__builtin_memcpy`等）の定数評価に委譲（GCC／Clangでの動作を検証する。`constexpr`は`inline`を意味するため、外部シンボルの確実な生成が課題）／(c)C++26の`void *`変換の解禁後 | (a)。検証結果が良ければ(b)を採用する |
| D-16 | スタブの素通りオプション（`ILIBC_STUB_PASSTHROUGH`）を残すか | 残す／削除する | 残す（統合試験で不要と判明すれば削除） |
| D-17 | `EXIT_SUCCESS`／`EXIT_FAILURE`／`MB_CUR_MAX`を定義し続けるか | 宣言互換のため残す／`exit`等がないため削除する | 残す |

### 14.1 確定事項（旧未決事項の解決）

| 旧ID | 内容 | 決定 |
|---|---|---|
| D-01 | `strtok` | **削除**（隠れた状態、再入不可） |
| D-02 | `ctype`の実装方式 | **数式方式のみ**。分類関数は`bool`を返す |
| D-03 | `rand`のアルゴリズム | `rand`、`srand`を**削除** |
| D-04 | `qsort`のアルゴリズム | `qsort`、`bsearch`を**削除** |
| D-06 | 対象アーキテクチャ | `setjmp`、`longjmp`を**削除**。アーキテクチャ非依存 |
| D-07 | `wchar_t`の型 | C++組込み型（ヘッダでの定義なし） |
| D-09 | `std::`名への`constexpr`付与 | **付ける**。技術的に可能な限り、規格を逸脱してでも付ける方針 |
| D-10 | C言語からの`<xxx.h>`の利用 | **対象外** |
| — | `div`、`ldiv` | **削除** |
| — | `atoi`、`atol`と`errno` | 残す。範囲外で`ERANGE`を設定。`ilibc::`にerr引数版を置く（第8.5節A） |
| — | 非提供ヘッダ | **`#error`のスタブ**にする（第4.5節） |
| — | 整数から文字列への変換 | 非標準の拡張関数`ilibc::_itoa_s`を追加（第8.5節C）。`_itoa_s`のみ。`std::`にもグローバルにも導入しない。MSVC準拠で、エラーコード（`int`。0は成功、`EINVAL`、`ERANGE`）を戻り値で返し、`errno`は使わない。バッファ不足は`ERANGE` |

---

## 改版履歴

| 版 | 日付 | 内容 |
|---|---|---|
| 0.1 | 2026-10-08 | 初版ドラフト（C89向けCヘッダ名で記述） |
| 0.2 | 2026-10-08 | 実装言語をC++23に変更。ヘッダを`<cxxx>`形式に変更し、`<xxx.h>`を互換ラッパとして定義。C++設計規則、`std::`シグネチャとオーバーロード、`constexpr`の3層構成、リンク名方式、`constinit`、コンパイル時テストを追加 |
| 0.3 | 2026-10-10 | **機能の削除**: `setjmp`／`longjmp`、`qsort`／`bsearch`、`rand`／`srand`、`strtok`、`div`／`ldiv`。関数数45→37、`std::`シグネチャ54→43。書き換え可能なグローバル状態を`errno`のみに（12→4バイト）。アセンブリを廃止。**仕様の変更**: `atoi`／`atol`が範囲外で`ERANGE`を設定。`ilibc::`にerr引数版を追加。`<cctype>`を数式方式・`bool`戻り値・任意の`int`で定義済みに変更。**`constexpr`を、規格を逸脱しても技術的に可能な限り付ける方針**に変更（`ilibc::`の重複した`constexpr`核を廃止し、`std::`をヘッダ内の`constexpr inline`に一本化。`.cpp`は`mem*`のみ）。非提供ヘッダを`#error`のスタブに。選定基準（第1.4節）、逸脱一覧の更新、確定事項（第14.1節）を追加。IDを振り直し（`strlen`: ST-19→ST-18、`atoi`: SL-05→SL-03、`atol`: SL-06→SL-04、`strtol`: SL-07→SL-05、`strtoul`: SL-08→SL-06） |
| 0.3（訂正） | 2026-10-10 | `<algorithm>`への言及を整理。現時点では使用しないことを明確にし、統合試験の対象を、ライブラリ自身が使うヘッダに限定（第1.4節、第2.2節、第7.1節、第10.3節、第12.2節） |
| 0.3（訂正2） | 2026-10-10 | ヘッダ仕様書（ILIBC-HDR-001）の作成に伴う整合: 設定ヘッダの名称を`ilibc/config.hpp`に統一。`errno`のリンク名の記述を削除（Cシンボルは提供しない）。`assert`失敗フックの既定実装を、`ilibc.a`ではなく別アーカイブ`ilibc_hooks.a`に変更 |
| 0.4 | 2026-10-10 | ライブラリの正式名称を**ilibc**に決定し、仮称libc89から全面的に置き換え（名前空間`libc89::`→`ilibc::`、マクロ接頭辞`LIBC89_`→`ILIBC_`、文書ID接頭辞`LIBC89-`→`ILIBC-`、ディレクトリ・アーカイブ名、`#error`メッセージ、`assert`失敗フック`__c89_assert_fail`→`__ilibc_assert_fail`）。名称と命名規則（第1.5節）を追加。過去の改版履歴中の名称も、新名称に置き換えた。機能・仕様の変更なし |
| 0.4（訂正） | 2026-10-10 | 試験仕様書（ILIBC-TST-001）の作成に伴う整合: 文書体系の表に試験仕様書を追加。ホスト差分テストを`-m32`でビルドすることを追記 |
| 0.5 | 2026-10-10 | 非標準の拡張関数`ilibc::_itoa_s`を追加（第8.5節C）。拡張関数の追加基準（第1.4節）を新設。`<cerrno>`に`EINVAL`を追加（`_itoa_s`の引数エラー用）。関数の一覧（第5.5節）、`errno`の扱い（第9.2節）、再入可能性（第9.4節）、依存（第11章）、逸脱（第3章）、確定事項を更新。未決事項D-18（戻り値型）を追加 |
| 0.5（訂正） | 2026-10-10 | `_itoa_s`の仕様をMSVC準拠に改訂: エラーコード（`int`）を戻り値で返し、`err`引数と`errno`版を廃止（4シグネチャ→2シグネチャ）。拡張関数の追加基準（第1.4節）、一覧（第5.5節）、拡張API（第8.5節C）、`errno`の扱い（第9.2節）、再入可能性（第9.4節）、確定事項を更新。未決事項D-18を削除 |
| 0.7 | 2026-10-12 | 入力API、read backend、clear_errnoを追補（第16節） |

---


## 15. I/O非依存書式化と条件付き出力拡張（ILIBC-IO-001 v0.4）

本節はI/O拡張の採用により、旧来の「`<cstdio>`は常にスタブ」という記述を置き換える。コアlibc自体は引き続きI/O非依存である。

| ヘッダ | バックエンドなし | POSIXバックエンド指定時 |
|---|---|---|
| `<ilibc/format.hpp>` | 利用可 | 利用可 |
| `<cstdio>` | `#error` | `ilibc::`のFD出力APIを提供 |
| `<stdio.h>` | `#error` | `#error` |

`<ilibc/format.hpp>`は`snprint_f`と配列安全な`sprintf`、コンパイル時の書式解析を提供する公開ヘッダ。FD依存関数`print_f`、`fprint_f`、`puts`、`putstr`、`fputs`、`putchar`、`fputc`、`fwrite`は`<cstdio>`に置く。バックエンド選択は`ILIBC_IO_BACKEND_POSIX`（またはホスト試験用`ILIBC_HOST_IO_TEST`）で明示し、どちらもないときは`<cstdio>`を拒否する。最初の実装対象はPOSIX `::write`。独自OSは将来の差替え例。

第4.1節のコア5ヘッダ、標準関数37個、標準シグネチャ43個、`ilibc.a`の5外部定義には加算しない。`<cstdio>`は禁止スタブ6件から除外し、**条件付き公開ヘッダ**として扱う。残りのC++禁止スタブは5件、C形式は禁止スタブ6件。`<ilibc/format.hpp>`は新しい公開拡張ヘッダである。第7.2節や旧D-08の「`sprintf`相当は提供しない」はコアについての説明に限定し、拡張ヘッダからは利用可能とする。第13章の文書一覧にILIBC-IO-001 v0.4を追加する。

**新たな参照仕様書:** ILIBC-IO-001 `ilibc_cstdio_spec.md` v0.5。先行するI/Oスタブ一覧・将来対応記述と矛盾する場合、本節とIO-001を優先する。

---
## 16. 入力I/O拡張とエラー初期化の追補（ILIBC-IO-001 v0.5）

- 出力APIを維持し、`<cstdio>`に`ilibc::fgets`、配列参照限定の`gets`、`fgetc`、`getchar`、`fread`を追加する。
- 入力は`int fd`を使用する。行読み込みは1バイトずつで、内部先読み・バッファリングを行わない。
- エラー報告は`int* err`明示版、およびilibcのグローバル`errno`を使用する省略版を提供する。成功・EOFでは値を書き換えない。
- `<cerrno>`に`clear_errno()`と`clear_errno(int&)`を追加する。
- I/O非依存の`<ilibc/format.hpp>`はそのまま。`<cstdio>`のバックエンド必須条件、`<stdio.h>`禁止スタブも維持。
- POSIX `::read()`を初版の検証対象とし、BaTTaXは将来のカスタムバックエンドの例に留める。
- 既存の標準37関数/43シグネチャの集計は変更しない。入力APIはオプション拡張として別管理する。
