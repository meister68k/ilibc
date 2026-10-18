# ilibc 移植ガイド

| 項目 | 内容 |
|---|---|
| 文書ID | ILIBC-PRT-001 |
| ライブラリ名 | ilibc |
| 版 | 0.3（ドラフト） |
| 作成日 | 2026-10-12 |
| 親文書 | ILIBC-OVR-001 v0.7 |
| 対象 | ツールチェーン選定、ヘッダ統合、ビルド、リンク、assertフック、ホスト・ターゲット検証 |
| 位置づけ | 既存仕様に基づく導入・移植手順。関数動作の新規規定は行わない |

---

## 1. 目的と適用範囲

本書は、ilibcを別のCPU・ボード・GCC/Clang系ツールチェーンへ導入する際の作業順序、構成上の契約、および判定項目を整理する。

ilibcはOS、ヒープ、標準入出力、浮動小数点に依存しない、C89標準ライブラリの部分実装である。ただし、**コンパイラ実行時ライブラリ、C++の一部コンパイラ付属ヘッダ、スタートアップコード、リンカスクリプト**は利用環境側の責務である。したがって「OS不要」は「ランタイム依存が一切ない」ことを意味しない。

### 1.1 参照仕様書

| 文書ID | 参照内容 |
|---|---|
| ILIBC-OVR-001 v0.7 | 第2章（前提・オプション）、第8章（C++規則）、第10章（移植層）、第12章（ビルド）、第14章（未決事項） |
| ILIBC-HDR-001 v0.5 | ヘッダ検索、設定マクロ、スタブ、assertフック、既定フックアーカイブ |
| ILIBC-STR-001 v0.3 | `mem*`実装、Cシンボル、自己再帰防止 |
| ILIBC-STL-001 v0.3 | 整数変換と除算ランタイム依存 |
| ILIBC-TST-001 v0.2 | 試験環境E1〜E5、試験ゲート、TC-GN/TC-TL |
| ILIBC-AST-001 v0.1 | `<cassert>`と`__ilibc_assert_fail` |
| ILIBC-ERR-001 v0.1 | `errno`の実体・初期化・ホスト試験 |
| ILIBC-EXT-001 v0.1 | 拡張APIの公開範囲と非`errno`経路 |

本書と他仕様書に矛盾がある場合、関数の挙動は個別関数仕様書、ヘッダの定義はILIBC-HDR-001、試験合否はILIBC-TST-001を優先する。

## 2. 移植先の成立条件

### 2.1 必須条件

| 条件 | 要求 | 確認 |
|---|---|---|
| データモデル | `sizeof(int)==4`、`sizeof(long)==4`、`sizeof(void*)==4`（ILP32） | `static_assert` |
| バイト | `CHAR_BIT==8` | `static_assert` |
| 文字セット | 実行文字集合がASCII | `'A'==0x41`等 |
| 整数 | C++20以降の2の補数表現 | 言語水準・処理系 |
| 言語 | C++20以上、C++23推奨 | コンパイル |
| コンパイラ | GCC/Clang系（GNU asm-label、`#include_next`に依存） | TC-TL-01/05 |
| スレッド | 単一スレッド前提 | システム設計 |
| 文字の符号 | `char`はsigned/unsignedいずれも許容 | 試験両系統 |
| エンディアン | 制約なし | 異なるCPU上で再試験 |
| 例外・RTTI | 不使用 | ビルドと成果物検査 |

これらの判定は`<ilibc/config.hpp>`に集約する。対象外のデータモデルで`static_assert`を取り除いて移植することは、現行仕様の範囲外である。

### 2.2 処理系側が提供するもの

- コンパイラとC++20/23対応フロントエンド。
- `cstddef`、`cstdint`、`climits`、`limits`、`type_traits`、`concepts`、`bit`、`utility`、`version`等、概要仕様書第2.2節で利用が許されたコンパイラ付属ヘッダ。
- ターゲットに必要な整数除算等のコンパイラ実行時関数（例：ARM EABIの`__aeabi_uidiv`）。
- 必要に応じて、スタートアップ・メモリ配置・割り込みベクタ・セミホスティングまたはUART出力。
- リンカとアーカイバ、`nm`・`objdump`・`readelf`相当の成果物調査ツール。

glibc等の完全なホストlibcや、C++の完全なホストランタイムは必要条件ではない。

## 3. ディレクトリと配布成果物

