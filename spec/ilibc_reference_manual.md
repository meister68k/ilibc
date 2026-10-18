# ilibc APIリファレンスマニュアル

| 項目 | 内容 |
|---|---|
| 文書ID | ILIBC-REF-001 |
| 版 | 0.1（仕様ベース・ドラフト） |
| 作成日 | 2026-10-12 |
| 対象 | ilibcの公開API、ヘッダ、マクロおよびC/C++標準からの相違点 |
| 情報源 | ILIBC-OVR-001 v0.7、CTP-001 v0.3、STR-001 v0.3、STL-001 v0.3、ERR-001 v0.2、HDR-001 v0.5、EXT-001 v0.3、IO-001 v0.5 |

> **重要 — 実装状態:** 本書は確定済み・更新済みの**仕様書に記載された公開API**のリファレンスである。従来の`cctype`/`cstring`/`cstdlib`は実装済みとの報告があるが、今回追加したI/O入力・出力拡張および`clear_errno`は実装担当者による対応と実機検証が未確認。本書の「使用可能」は**仕様上提供される**という意味で、現在のソース一式ですべてリンクできることを保証しない。

---

## 1. まず知っておくこと

- ターゲットは**ILP32**（`int`、`long`、ポインタが32ビット）、8ビット`char`、ASCII、`"C"`ロケール。C++23を標準とし、一部ヘッダはC++20でもコンパイル可能。
- 原則、ヒープ・OS・入出力・浮動小数点・例外・RTTIなし。**I/O拡張だけは明示的なバックエンド指定時に有効。**
- C++形式の標準関数は`std::`に配置し、独自拡張は`ilibc::`。C形式の互換ヘッダは`using`による標準関数のグローバル導入だけを行う。
- 標準関数は **37関数（シグネチャ43個）**。加えて独自整数変換・書式化・I/O・エラー補助APIがある。
- C/C++標準ライブラリの完全実装ではない。ヘッダと関数は**明示的な提供一覧にあるものだけ**利用可能と考えること。
- `std::`関数への`constexpr`、`noexcept`、`[[nodiscard]]`、`cctype`分類の`bool`戻り値など、意図的な標準規格逸脱がある。

## 2. ヘッダ早見表

| インクルード | 主な提供物 | 前提 |
|---|---|---|
| `<cctype>` | ASCII文字分類、大小文字変換 | 常時 |
| `<cstring>` | メモリ・NUL終端文字列関数 | 常時 |
| `<cstdlib>` | 整数変換、絶対値、`ilibc::`拡張 | 常時 |
| `<cassert>` | `assert` | 常時、失敗フック必要 |
| `<cerrno>` | `errno`、エラー定数、`clear_errno` | 常時 |
| `<ilibc/format.hpp>` | `snprint_f`、安全`sprintf` | **I/O不要** |
| `<cstdio>` | `print_f`、`fgets`等のFD入出力 | **バックエンド選択必須** |
| `<ctype.h>` `<string.h>` `<stdlib.h>` `<assert.h>` `<errno.h>` | 上記5種のC形式互換ラッパ | 常時 |
| `<stdio.h>` | **提供しない**（`#error`） | バックエンドがあっても禁止 |
| `<cmath>` `<ctime>` `<csignal>` `<clocale>` `<csetjmp>`と対応するC形式ヘッダ | **提供しない**（`#error`） | 一部診断用passthroughを除く |
| `<cstddef>` `<climits>` `<cstdarg>` `<cfloat>` `<cstdint>`等 | コンパイラ付属版を使用 | ツールチェーン依存 |

`<cstdio>`を使用する場合：
```cpp
#define ILIBC_IO_BACKEND_POSIX
#include <cstdio>
// ホストPOSIXの read / write を使う
```
`ILIBC_HOST_IO_TEST`もPOSIXバックエンド選択の別名。`ILIBC_IO_BACKEND_CUSTOM`は予約であり、現行初版では未実装。未指定または未実装のバックエンドでは`<cstdio>`が`#error`となる。I/Oなしでも`<ilibc/format.hpp>`は使える。

## 3. `<cctype>` — 文字分類（13関数）

