# `<cstdio>`／`<ilibc/format.hpp>` 書式化・出力・入力拡張仕様書

| 項目 | 内容 |
|---|---|
| 文書ID | ILIBC-IO-001 |
| 版 | 0.5（ドラフト） |
| 親文書 | ILIBC-OVR-001 v0.7 |
| 位置づけ | ilibcのオプション拡張。I/O非依存書式化とI/O依存出力・入力を分離 |
| 言語 | C++23（C++20で使用可能な構文に限定） |

## 1. 目的・基本方針

- I/Oを使用しない書式化は**公開ヘッダ`<ilibc/format.hpp>`**だけで利用できる。I/Oバックエンドが存在しない環境でも利用可能。
- FD書き込みが必要な関数は、**`<cstdio>`**に集約する。バックエンドを選択しない状態で`<cstdio>`をインクルードした場合は**コンパイル時`#error`**。従来のI/O禁止方針と両立する。
- `<stdio.h>`はバックエンドの有無によらず従来の`#error`スタブ。`std::`、グローバルには関数を追加せず、拡張APIは`ilibc::`に配置。
- `FILE*`、`va_list`/`va_arg`、浮動小数点、ヒープ、実行時書式解釈、出力全体のバッファリングは使用しない。
- **最初の実装・検証対象はホストPOSIXの`::write()`**。BaTTaX等への適用は将来のバックエンド追加例であり、現時点でそのヘッダの存在やABIを必須条件にしない。

## 2. 公開APIとヘッダの境界

### 2.1 `<ilibc/format.hpp>`（I/O非依存、常時利用可能）

```cpp
namespace ilibc {
  template<detail::fixed_string F, class... Args>
  int snprint_f(char* dst, std::size_t capacity, Args&&... args) noexcept;

  template<detail::fixed_string F, std::size_t N, class... Args>
  int sprintf(char (&dst)[N], Args&&... args) noexcept;
}
```

`detail::fixed_string`は文字列リテラルを非型テンプレート引数として受ける構造的型。具体的な実装詳細は非公開。公開の2関数は、`<cstdio>`を一切インクルードせずに使用できる。`<ilibc/format.hpp>`には`io_backend.hpp`、`<unistd.h>`、`btx_unistd.h`等への依存を持たせない。

### 2.2 `<cstdio>`（I/Oバックエンド必須）

```cpp
#include <ilibc/format.hpp> // 書式関連の宣言も利用可能になる
namespace ilibc {
  template<detail::fixed_string F, class... Args>
  int print_f(Args&&... args) noexcept;
  template<detail::fixed_string F, class... Args>
  int fprint_f(int fd, Args&&... args) noexcept;

  int putchar(int ch) noexcept;
  int fputc(int ch, int fd) noexcept;
  int puts(const char* s) noexcept;
  int putstr(const char* s) noexcept;
  int fputs(const char* s, int fd) noexcept;
  std::size_t fwrite(const void* ptr, std::size_t size,
                     std::size_t count, int fd) noexcept;
}
```

- `fd`は`int`型。標準Cの`FILE*`と互換ではない。
- `print_f`、`putchar`、`puts`、`putstr`はfd=1を使う。
- 書式機能のための`<ilibc/format.hpp>`と`<cstdio>`の両方から、`snprint_f`と安全な`sprintf`を使用できる。唯一の定義は`<ilibc/format.hpp>`に置く。
- 入力API（`fgets`、配列限定`gets`、`fgetc`、`getchar`、`fread`）の正本は第14章にある。本節のブロックには出力APIのみを掲載する。
- `fprintf`、`snprintf`、`vprintf`、`vsprintf`、`vsnprintf`、`scanf`系、`putc`、`putw`、`perror`、`fflush`は提供しない。

## 3. コンパイル時書式解釈

| トークン | 対応引数 | 結果 |
|---|---|---|
| 通常文字列 | なし | そのまま出力 |
| `%d` | **厳密に`int`** | 符号付き10進数、`INT_MIN`対応 |
| `%s` | `const char*`へ変換可能なNUL終端文字列 | NULを除いた文字列 |
| `%%` | なし | `%`を1文字 |