設計上のファイル配置は以下とする。実際のリポジトリのMakefileとパスが異なる場合は、その構成を優先し、仕様上の責務だけを一致させる。

```text
ilibc/
  include/
    cctype cstring cstdlib cassert cerrno
    ctype.h string.h stdlib.h assert.h errno.h
    cstdio cmath ctime csignal clocale csetjmp
    stdio.h math.h time.h signal.h locale.h setjmp.h
    ilibc/config.hpp
    ilibc/assert_fail.hpp
    ilibc/string_mem.hpp
    ilibc/stdlib.hpp
    ilibc/format.hpp
    ilibc/io_backend.hpp
    ilibc/io_sink.hpp
  src/string/
    memcpy.cpp memmove.cpp memcmp.cpp memchr.cpp memset.cpp
  src/hooks/
    assert_fail.cpp
  test/
  doc/
```

| 成果物 | 内容 | 責務 |
|---|---|---|
| `include/` | ヘッダと互換ラッパ | コンパイル時に検索パスへ追加 |
| `ilibc.a` | `mem*`5関数のみを提供 | 最小の外部定義 |
| `ilibc_hooks.a` | assertの既定弱フック | アプリケーション側で上書き可能 |
| コンパイラ実行時ライブラリ | 除算等 | ツールチェーン側から供給 |

## 4. コンパイラオプションとビルド

### 4.1 共通オプション

ライブラリ自身と利用者の翻訳単位の両方に指定する（ヘッダ内の`constexpr inline`関数にも影響するため）。

```sh
-std=c++23 -pedantic -Wall -Wextra -Wconversion
-ffreestanding -fno-builtin
-fno-exceptions -fno-rtti -fno-threadsafe-statics
```

リンク段階は、独立したフリースタンディング実行ファイルを作る場合に`-nostdlib`を指定する。通常のホスト差分試験ではホストランタイムが必要なので、同じリンク方法を一律に適用しない。

最適化は試験時に`-O0`、`-Os`、`-O2`、`-O3`を切り替える（ILIBC-TST-001）。本番の最適化条件は移植先のコードサイズ・速度の要件に従う。

### 4.2 `mem*`のみの追加オプション

| コンパイラ | `src/string/mem*.cpp`で追加する指定 |
|---|---|
| GCC | `-fno-tree-loop-distribute-patterns` |
| Clang | `-fno-builtin-memcpy -fno-builtin-memmove -fno-builtin-memset -fno-builtin-memcmp` |

コンパイラがバイトコピーループを自身の`memcpy`呼び出しへ置換すると自己再帰になる危険があるため、**`mem*`実装にはこの専用条件を適用する**。`-fno-builtin`の一般指定だけでこの危険が完全に消えると仮定しない。TC-TL-03と成果物検査で確認する。

### 4.3 移植用のビルド例

以下は**構成例**であり、既存リポジトリのMakefileに追加することを強制しない。アーカイブの格納場所、ターゲット名、リンカスクリプトは環境ごとに置き換える。

```sh
CXX=arm-none-eabi-g++
AR=arm-none-eabi-ar
COMMON='-std=c++23 -Os -ffreestanding -fno-builtin -fno-exceptions -fno-rtti -fno-threadsafe-statics -mcpu=cortex-m0 -mthumb'
# <ilibc/...>を含むライブラリヘッダの探索順序は5章を参照。
# 各mem*.cppに専用の最適化対策フラグを追加して個別コンパイルし、
# 出来上がった5オブジェクトを ilibc.a に格納する。
```

**留意:** ここに示したコマンド断片は、そのまま実行できる完成ビルドスクリプトではない。移植先の完全なコマンドラインは、使用するターゲット、標準ヘッダ配置、スタートアップ、リンカスクリプトに依存する。

## 5. 標準ヘッダの探索順序

### 5.1 正規の探索経路

ilibcの`include/`を、コンパイラ付属のC++標準ヘッダより**前に**見つけさせる。ILIBC-HDR-001では、`-I`ではなく`-isystem`による登録を規定している。

```sh
g++ -std=c++23 -ffreestanding -fno-builtin \
  -isystem /path/to/ilibc/include \
  -c example.cpp -o example.o
```

この例を採用する前に、コンパイラの`-v`出力等で実際の探索順序を確認する。コマンドラインに指定したことだけでは、期待どおりの優先順位が保証されたとみなさない。