```cpp
#include <cctype>
```

| 関数 | 機能 |
|---|---|
| `std::isalnum(int)` | ASCII英数字 |
| `std::isalpha(int)` | ASCII英字 |
| `std::iscntrl(int)` | 制御文字 |
| `std::isdigit(int)` | `'0'`～`'9'` |
| `std::isgraph(int)` | 空白を除く印字可能文字 |
| `std::islower(int)` | `'a'`～`'z'` |
| `std::isprint(int)` | 空白を含む印字可能文字 |
| `std::ispunct(int)` | 印字可能な記号 |
| `std::isspace(int)` | 空白、`\t`、`\n`、`\v`、`\f`、`\r` |
| `std::isupper(int)` | `'A'`～`'Z'` |
| `std::isxdigit(int)` | ASCII 16進数字 |
| `std::tolower(int)` | ASCII大文字を小文字へ |
| `std::toupper(int)` | ASCII小文字を大文字へ |

**相違点と注意:** 分類11関数の戻り値は**`bool`**（標準C/C++は`int`）。全関数`constexpr noexcept`・`[[nodiscard]]`。表方式でなくASCII範囲比較で動作し、**任意の`int`を受け付ける**。0x80以上・負数・256以上などは分類でfalse、大小文字変換ではそのまま返す。標準Cの分類関数が規定する「`EOF`または`unsigned char`相当以外は未定義」と異なる。ロケール変更不可。

## 4. `<cstring>` — メモリ・文字列（18関数）

```cpp
#include <cstring>
```

| 関数 | 概要 | 補足 |
|---|---|---|
| `std::memcpy(dst,src,n)` | nバイトコピー | 重複領域は不可 |
| `std::memmove(dst,src,n)` | nバイト移動 | 重複可 |
| `std::memcmp(a,b,n)` | nバイト比較 | 符号なしバイト順 |
| `std::memchr(s,c,n)` | nバイトから値cを検索 | `const`/非`const`オーバーロード |
| `std::memset(s,c,n)` | nバイト充填 | cの下位バイト |
| `std::strcpy(dst,src)` | NULを含めコピー | 容量を自動検査しない |
| `std::strncpy(dst,src,n)` | nバイトまでコピー | NUL終端を保証しない |
| `std::strcat(dst,src)` | NUL終端文字列を連結 | 容量を自動検査しない |
| `std::strncat(dst,src,n)` | 最大n文字を連結 | 終端NULを追加 |
| `std::strcmp(a,b)` | 文字列比較 | unsigned charとして比較 |
| `std::strncmp(a,b,n)` | 最大n文字比較 | 同上 |
| `std::strchr(s,c)` | 先頭から文字検索 | `const`/非`const`版 |
| `std::strrchr(s,c)` | 末尾から文字検索 | `const`/非`const`版 |
| `std::strstr(s,needle)` | 部分文字列検索 | `const`/非`const`版 |
| `std::strspn(s,accept)` | accept文字だけからなる先頭長 | |
| `std::strcspn(s,reject)` | rejectを含まない先頭長 | |
| `std::strpbrk(s,accept)` | 指定文字集合の最初の出現 | `const`/非`const`版 |
| `std::strlen(s)` | NULまでの文字数 | NUL自身を除く |

**相違点と注意:** 全関数`noexcept`。文字列系13関数は`constexpr`（可能な関数）、`mem*`5関数はC++23の`void*`変換制約のため暫定非`constexpr`。`memchr`等5関数は戻り値の`const`性を保つオーバーロードがある。`mem*`だけが`ilibc.a`の外部定義シンボルを持ち、コンパイラ暗黙呼び出しにも対応する。実装は単純な基準実装で、最適化を保証しない。`strtok`、`strerror`は提供しない。ポインタ、文字列終端、領域の大きさなどを呼び出し側が守る必要がある。

## 5. `<cstdlib>` — 整数処理（標準名6関数）