書式は`template<detail::fixed_string F>`のNTTPで渡す。`consteval`により解析し、書式トークンと引数型・個数をコンパイル時に検査する。`%x`、`%u`、`%c`、修飾子、桁数指定、単独`%`、書式内NUL、引数の過不足はコンパイルエラー。実行時に`%`を走査してはならない。

`%s`の入力は有効なNUL終端文字列でなければならず、nullポインタは契約違反。引数式は各1回評価するが、互いの評価順序は規定しない。

## 4. 内部構成と共通化

```text
<ilibc/format.hpp>  [I/O非依存]
   ├─ detail::fixed_string / consteval parser
   ├─ detail::emit_literal / emit_decimal / emit_string
   ├─ Count sink  ── sprintf の容量事前検査
   └─ Buffer sink ── snprint_f, sprintf

<cstdio>             [I/Oバックエンド必須]
   ├─ <ilibc/format.hpp> で同じparserとemit処理を利用
   ├─ FD sink / write_all / 生文字列出力
   └─ <ilibc/io_backend.hpp> ── backend::write_some(fd,buf,n)
```

- `emit_*`はシンクを型パラメータとするテンプレートで、仮想関数を使わない。
- `putstr`、`puts`、`fputs`等の書式なし関数は書式パーサを通さず、FDシンクのみ共有。
- 数値変換は`ilibc::_itoa_s`を利用する。スタック上12バイト以上の一時配列を使用し、終端NULは出力しない。
- I/O非依存の`Count`/`Buffer`シンクにリンク時の`write()`依存を生じさせない。`<ilibc/format.hpp>`単独でのコンパイル・リンクを必須試験にする。

## 5. `io_backend.hpp`と有効化条件

### 5.1 選択マクロ

| 選択 | 用途 | 状態 |
|---|---|---|
| `ILIBC_IO_BACKEND_POSIX` | POSIXホスト用：`<unistd.h>`と`::write()` | **初版実装対象** |
| `ILIBC_HOST_IO_TEST` | テスト用の従来名称。POSIXを選択する互換指定 | **初版で認める** |
| `ILIBC_IO_BACKEND_CUSTOM` | 将来の独自OS用（BaTTaX等） | 予約。初版では未実装として`#error` |
| 指定なし | I/Oを使用しない環境 | `<cstdio>`で`#error`、`<ilibc/format.hpp>`は利用可 |

`ILIBC_IO_BACKEND_POSIX`と`ILIBC_HOST_IO_TEST`は同じPOSIXバックエンドを選択し、両方指定されても矛盾しない。POSIX選択とCUSTOMの同時指定は`#error`。設定はヘッダの初回インクルード前に定義する。`ILIBC_HOST_TEST`（ホストlibcとのシンボル隔離）は独立した設定であり、バックエンド選択には使用しない。

### 5.2 バックエンド境界

`<cstdio>`は必ず`<ilibc/io_backend.hpp>`を取り込み、そこで未選択・未実装の構成を`#error`にする。内部の最小インタフェースは次のとおり。

```cpp
namespace ilibc::detail {
    // 正の値: 実際に送信したバイト数。0/負: 送信不能
    // 1回で要求サイズ全体を書ける必要はない
    int write_some(int fd, const void* buf, unsigned int size) noexcept;
}
```

POSIXバックエンドは`::write(fd,buf,size)`を呼ぶ。`size`は`unsigned int`に収め、戻り値は範囲を検査して`int`に変換する。POSIXシステムコールの実戻り型`ssize_t`とilibc境界の`int`を混同しない。要求の分割（`INT_MAX`以下）と部分書き込みの再送は**共通FDシンク**の責務とする。`::write`が負数を返した場合は`-1`、0なら進捗なしとして`-1`。`EINTR`の自動再試行はしない。書き込みの原子性は保証しない。

このAPI境界だけを将来のOS移植対象とし、`btx_unistd.h`や`btx::sys_write`は実装必須の参照ヘッダにしない。`io_backend.hpp`が存在するだけでI/O有効とみなさず、明示されたバックエンド選択を要求する。

### 5.3 使用例