### 5.2 上書き・非上書きヘッダ

| 分類 | 名前 | 動作 |
|---|---|---|
| ilibcが提供 | `<cctype>`、`<cstring>`、`<cstdlib>`、`<cassert>`、`<cerrno>` | ilibc版を採用 |
| C形式ラッパ | `<ctype.h>`等5個 | 対応するC++版をインクルード |
| 条件付き提供 | `<cstdio>` | `ILIBC_IO_BACKEND_POSIX`等のバックエンド指定で有効、未指定は`#error`（ILIBC-IO-001第5.1節） |
| 禁止スタブ | `<cmath>`、`<ctime>`、`<csignal>`、`<clocale>`、`<csetjmp>`とC形式各6個 | 原則`#error` |
| 処理系に委ねる | `<cstddef>`、`<climits>`、`<cstdint>`等 | コンパイラ付属版を利用 |

`-nostdinc++`は指定しない。ilibc自身が一部のコンパイラ付属C++ヘッダを必要とするためである。

### 5.3 スタブの素通りと統合不具合の切り分け

禁止スタブが、コンパイラ付属ヘッダから間接的に参照される場合、`ILIBC_STUB_PASSTHROUGH`を一時的に定義し、`#include_next`で付属版を取り込んで原因を調べる。

```sh
# 診断用。通常リリース設定には含めない。
-DILIBC_STUB_PASSTHROUGH
```

これでコンパイルが進んでも、禁止したI/O等がそのまま移植可能になったとはみなさない。ヘッダ依存とリンク依存は別々に検証する。D-16により素通りオプションの将来的な存廃は未確定。

## 6. 名前空間、リンク名、外部定義

### 6.1 標準APIと拡張API

- 通常モードではC++形式の標準名は`std::`に配置する。意図的な規格逸脱（`constexpr`・戻り値等）はOVRと個別仕様書を参照する。
- 拡張の整数変換4関数および`_itoa_s`2シグネチャは`ilibc::`にのみ配置し、`<cstdlib>`から利用可能とする。
- `ILIBC_HOST_TEST`時には標準名を`ilibc_under_test`に分離し、同じ翻訳単位でホストlibcと衝突しないようにする。

### 6.2 `mem*`とasmラベル

`memcpy`、`memmove`、`memcmp`、`memchr`、`memset`は、コンパイラが暗黙に生成する呼び出しを解決できる**Cシンボル名**を持つ。C++関数宣言はasmラベルを使い、`memchr`は内部実体に同じラベルを割り当てる（ILIBC-STR-001）。

| `ilibc.a`の定義シンボル | 期待 |
|---|---|
| `memcpy` | あり |
| `memmove` | あり |
| `memcmp` | あり |
| `memchr` | あり |
| `memset` | あり |
| それ以外の定義グローバルシンボル | なし |

`extern "C"`に公開関数を置き換えるなど、既存仕様と異なるABI変更を独断で行わない。

### 6.3 リンク順序

静的アーカイブの一般的な利用では、参照側オブジェクトの**後**にライブラリを置く。

```sh
# 説明用のリンク順序例
target-ld-or-cxx  startup.o app.o ... ilibc.a ilibc_hooks.a  [compiler-runtime-library] ...
```

`ilibc_hooks.a`はフックを利用者が強シンボルで提供する場合も考慮して、規定どおりライブラリ群の最後側に置く。除算実行時ライブラリについては、該当未解決参照を解決できる位置に指定する。上記は正確なリンカ呼び出しやスクリプトを定めるものではない。

## 7. `assert`失敗フックの移植

標準出力と`abort`がないため、利用者またはボード側で次のフックを供給できる。

```cpp
extern "C" [[noreturn]]
void __ilibc_assert_fail(const char* expr, const char* file, int line) noexcept;
```

満たすべき契約:

1. **戻らない。** デバッガ停止、システムリセット、副作用のある無限ループなどを用いる。
2. ヒープ・例外・通常の標準I/Oには依存しない。
3. `expr`、`file`、`line`をログやデバッガに利用することは任意。
4. ユーザーのフックは強シンボル、`ilibc_hooks.a`内の既定実装は弱シンボル。
5. `NDEBUG`による公開`assert`の動作と、`ILIBC_DEBUG`による`ILIBC_ASSERT`の動作は別である。

既定の弱フックは、オプティマイザに消されない無限ループを使用する（ILIBC-HDR-001第9章）。ユーザー側で再実装する際に、単なる副作用のない空ループを使わない。