| 関数 | 宣言の概略 | 機能 |
|---|---|---|
| `std::abs` | `int abs(int)`、`long abs(long)` | 符号付き絶対値 |
| `std::labs` | `long labs(long)` | long絶対値 |
| `std::atoi` | `int atoi(const char*)` | 10進整数変換 |
| `std::atol` | `long atol(const char*)` | 10進long変換 |
| `std::strtol` | `long strtol(const char*, char**, int)` | 基数指定符号付き変換 |
| `std::strtoul` | `unsigned long strtoul(const char*, char**, int)` | 基数指定符号なし変換 |

**相違点と注意:** すべて`constexpr noexcept`（C++標準で要求されないものも含む）。範囲外の場合`errno = ERANGE`、成功時は`errno`を変更しない。範囲外で`errno`に書く経路は`static_assert`による定数評価に使えない。`abs(INT_MIN)`および`labs(LONG_MIN)`の表現不能ケースには注意。`int`と`long`はともに32bit。`abs(long long)`・浮動小数点オーバーロードは提供しない。

### 5.1 ilibc独自の整数変換

| 関数 | 宣言の概略 | 用途 |
|---|---|---|
| `ilibc::strtol(s,end,base,err)` | `long`, `int* err`追加 | `errno`に触れずに変換エラーを受ける |
| `ilibc::strtoul(s,end,base,err)` | `unsigned long` | 同上 |
| `ilibc::atoi(s,err)` | `int` | 同上 |
| `ilibc::atol(s,err)` | `long` | 同上 |
| `ilibc::_itoa_s(value,buf,size,radix)` | `int`エラーコード | 整数を文字列化 |
| `ilibc::_itoa_s(value,arr,radix)` | `char[N]`配列版 | 配列サイズを自動取得 |

拡張の`err`版はエラー時に`*err = ERANGE`、**成功時は変更しない**。`err == nullptr`なら報告破棄。`errno`は変更しない。`_itoa_s`は成功0、失敗時エラーコードを返す（詳細な基数・負数・バッファ不足時規則はSTL仕様書を参照）。`std::`にもグローバルにも拡張名は導入しない。

### 5.2 `clear_errno` — エラー出力先の初期化

```cpp
#include <cerrno>
int* ilibc::clear_errno() noexcept;           // ilibcのグローバルerrnoを0に
constexpr int* ilibc::clear_errno(int& err) noexcept; // errを0に
```

両者とも対象変数のポインタを返す。引数あり版は**`int&`のみ**。引数なし版は`ILIBC_HOST_TEST`でもilibcの内部`g_errno`を操作する。I/Oバックエンド不要。

```cpp
int err;
auto n = ilibc::atoi("123", ilibc::clear_errno(err));
```

## 6. `<cassert>` / `<cerrno>` — マクロと実行時状態

| 名前 | 概要 |
|---|---|
| `assert(expr)` | `NDEBUG`が未定義なら条件確認、失敗で`__ilibc_assert_fail`呼び出し |
| `NDEBUG` | 公開assertを無効化。ヘッダの再インクルードで反映 |
| `ILIBC_DEBUG` | 内部`ILIBC_ASSERT`の有効化に使用 |
| `errno` | `int`の変更可能な左辺値。初期値0、単一グローバル |
| `EDOM` | 33 |
| `ERANGE` | 34 |
| `EINVAL` | 22 |
| `ILIBC_ERRNO`等 | ilibc内部用のエラー参照マクロ |

**注意:** `assert`失敗フックは`extern "C"`・`[[noreturn]]`で、利用者の強定義か`ilibc_hooks.a`の既定弱実装が必要。`errno`はスレッドローカルではない。既存の標準関数やI/Oの成功時に自動クリアされない。POSIXホストの`errno`とilibc側の`errno`は別実体（ホスト試験構成では特に注意）。

## 7. `<ilibc/format.hpp>` — I/O非依存の書式化（拡張・実装状況未確認）

書式文字列は**非型テンプレート引数**にする。通常の`printf(fmt,...)`とは呼び出し方が異なる。

```cpp
#include <ilibc/format.hpp>

char text[32];
int n = ilibc::sprintf<"value=%d">(text, 42);
int needed = ilibc::snprint_f<"%s=%d">(text, sizeof(text), "x", 123);
```