```cpp
#include <ilibc/format.hpp>
char buf[16];
int n = ilibc::sprintf<"x=%d">(buf, 42); // I/Oバックエンド不要
```

```cpp
#define ILIBC_IO_BACKEND_POSIX
#include <cstdio>
ilibc::print_f<"x=%d\n">(42);
```

## 6. 戻り値・エラー

| 関数 | 成功 | 失敗 |
|---|---|---|
| `print_f`、`fprint_f` | 実際に出力したバイト数（NUL除外） | `-1` |
| `putchar`、`fputc` | 送った文字の`unsigned char`値 | `-1` |
| `puts`、`putstr`、`fputs` | 出力したバイト数。`puts`の改行も含む | `-1` |
| `fwrite` | 完全に書けた要素数 | エラーまでに完了した要素数 |
| `snprint_f` | 切り詰め前の必要長（NUL除外） | 必要長が`INT_MAX`を超えると`-1` |
| `sprintf` | 出力したバイト数（NUL除外） | 容量不足・必要長過大で`-1`、書込先は不変 |

`int`戻り値の計数はオーバーフローを検出する。`fwrite`は`size*count`のオーバーフローを出力前に検出して0を返し、`size==0`または`count==0`も0。途中で端数バイトだけ書けた場合、その端数は取り消せないが完了要素数に算入しない。ilibc自身は`errno`を設定しない（ホスト`write`によるホスト`errno`更新はあり得る）。

## 7. `snprint_f`と安全な`sprintf`

- `snprint_f<F>(dst, capacity, ...)`は、`capacity>0`なら最大`capacity-1`文字を書いてNUL終端。`capacity==0`では何も書かず、`dst==nullptr`も許容。常に切り詰め前の必要バイト数を返す。
- `capacity>0`かつ`dst==nullptr`は契約違反。入力`%s`と出力配列の重複は禁止。
- `sprintf<F>(char (&dst)[N], ...)`は2パス：Countシンクで長さを測り、`N >= 必要長+1`ならBufferシンクで書く。容量不足・必要長過大なら`-1`を返し**出力配列を一切変更しない**。
- 2パス間に引数の参照する文字列が変更されないことを要求。書式引数の式そのものを二度評価しない。
- 両関数は`write`にも`errno`にもアクセスしない。

## 8. 書式なしのFD出力関数

| 関数 | 動作 |
|---|---|
| `putstr(s)` | fd=1へ文字列を出力。改行なし |
| `puts(s)` | fd=1へ文字列の後に`\n`を1バイト出力 |
| `fputs(s,fd)` | 指定fdへ文字列を出力、改行なし |
| `putchar(c)` | fd=1へ`unsigned char`に変換した1バイトを出力 |
| `fputc(c,fd)` | 指定fdへ同じ1バイトを出力 |
| `fwrite(ptr,size,count,fd)` | 指定fdへ`size*count`バイト送出 |

文字列引数は有効なNUL終端領域を指さなければならない。`fwrite`は要求バイト数の計算時にオーバーフローを検出する。

## 9. オプションの`printf`互換マクロ

`ILIBC_ENABLE_PRINTF_MACRO`が**`<cstdio>`の初回インクルード前**に定義されている場合のみ、`printf(fmt, ...)`を`::ilibc::print_f<fmt>(...)`へ展開する。ゼロ個の値引数も可。

```cpp
#if defined(ILIBC_ENABLE_PRINTF_MACRO)
#  define printf(fmt, ...) (::ilibc::print_f<fmt>(__VA_ARGS__))
#endif
```

無指定ではマクロを一切提供しない。`std::printf`のトークン等にも干渉するため、外部ヘッダとの併用に注意し、必要なら`#undef printf`する。`fprintf`/`sprintf`/`snprintf`の互換マクロは設けない。書式文字列はNTTPに使えるリテラル限定。

## 10. ファイル構成・リンク条件