### 7.1 本番使用時の注意

`-nostdlib`でリンクすると、`__ilibc_assert_fail`の未解決参照は自動的には解消されない。利用者によるフック提供か`ilibc_hooks.a`のリンクを選ぶ。なお、`abort`等の追加の既定フックについては、OVRのD-11に未決事項があるため、本書では提供済みとみなさない。

## 8. `errno`と割り込み・再入性

`errno`の実体は、`inline constinit int ilibc::detail::g_errno = 0;`である。単一の共有状態であり、TLSではない。

- 通常の整数変換関数（`std::strtol`等）は、規定の範囲外エラー時に`errno`へ`ERANGE`を書き込む。
- `ilibc::`のerr引数版は`errno`を変更しない。割り込みや再入を考慮する場合は、呼び出し側がエラー格納先を保持できる後者が適する。
- 初期化を`constinit`とすることで動的初期化を回避するが、割り込みや複数実行主体間での同期・排他を提供するものではない。

`ILIBC_HOST_TEST`ではilibc側の`ILIBC_ERRNO`とホストlibcの`errno`を混同しない。

## 9. コンパイラ実行時ライブラリとスタートアップ

### 9.1 整数除算

`strtol`、`strtoul`、`_itoa_s`等は整数除算を実行する。ARM Cortex-M0等、除算命令のないコアではコンパイラがランタイム関数を参照する場合がある。

```sh
# ARM GCC環境での典型的な指定例。必要性と位置は実測で判断する。
-lgcc
```

`__aeabi_idiv`、`__aeabi_uidiv`等の正確な未解決シンボルと、ランタイムライブラリ側の提供状況はターゲットで検証する（TC-TL-09）。`libgcc`をilibcの実装の一部と混同しない。

### 9.2 スタートアップ・リンカスクリプト

スタートアップ側で、少なくとも`.data`のロード、`.bss`のゼロ初期化、スタック領域の準備を行う設計とする。`errno`の初期値0はこの前提に依存する。必要な追加初期化はターゲット処理系の責務であり、ilibc自身が独自の`init()`を要求しない。

メモリマップ、割り込みベクタ、リセット処理、ホスト通信は本書の範囲外である。

## 10. ターゲット別の導入順序

| ID | 環境 | 目的 | 主な留意点 |
|---|---|---|---|
| E1 | Debian/Linux x86 `-m32` | ホスト差分、サニタイザ、静的解析、定数評価 | 32bit C++ヘッダとlibcを揃える |
| E2 | Cortex-M3 | ARM上のランタイム検査 | エミュレータとセミホスティングを準備 |
| E3 | Cortex-M0 | 除算ランタイム依存の検査 | 除算命令なし、`-lgcc`など |
| E4 | RV32IMC | ISA独立性の検証 | RV32用C++コンパイラ・ランタイム |
| E5 | 実機（任意） | 実環境の最終検証 | フック・スタック・タイミング |

現時点で、E2〜E4の特定のエミュレータ機種やツールチェーン最低バージョンが確定したとはみなさない（ILIBC-TST-001 D-T2、OVR D-14）。

### 10.1 Debianでの最短手順

既に開発環境がある場合、新規パッケージを導入する前に以下を確認する。

```sh
g++ --version
g++ -m32 -x c++ -std=c++23 -c -o /tmp/ilibc-prt-check.o - <<'CPP'
static_assert(sizeof(int)==4 && sizeof(long)==4 && sizeof(void*)==4);
CPP

make -C test check
```

最後のコマンドは**現行リポジトリの`test/Makefile`が想定する正規ターゲットがある場合**の実行例である。実際のMakefileのターゲットと必須設定は、そのファイルの記述を優先する。

このテストが通っても、未実装・未実施のTC-TL、クロスコンパイラ、実機での動作まで合格したとは扱わない。

## 11. 移植検証と合格判定

検証の正本はILIBC-TST-001。以下は導入担当者の作業順序を簡潔に示す。