| 関数 | 用途 | 戻り値・エラー |
|---|---|---|
| `ilibc::snprint_f<fmt>(dst,capacity,args...)` | 最大capacity-1文字を書きNUL終端 | **切り詰め前の必要バイト数**。capacity=0なら書かない |
| `ilibc::sprintf<fmt>(char (&arr)[N],args...)` | 配列の容量をコンパイル時に取得、2パスで安全に書く | 成功時文字数、**容量不足時は-1で配列を一切変更しない** |

対応する変換は **`%d`（`int`）、`%s`（NUL終端文字列）、`%%`（文字`%`）のみ**。引数個数・型と書式構文はコンパイル時検査。幅・精度、`%u`、`%x`、`%c`、浮動小数点、`va_arg`はなし。実行時書式解析はしない。

`snprint_f`は必要長が`INT_MAX`を超える場合-1。`sprintf`の入力`%s`と出力配列は重複させず、2パス間に文字列を書き換えないこと。

## 8. `<cstdio>` — 条件付きI/O拡張（実装状況未確認）

`<cstdio>`は**I/Oバックエンド指定時のみ**利用できる。公開APIは`std::`でなく**`ilibc::`**、FDはすべて**`int`**（標準の`FILE*`とは互換性なし）。

### 8.1 出力

| API | 機能 | 戻り値 |
|---|---|---|
| `print_f<fmt>(args...)` | fd=1へ書式付き出力 | 出力バイト数、失敗-1 |
| `fprint_f<fmt>(fd,args...)` | 指定fdへ書式付き出力 | 同上 |
| `puts(s)` | 文字列の後に改行 | **改行込み**のバイト数、失敗-1 |
| `putstr(s)` | 文字列を改行なしで出力 | バイト数、失敗-1 |
| `fputs(s,fd)` | 指定fdへ改行なし出力 | バイト数、失敗-1 |
| `putchar(ch)` | fd=1へ1文字出力 | 書いた文字値、失敗-1 |
| `fputc(ch,fd)` | 指定fdへ1文字出力 | 書いた文字値、失敗-1 |
| `fwrite(ptr,size,count,fd)` | 指定fdへバイナリ出力 | **完全に書き終えた要素数** |

ストリームバッファリングなし。`write_some()`の正の部分書き込みは再送、0/負は停止。`EINTR`の自動再試行なし。`fwrite`の端数バイトは取り消せず、完了要素数には数えない。`puts`の正の戻り値は**標準Cの「非負なら成功」より強い独自契約**。

オプションマクロ：
```cpp
#define ILIBC_IO_BACKEND_POSIX
#define ILIBC_ENABLE_PRINTF_MACRO
#include <cstdio>
printf("x=%d\n", 123); // ilibc::print_f<"x=%d\n">(123)へ展開
```
`printf`マクロは明示指定時のみ。名前空間を無視して`std::printf`などにも干渉するので注意。

### 8.2 入力

| API | 機能 | 注意 |
|---|---|---|
| `fgets(buf,size,fd[,err])` | NUL終端の1行入力 | サイズを**明示**。改行も格納 |
| `gets(char (&arr)[N],fd=0[,err])` | 配列容量自動取得の安全版 | **ポインタ版は存在しない** |
| `fgetc(fd[,err])` | 1バイト入力 | 成功0〜255、EOF/失敗は-1 |
| `getchar([err])` | fd=0から1バイト入力 | 同上 |
| `fread(ptr,size,count,fd[,err])` | バイナリ入力 | 完全に読み込んだ要素数を返す |

ここで`[,err]`は説明用略記であり、実際には`int* err`を受け取る版と省略版の**オーバーロード**がある。`gets`の`fd`を省略する場合は`gets(arr)`または`gets(arr,fd)`を使用する。明示した`err`はエラー番号の書き込み先で、`nullptr`で報告破棄。省略時はilibcのグローバル`errno`へ報告する。

```cpp
char line[80];
int err;
auto* got = ilibc::gets(line, 0, ilibc::clear_errno(err));
```