| ファイル | 区分 | 機能 |
|---|---|---|
| `include/ilibc/format.hpp` | **公開（新設）** | コンパイル時解析、`snprint_f`、安全な`sprintf` |
| `include/cstdio` | 条件付き公開 | 出力関数、`printf`互換マクロ |
| `include/ilibc/io_backend.hpp` | 内部 | POSIX `::write`への委譲。未選択なら`#error` |
| `include/ilibc/io_sink.hpp` | 内部 | 共通シンク（FD依存部分はバックエンド選択時のみ実体化） |
| `include/stdio.h` | 禁止スタブ | 常に`#error`（既存の診断用素通り設定を除く） |

I/O非依存フォーマットヘッダは単独でリンク可能。出力拡張は原則ヘッダオンリー。`ilibc.a`の外部定義は従来どおり`mem*`の5関数のみで、新規`write`実体を含めない。

## 11. 試験仕様

| ID | 試験 |
|---|---|
| TC-IO-01 | `<ilibc/format.hpp>`単独・`<cstdio>`POSIX有効時の自己完結、C++20/23 |
| TC-IO-02 | `%d`/`%s`/`%%`/空書式/多引数 |
| TC-IO-03 | 不正書式・型/個数不一致のコンパイル失敗 |
| TC-IO-04 | `INT_MIN`、`INT_MAX`、負数/0 |
| TC-IO-05 | `%s`の空文字列/長い文字列 |
| TC-IO-06 | `snprint_f`の容量0/1、切詰め、NUL、必要長 |
| TC-IO-07 | 安全`sprintf`の容量不足時不変・ポインタ拒否・2パス |
| TC-IO-08 | POSIXの`pipe/read/write`を使ったfd出力（**BaTTaX非依存**） |
| TC-IO-09 | モックバックエンドで部分書込み・0・負値を故障注入 |
| TC-IO-10 | `putstr`/`puts`/`fputs`・改行 |
| TC-IO-11 | `putchar`/`fputc`/`fwrite`・完了要素数とオーバーフロー |
| TC-IO-12 | `printf`マクロON/OFFと他ヘッダへの影響 |
| TC-IO-13 | `nm`等でヒープ/va_arg/不要実行時依存を検査 |
| TC-IO-14 | 従来の`make -C test check`による回帰 |
| TC-IO-15 | **バックエンド未指定で`<cstdio>`はコンパイル失敗。`<ilibc/format.hpp>`は成功** |
| TC-IO-16 | POSIX/CUSTOM二重指定の拒否、未実装CUSTOM拒否、`ILIBC_HOST_TEST`からの独立 |
| TC-IO-17 | I/Oなしでの`snprint_f`/`sprintf`リンク。`write`未解決参照が発生しない |
| TC-IO-18 | `<stdio.h>`がバックエンド選択にかかわらず`#error` |

TC-IOは追加試験であり、従来の試験88項目がそのまま消えるわけではない。既存試験のIDは変更しない。

## 12. 他仕様書の改訂と適用範囲

ILIBC-OVR-001、HDR-001、EXT-001、TST-001、PRT-001、ERR-001を本仕様に合わせて更新する。コアの37標準関数・43シグネチャと`ilibc.a`の5外部定義は変更しない。新機能はオプション拡張として別集計する。

## 13. 設計判断・改版履歴

| ID | 確定事項 |
|---|---|
| IO-01 | 書式化の公開先を`<ilibc/format.hpp>`へ分離 |
| IO-02 | `cstdio`は明示バックエンド選択がない場合`#error` |
| IO-03 | POSIXバックエンドを初版の唯一の完成実装対象にする |
| IO-04 | BaTTaXは追加バックエンド候補として扱う |
| IO-05 | `<stdio.h>`は従来どおりスタブ |
| IO-06 | 既存の`putstr`、`sprintf`、FD指定API等の合意動作を維持 |
| IO-07 | 入力のerr省略版はオーバーロードで`&detail::g_errno`を渡し、`nullptr`は破棄 |
| IO-08 | `read_some`の`size > INT_MAX`は`INT_MAX`に丸めて正の部分読みを返す |