| 段階 | 実施事項 | 合否・記録 |
|---|---|---|
| 0 | TC-TL-01〜09 | 新しいコンパイラ版/ターゲットごとに実施。TC-TL-07は結果記録 |
| 1 | `mem*`ブートストラップ | 失敗した場合、以降の実行時試験を止める |
| 2 | ヘッダ単独・順序・互換ラッパ・スタブ | コンパイルと診断を確認 |
| 3 | `static_assert`とコンパイル失敗試験 | C++20/23の必要条件を確認 |
| 4 | ホスト差分とサニタイザ | E1で実施 |
| 5 | ターゲット実行 | E2→E3→E4→E5 |
| 6 | 成果物とリンク | シンボル、依存、`.init_array`等を調査 |
| 7 | 計測 | サイズ、スタック、カバレッジ等を記録 |

### 11.1 成果物に対する必須検査

```sh
# ツールチェーンに応じてプレフィックスを変更する
nm -g --defined-only ilibc.a
nm -u ilibc.a
readelf -SW some-linked-elf
```

確認事項：

- `ilibc.a`が定義するグローバルシンボルは`mem*`の5つのみ。
- 5オブジェクト内に自己再帰や相互の未解決参照がない。
- `__cxa_*`、`_Unwind_*`、`__gxx_personality_*`、`new/delete`、`typeinfo`、`vtable`等への依存がない。
- 動的初期化を示す非空の`.init_array`/`.ctors`がない。
- `errno`の実体は翻訳単位を跨いで1つになり、ガード変数を持たない。
- 独立リンクでは必要な`mem*`、除算、失敗フック等の参照が解決する。

試験項目が未作成なら「未実施」と記録し、成功に算入しない。Clangと各CPU環境はGCCホスト試験の合格をもって代替しない。

### 11.2 報告形式

最低限、`target triple`、CPU、ABI、コンパイラ名・版、言語水準、最適化レベル、設定マクロ、ライブラリ/ヘッダ探索パス、各TC IDのPASS/FAIL/SKIP/未実装、主要な`nm`/`readelf`結果を保存する。

## 12. よくある問題と切り分け

| 症状 | 典型的な要因 | 最初の確認 |
|---|---|---|
| `sizeof(long)==4`等の`static_assert`失敗 | LP64でコンパイル | `-m32`、ターゲットABI |
| `bits/c++config.h`がない | ホスト32bit C++開発環境不足 | multilib/sysroot |
| `#error "ilibc: <cstdio>..."` | バックエンド未指定の`<cstdio>`、または禁止ヘッダ経路からの参照 | `ILIBC_IO_BACKEND_POSIX`、`-H`、`ILIBC_STUB_PASSTHROUGH` |
| `memcpy`が未解決 | `ilibc.a`のリンク漏れ/順序 | `nm`とリンクコマンド |
| `memcpy`が再帰する | 最適化が自身のバイトループを関数呼び出しに変換 | `mem*`専用フラグ、逆アセンブル |
| `__aeabi_uidiv`が未解決 | 除算ランタイム不足 | `-lgcc`等 |
| `__ilibc_assert_fail`が未解決 | フックの未提供 | アプリ実装か`ilibc_hooks.a` |
| `errno`が複数あるように見える | ホスト版との混在/ODR違反 | `ILIBC_HOST_TEST`、`nm` |
| `static_assert`がコンパイラで異なる | 定数式評価・組込み関数差 | TC-TL-06/07 |
| `__cxa_*`が出現 | 標準ライブラリ依存やコンパイル設定混入 | `nm -u`、`-nostdlib` |

## 13. 変更管理と未決事項

本書はAPIやビルド契約の変更を認めるものではない。移植先で解決が必要な主な既存の未決事項を以下にまとめる。

| ID | 項目 | 現在の扱い |
|---|---|---|
| D-11 | `abort`等の追加弱フック | 将来対応。既に提供済みとは扱わない |
| D-14 | GCC/Clangの最低バージョン | 導入先で確定し記録 |
| D-15 | `mem*`の`constexpr`化 | 暫定非`constexpr`。TC-TL-07で調査 |
| D-16 | `ILIBC_STUB_PASSTHROUGH`の存廃 | 現時点では維持 |
| D-T2 | エミュレータ機種 | テスト環境決定時に選択 |
| D-T6 | 試験対象コンパイラの具体的バージョン | 版を固定して試験結果に記録 |

**実装者への注意:** 本書のシェル例は可搬性を説明するものであり、全ターゲットで検証済みのビルド手順ではない。既存の`test/Makefile`、ツールチェーンのドキュメント、ターゲットのリンカスクリプトを参照する。

## 14. 移植チェックリスト