`fgets`/`gets`は1バイトずつ`read()`相当を使い、先読みせず、改行があれば格納。`size-1`文字で打ち切ったら残りは次回読む。最初の文字の前にEOFなら`nullptr`、1文字以上読んだ後のEOFならバッファとNUL終端を返す。読み込みエラー時は`nullptr`で、部分バッファ内容は保証しない。

`fread`は要求バイト数まで繰り返し読み、途中EOFまたはエラーで止まる。端数のバイトが入力バッファに残ることがある。成功・EOFではエラー出力先を変更しないので、**呼び出し前に`clear_errno`する**と区別しやすい。初版POSIXバックエンドは`::read()`/`::write()`を使用し、BaTTaX等は将来のカスタム実装例に留める。

## 9. 主な標準との相違点・移植時注意

| 分野 | 相違・制約 | 影響 |
|---|---|---|
| C++規格との適合 | `std::`関数を独自提供、`constexpr`/`noexcept`属性も変更 | 他のC++標準ライブラリとの混在に注意 |
| 文字分類 | `bool`戻り、ASCII専用、任意のint許容 | 関数ポインタ型やロケール依存コードに差異 |
| 文字列 | 一部を除き容量検査なし | 不正な長さやNUL欠如は危険 |
| `mem*` | 一部外部シンボル、暫定非constexpr | 最適化/リンク条件の検証が必要 |
| 整数 | `int`/`long`は32bit、浮動小数点・long long版なし | LP64ホストではそのまま使えない |
| エラー | 省略時エラー格納先を持つが、成功ではクリアしない | `clear_errno`併用、ホストerrno区別 |
| `sprintf` | 配列参照専用・テンプレート書式、容量不足は未変更 | 標準`sprintf`と互換でない |
| `fgets`/`fread`等 | `FILE*`でなく`int fd`、`gets`は独自安全版 | 移植コードで呼出変更が必要 |
| 書式 | `%d` `%s` `%%`だけ | 浮動小数点、幅/精度、16進表示不可 |
| I/O | バックエンドを選ばない限り`<cstdio>`は#error | I/O不要環境では隔離できる |
| stdio.h | 依然スタブ | C言語の既存stdioコードを無修正移植できない |
| scanf | `scanf` `fscanf` `sscanf` `v*scanf`なし | 移植コードの置換が必要 |
| `errno` | TLSでない単一の変数 | マルチスレッド用途には未対応 |
| 機能削減 | `malloc`/`free`、`exit`、`atexit`、`rand`、`qsort`、`bsearch`、`strtok`等なし | 典型的な汎用アプリの移植には代替が必要 |

## 10. 使用例

### 10.1 I/Oなしで文字列操作と整数変換

```cpp
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <ilibc/format.hpp>

char text[24];
ilibc::sprintf<"N=%d">(text, 123);
int error;
long x = ilibc::strtol("42", nullptr, 10, ilibc::clear_errno(error));
```

### 10.2 POSIXバックエンドで行入力と表示

```cpp
#define ILIBC_IO_BACKEND_POSIX
#include <cstdio>
#include <cerrno>

char line[64];
int err;
if (ilibc::gets(line, 0, ilibc::clear_errno(err))) {
    ilibc::print_f<"read: %s">(line);
}
```

## 11. 実装状態と参照資料

本書のベースは仕様書であり、実装リポジトリの全APIに対する実測監査ではない。特に`<cstdio>`、`<ilibc/format.hpp>`、入力API、`clear_errno`は**仕様確定済み・実装確認未了**として扱う。

詳細な境界動作は以下を参照する。

| 内容 | 正本 |
|---|---|
| 全体の対象範囲・標準との差異 | ILIBC-OVR-001 |
| 文字分類 | ILIBC-CTP-001 |
| メモリ・文字列 | ILIBC-STR-001 |
| 整数変換・`_itoa_s` | ILIBC-STL-001 |
| `assert`・ヘッダ共通 | ILIBC-AST-001、ILIBC-HDR-001 |
| `errno`、`clear_errno` | ILIBC-ERR-001 |
| I/O非依存書式・入出力 | ILIBC-IO-001 |
| 追加APIの位置づけ | ILIBC-EXT-001 |
| ビルド・試験・移植 | ILIBC-TST-001、ILIBC-PRT-001 |