| 版 | 日付 | 変更 |
|---|---|---|
| 0.1 | 2026-10-12 | 初版 |
| 0.2 | 2026-10-12 | 4項目を確定 |
| 0.3 | 2026-10-12 | 出力APIの動作仕様を確定 |
| 0.4 | 2026-10-12 | I/O非依存フォーマットヘッダ分離、バックエンド条件付き`cstdio`、POSIX優先を確定 |
| 0.5 | 2026-10-12 | 入力API（`fgets`、配列限定`gets`、`fgetc`、`getchar`、`fread`）とerr引数方式、`read_some`を確定 |

---
## 14. 入力API（2026-10-12確定）

本章は第1〜13章の**出力仕様を維持したまま追加**する入力側の正本である。提供先はバックエンドが選択された`<cstdio>`、名前空間は`ilibc::`。I/O非依存の`<ilibc/format.hpp>`には入力APIを置かない。

### 14.1 公開シグネチャ

```cpp
namespace ilibc {
    char* fgets(char* buffer, int size, int fd, int* err) noexcept;
    char* fgets(char* buffer, int size, int fd) noexcept;

    template<std::size_t N>
    char* gets(char (&buffer)[N], int fd, int* err) noexcept;
    template<std::size_t N>
    char* gets(char (&buffer)[N], int fd = 0) noexcept;

    int fgetc(int fd, int* err) noexcept;
    int fgetc(int fd) noexcept;
    int getchar(int* err) noexcept;
    int getchar() noexcept;

    std::size_t fread(void* buffer, std::size_t size,
                      std::size_t count, int fd, int* err) noexcept;
    std::size_t fread(void* buffer, std::size_t size,
                      std::size_t count, int fd) noexcept;
}
```

省略時の`err`は、各オーバーロードが`&ilibc::detail::g_errno`を渡す。明示された`err == nullptr`はエラー報告を破棄する。既存の`ILIBC_HOST_TEST`でもホスト側`errno`へ誤って報告しないため、`int* err = &errno`というデフォルト引数ではなく、上記の**オーバーロード**を正とする。

`gets`は旧C標準の無制限`gets(char*)`とは異なる**配列参照限定**の安全APIであり、ポインタ版は提供しない。`N >= 2`かつ`N <= INT_MAX`をコンパイル時に検査する。`gets(buf,fd,err)`は`fgets(buf,static_cast<int>(N),fd,err)`へ委譲し、動作を一致させる。

### 14.2 行読み込み（`fgets`と`gets`）

- `size >= 2`、書き込み可能な少なくとも`size`バイトの領域を前提とする。`buffer == nullptr`や不正なサイズは契約違反とし、動作を保証しない。
- `'\n'`を読んだ場合、その文字を格納して終了する。最大で`size-1`バイトを取り込み、成功時は必ずNUL終端する。
- バッファが先に一杯になった場合、**それ以上読み込まない**。残りの文字は次回の呼び出しで読み取る。
- 最初のバイトより前のEOFでは`nullptr`を返し、バッファは変更しない。1バイト以上読んだ後のEOFならNUL終端して`buffer`を返す。
- 読み込みエラー時は`nullptr`を返す。部分的に読み込んだバッファの内容は保証しない。エラーコードを`err`出力先へ報告する。
- ストリーム先読み・内部バッファリングは行わず、**1回の`read_some`で1バイトのみを要求する**。
- 成功・EOF時は`*err`を変更しない。これらをエラーから区別したい場合は`ilibc::clear_errno()`などを呼び出し直前に指定する。

### 14.3 文字入力（`fgetc`、`getchar`）

- `fgetc(fd)`は1バイトを読み、成功時には符号なしバイト値を`int`の0〜255として返す。
- EOFまたはエラー時は`-1`を返す（このライブラリのEOF相当値）。EOF時はエラー出力先を変更しない。エラー時のみ`*err`に番号を書き込む。
- `getchar()`は`fgetc(0)`へ委譲する。`err`明示版も同様。
- 内部先読みや押し戻し用状態は持たない。

### 14.4 バイナリ入力（`fread`）