- [ ] ILP32/ASCII/8-bit charとC++20/23の前提が成立する。
- [ ] 必要なコンパイラ付属C++ヘッダが利用可能。
- [ ] ilibcの公開ヘッダが優先して探索され、スタブが意図どおり動く。
- [ ] `mem*`にコンパイラ別最適化対策を適用。
- [ ] `ilibc.a`に5つの外部定義のみ含まれる。
- [ ] 既定または独自のassertフックを用意。
- [ ] 整数除算のランタイム依存を解決。
- [ ] `errno`の初期化と共有状態を確認。
- [ ] C++ランタイム依存・動的初期化が混入していない。
- [ ] TC-TLの結果と未実施の項目を明示。
- [ ] E1の既存試験、必要なターゲット試験が完了。
- [ ] コンパイラ版・ABI・オプションを固定して記録。

## 15. 改版履歴

| 版 | 日付 | 内容 |
|---|---|---|
| 0.1 | 2026-10-12 | 初版。既存仕様を統合し、移植・リンク・検証手順を整理 |
| 0.2 | 2026-10-12 | ILIBC-IO-001 v0.4に伴う更新: ディレクトリ一覧に`ilibc/format.hpp`・`io_backend.hpp`・`io_sink.hpp`を追加。上書き・非上書きヘッダ表の`<cstdio>`を禁止スタブから条件付き提供へ移動。トラブルシューティングの`<cstdio>`エラー行をバックエンド未指定に対応 |
| 0.3 | 2026-10-12 | 入力バックエンドとエラー伝達規則を追補（第17節） |

---


## 16. オプション書式化・POSIX出力拡張の移植（ILIBC-IO-001 v0.4）

| 機能 | 用意するもの | 実行環境 |
|---|---|---|
| `snprint_f`、安全な`sprintf` | `<ilibc/format.hpp>`のみ | フリースタンディング・I/Oなしで利用可 |
| `print_f`などFD出力 | `ILIBC_IO_BACKEND_POSIX`を定義して`<cstdio>`を取り込む | 初版はPOSIX `::write`を備えたホスト |
| 独自OSへの出力 | 将来`io_backend.hpp`の薄い境界を実装 | 追加移植時に検証。`btx_unistd.h`は一例 |

**例（I/Oなし）**

```cpp
#include <ilibc/format.hpp>
char buf[24];
int n = ilibc::sprintf<"v=%d">(buf, 123);
```

**例（ホストPOSIX）**

```cpp
#define ILIBC_IO_BACKEND_POSIX
#include <cstdio>
ilibc::print_f<"v=%d\n">(123);
```

`<cstdio>`は選択なしで`#error`とする。`<stdio.h>`の禁止は変わらない。`ILIBC_HOST_IO_TEST`はPOSIX選択のテスト用別名であり、`ILIBC_HOST_TEST`と独立。ホスト検証時はホスト`<unistd.h>`を利用し、`btx_unistd.h`が存在することを要求しない。`write`の部分書込み処理は共通FDシンクに実装し、POSIX backendは1回の`write`だけを行う。

第3章の配布構成には`include/ilibc/format.hpp`（公開）・`include/ilibc/io_backend.hpp`と`include/ilibc/io_sink.hpp`（内部）を追加。第5章の「`<cstdio>`は常に禁止スタブ」という表と第12章の該当説明は、本節の条件付き提供規則に読み替える。

---
## 17. 入力バックエンドの移植（ILIBC-IO-001 v0.5）

`<ilibc/io_backend.hpp>`は`write_some`に加え、`read_some(fd,buffer,count,err)`を提供する。I/Oを選択しないビルドでは`<cstdio>`を引き続き`#error`とし、I/O非依存のformat.hppとcerrnoは使用できる。

初版のPOSIXバックエンドはホストの`<unistd.h>`を取り込み、`::read()`を用いる。`read()`の負値に対応するホストエラー番号は直ちに保存し、指定`err`またはilibc側の`detail::g_errno`へ渡す。EOFは0としてエラーと区別する。複数のreadをまとめて先読みしない。

カスタムOSでの移植はread_some境界の実装を差し替える。`btx_unistd.h`への依存は移植必須条件にしない。`read`に相当するシステムコールのエラー番号がPOSIXと違う場合は、カスタムバックエンド側の契約として記録する。

`<cerrno>`の`clear_errno`はI/Oバックエンドなしでも利用可能。ホスト`errno`とilibcのエラー格納変数を混同しない。