- 指定先へ`size * count`バイトを読み込む。ゼロサイズ/ゼロ件数ならI/Oせず0を返す。
- `size * count`が`std::size_t`でオーバーフローする条件、または不正ポインタは契約違反とし、実装側はオーバーフローを検出して読み込み前に0を返す（エラーコードの追加規定は行わない）。
- 正の部分読み込みを繰り返し、要求総量を満たすかEOF/エラーで停止する。内部バッファなし。
- 戻り値は**完全に読み終えた要素数** `bytes_read / size`。端数バイトは呼び出し側の配列に残り得る。
- EOFならエラー格納先を変更しない。途中エラーなら完全要素数を返すとともに`err`へ番号を報告する。
- `fread`に文字列終端の追加処理はない。

### 14.5 エラー伝達・バックエンド

```cpp
namespace ilibc::detail {
    // 戻り値: 正値=読み込んだバイト数、0=EOF、負値=エラー
    // エラー時は、可能ならerrへバックエンド由来の番号を渡す。
    int read_some(int fd, void* buffer, unsigned int size, int* err) noexcept;
}
```

- `write_some()`に対称な入力境界を`<ilibc/io_backend.hpp>`へ追加する。
- 初版はPOSIX `::read()`を使用。POSIX `::read`が負値を返した場合、**ホスト側のエラー番号をその場で取得し**、ilibc側へコピーする。`ILIBC_HOST_TEST`ではホストの`errno`と`ilibc::detail::g_errno`は別の実体である。
- `EINTR`を含め自動再試行しない。OSが返す番号を変換せず伝達する方針とし、ilibcの公開`<cerrno>`へ個々のPOSIX `E*`定数を網羅的に追加することは本初版の必要条件としない。
- POSIX以外のバックエンド（BaTTaX等）は例示に留め、固有の`read` ABIやエラーコードを本仕様の必須条件にしない。
- `err != nullptr`ならエラー時のみ`*err`を書き換え、成功およびEOFでは書き換えない。`err == nullptr`なら報告を破棄する。エラー番号を取得できないバックエンドについては導入時に適切な報告規則を個別規定する。
- `size`が`INT_MAX`を超える場合、`read_some`は実効的な要求サイズを`INT_MAX`に絞って読み込み、得られた正の部分読みをそのまま返す。呼び出し側（`fread`等）は短い読み込みを繰り返して完了させる。1バイト要求の`fgets`には影響しない。
- ホスト側エラー番号の取得は公開`<cerrno>`を経由しない（`<errno.h>`はスタブのため）。POSIXバックエンドは`::read`の直後にそのホスト環境のエラー格納機構を直接参照して転送する。この参照は翻訳単位の外部依存としてTC-IO-13方式のnm検査対象となる。

### 14.6 使用例

```cpp
#define ILIBC_IO_BACKEND_POSIX
#include <cstdio>
#include <cerrno>

char line[128];
if (ilibc::gets(line, 0, ilibc::clear_errno())) {
    ilibc::print_f<"%s">(line);
}
int local_error;
char* p = ilibc::fgets(line, sizeof(line), 0,
                       ilibc::clear_errno(local_error));
```

**注意:** `gets`の未指定`fd`は0。`fgets`は指定サイズとFDを必要とする。標準Cの`FILE*`を使う`fgets`とはシグネチャが異なる。

## 15. 入力APIの試験追補

従来のTC-IO-01〜18は維持し、以下を追加する。TC IDの結果はILIBC-TST-001にも反映する。

| ID | 試験内容 |
|---|---|
| TC-IO-19 | `fgets`の改行保持、NUL終端、容量境界、行の継続 |
| TC-IO-20 | `gets`配列サイズ推定、デフォルトfd、非配列型のコンパイル拒否 |
| TC-IO-21 | `fgetc`/`getchar`の0〜255、EOF、エラー |
| TC-IO-22 | `fread`完全要素数、端数、EOF、ゼロサイズ、短いread |
| TC-IO-23 | `int* err`明示/省略/`nullptr`、成功・EOF時のエラー不変 |
| TC-IO-24 | POSIX `::read`、pipeおよび故障注入、エラー番号の転送 |
| TC-IO-25 | `clear_errno()`/`clear_errno(int&)`との結合、ホストerrnoの分離 |
| TC-IO-26 | バックエンドなしの`<cstdio>`拒否とformat.hppの独立性（回帰） |
