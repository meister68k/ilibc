# `<cassert>` 実装仕様書

| 項目 | 内容 |
|---|---|
| 文書ID | ILIBC-AST-001 |
| ライブラリ名 | ilibc |
| 版 | 0.1（ドラフト） |
| 親文書 | ILIBC-OVR-001 全体概要仕様書 v0.5 |
| 対象 | `assert`マクロ、`<cassert>`、`<assert.h>`、失敗フック`__ilibc_assert_fail`、内部マクロ`ILIBC_ASSERT` |
| 動作の基準 | ISO C89/C90の`assert`、C++23のマクロとしての用法、およびILIBC-HDR-001 v0.3 第4章・第9章 |
| 実装方式 | ヘッダ内マクロと、利用者提供または弱シンボルの失敗フック |

---

## 1. 範囲と参照文書

本書は、`<cassert>`の宣言・展開規則・評価規則・失敗フック・リンク構成・試験観点を定める。`assert`は関数ではなくマクロであり、`std::`名前空間に名前を追加しない。

| 参照文書 | 参照箇所 |
|---|---|
| ILIBC-OVR-001 | 第2章（前提条件）、第8章（設計規則）、第12章（ビルド・検証） |
| ILIBC-HDR-001 v0.3 | 第3章（ヘッダ共通規則）、第4章（`cassert`）、第9章（弱フック）、第10章（TC-HD） |
| ILIBC-TST-001 v0.2 | 共通試験方式、試験基盤・合否判定 |

**仕様の正本:** 既存のILIBC-HDR-001第4章・第9章を優先する。本書は、実装担当者が使いやすいように規則・基準コード・試験観点を整理したものであり、既存仕様の変更を意図しない。

## 2. 提供物

| ID | 提供物 | 所在 | 役割 |
|---|---|---|---|
| AS-01 | `assert(...)` | `<cassert>` | 利用者用アサーションマクロ |
| AS-02 | `<assert.h>` | 互換ラッパ | `<cassert>`の再インクルード |
| AS-03 | `__ilibc_assert_fail` | `<ilibc/assert_fail.hpp>` | 失敗時のフック宣言 |
| AS-04 | `ILIBC_ASSERT(...)` | `<ilibc/config.hpp>` | ライブラリ内部のデバッグ用アサーション |
| AS-05 | 既定フック実装 | `src/hooks/assert_fail.cpp` / `ilibc_hooks.a` | 利用者が定義しない場合の停止動作 |

- `<cassert>`と`<assert.h>`は、**包含ガードも`#pragma once`も持たない**。`NDEBUG`の変化を、再インクルード時に反映する。
- `<ilibc/assert_fail.hpp>`は`#pragma once`を持つ。
- `assert`、`ILIBC_ASSERT`は関数形式マクロであり、関数、`constexpr`関数、`std::assert`は提供しない。
- `ILIBC_HOST_TEST`時には、既存ヘッダ仕様どおり`<cassert>`および互換ラッパをホスト差分試験に用いない。

## 3. `assert`の動作仕様

### 3.1 条件別動作

| 条件 | 式の評価 | 結果 |
|---|---|---|
| `NDEBUG`未定義、式が真 | 1回 | `void`の式として正常終了 |
| `NDEBUG`未定義、式が偽 | 1回 | `__ilibc_assert_fail`を呼ぶ。復帰しない |
| `NDEBUG`定義 | 0回 | `static_cast<void>(0)`へ展開。式の副作用なし |

- 引数は可変長マクロ引数（`...`）で受け取り、コンマを含むテンプレート式などを許容する。**空引数は対象外**。
- 有効時には`static_cast<bool>(__VA_ARGS__)`で文脈的に真偽を判断する。
- 失敗時に文字列化した式（`#__VA_ARGS__`）、`__FILE__`、`__LINE__`をフックに渡す。
- 式が真の場合、`assert`は定数評価中にも使える。偽の場合、非`constexpr`のフック呼び出しのため定数式として成立しない。
- マクロ名は再インクルードごとに`#undef assert`してから定義する。他の同名マクロが先に存在していても置き換える。

### 3.2 基準実装（`include/cassert`）

```cpp
// <cassert> -- intentionally has no include guard
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

**注意:** `#include <cassert>`を一度だけ行った後に`NDEBUG`を変更しても、既存マクロは自動的には変わらない。切り替える場合はヘッダを再度インクルードする。

### 3.3 再インクルードの例

```cpp
#include <cassert>   // assert enabled
#define NDEBUG
#include <cassert>   // assert disabled
#undef NDEBUG
#include <cassert>   // assert enabled again
```

### 3.4 定数式

```cpp
#include <cassert>

constexpr int checked(int x) {
    assert(x >= 0);
    return x;
}
static_assert(checked(1) == 1);
// static_assert(checked(-1) == -1);  // intentionally fails constant evaluation
```

## 4. 失敗フック

### 4.1 宣言（`include/ilibc/assert_fail.hpp`）

```cpp
#pragma once
extern "C" [[noreturn]]
void __ilibc_assert_fail(const char *expr, const char *file, int line) noexcept;
```

| 項目 | 契約 |
|---|---|
| リンケージ | `extern "C"`。シンボル名は`__ilibc_assert_fail` |
| `expr` | 失敗した式の文字列 |
| `file` | 呼び出し元の`__FILE__` |
| `line` | 呼び出し元の`__LINE__` |
| 戻り値 | なし。`[[noreturn]]`であり、**復帰してはならない** |
| 例外 | `noexcept`。例外を使わない |
| 呼び出し状況 | 通常処理・割り込み・任意タスクから呼ばれうる |
| 実装上の制約 | `assert`を含むilibcの機能に依存せず、再帰しない |

利用者は、ターゲットに応じて停止、リセット、デバッガブレーク等を行う強シンボルの実装を提供できる。失敗の通知後に呼び出し元へ戻る実装は契約違反である。

### 4.2 既定の弱フック

既定実装は`ilibc.a`に含めず、別アーカイブ`ilibc_hooks.a`に格納する。既存ヘッダ仕様の基準実装は次のとおり。

```cpp
// src/hooks/assert_fail.cpp
#include <ilibc/assert_fail.hpp>

extern "C" [[noreturn]] __attribute__((weak))
void __ilibc_assert_fail(const char *, const char *, int) noexcept {
    for (;;) {
        asm volatile("");
    }
}
```

- 出力手段を前提とせず、停止を継続する。
- `asm volatile("")`は空ループが最適化で除去されるのを防ぐための、既存仕様で明示された実装上の例外である。
- 利用者が強シンボルでフックを定義した場合は、そちらを使用する。
- `ilibc_hooks.a`はリンクコマンドの末尾に置く。ライブラリをリンクしなければ、呼び出されるフックの定義がない場合には未解決シンボルとなる。
- `ilibc.a`自体は`__ilibc_assert_fail`を**定義しない**。

## 5. 内部用`ILIBC_ASSERT`

`ILIBC_ASSERT`は`<ilibc/config.hpp>`に定義する内部マクロであり、**`ILIBC_DEBUG`だけ**に依存する。利用者の`NDEBUG`とは独立させる。

```cpp
#if defined(ILIBC_DEBUG)
#  include <ilibc/assert_fail.hpp>
#  define ILIBC_ASSERT(...)                                                    \
     (static_cast<bool>(__VA_ARGS__) ? static_cast<void>(0)                     \
                                     : ::__ilibc_assert_fail(#__VA_ARGS__, __FILE__, __LINE__))
#else
#  define ILIBC_ASSERT(...) (static_cast<void>(0))
#endif
```

| `ILIBC_DEBUG` | `NDEBUG` | `assert` | `ILIBC_ASSERT` |
|---|---|---|---|
| 未定義 | 未定義 | 有効 | 無効 |
| 未定義 | 定義 | 無効 | 無効 |
| 定義 | 未定義 | 有効 | 有効 |
| 定義 | 定義 | 無効 | 有効 |

内部用マクロと利用者用マクロを同一視しないこと。`ILIBC_DEBUG`の翻訳単位間での不一致は、ILIBC-HDR-001の設定一貫性規則に従う。

## 6. 互換ラッパおよび依存関係

### 6.1 `include/assert.h`

```cpp
// no include guard
#include <cassert>
```

`assert`はマクロなので、`using std::assert;`等は書かない。`<assert.h>`の再インクルードも、`<cassert>`と同じ`NDEBUG`再評価を行う。

### 6.2 ファイルと依存関係

```text
include/assert.h ------> include/cassert
                               |
                               v
                    include/ilibc/assert_fail.hpp
                               ^
                               |
include/ilibc/config.hpp ---(ILIBC_DEBUG時)

src/hooks/assert_fail.cpp -> ilibc_hooks.a
```

`<cassert>`自体のための`.cpp`は不要。公開ヘッダは他のヘッダを先に含めなくてもコンパイルでき、ヘッダ内のコメント・診断メッセージはASCIIで記述する。

## 7. 制約と注意事項

1. **文字列領域:** 有効な`assert`ごとに式文字列とソースファイル名の文字列がROMへ残りうる。製品ビルドで省くなら`NDEBUG`を定義する。ソースパス短縮には`-ffile-prefix-map`を利用できる。
2. **副作用:** `assert(++counter)`のような使い方では、`NDEBUG`の有無でプログラムの結果が変わる。プログラム動作に必要な副作用を条件式に含めない。
3. **停止処理:** フック実装は割り込みコンテキストを含む呼び出しを想定し、libcへの依存や再帰を避ける。
4. **最適化:** `[[noreturn]]`契約と既定弱フックの非復帰性を守る。フックを正常復帰させて試験する方式は採用しない。
5. **標準との関係:** `assert`はグローバルなマクロであり、`std`名前空間の関数ではない。拡張の内部用`ILIBC_ASSERT`はISO C89の公開APIには含めない。

## 8. 試験仕様

試験の実施方式、環境、合否判定、報告方式はILIBC-TST-001による。TC番号を新設せず、既存のILIBC-HDR-001の項目を利用する。

| TC ID | 対象 | 検証の要点 |
|---|---|---|
| TC-HD-01 | ヘッダの自己完結性 | `<cassert>`、`<assert.h>`、フックヘッダが単独でコンパイルできる |
| TC-HD-02 | 順序非依存・重複 | 互換ラッパを含む重複・順序違いで整合する |
| TC-HD-03 | 警告 | 指定警告オプションで警告なし |
| TC-HD-06 | `assert`の機能 | `NDEBUG`切替、無効時の非評価、コンマ式、定数式の成功／失敗 |
| TC-HD-07 | 失敗フック | 式・ファイル・行番号、呼び出し有無、`ILIBC_ASSERT`との独立性 |
| TC-HD-08 | マクロ集合 | 想定外のマクロ追加がない |
| TC-HD-15 | 設定一貫性 | `ILIBC_DEBUG`混在時の検出または禁止 |
| TC-HD-16 | リンク | 利用者強フック、既定弱フック、フックなしの未解決シンボル |
| TC-HD-17 | 成果物 | `ilibc.a`がフックを定義しない |

### 8.1 最小試験コード例

```cpp
#include <cassert>

constexpr int test_constexpr(int n) {
    assert(n >= 0);
    return n;
}
static_assert(test_constexpr(0) == 0);

int test_disabled() {
    int n = 0;
#define NDEBUG
#include <cassert>
    assert(++n);
    return n; // 0
}
#undef NDEBUG
#include <cassert>
```

**試験上の注意:** 強フックの試験は、フック内部から通常復帰させない。停止・終了を別プロセスから観測するなど、`[[noreturn]]`契約を維持できるハーネスを用いる。

## 9. 実装確認時のチェックリスト

- [ ] `<cassert>`、`<assert.h>`に包含ガードがない。
- [ ] `assert`を毎回`#undef`してから再定義する。
- [ ] `NDEBUG`無効時に式を1回だけ評価する。
- [ ] `NDEBUG`有効時に式を評価しない。
- [ ] コンマを含むテンプレート式に対応する。
- [ ] 真の条件は定数式で使える。
- [ ] フックのCリンケージ、`noexcept`、`[[noreturn]]`を守る。
- [ ] `ILIBC_ASSERT`は`NDEBUG`から独立している。
- [ ] `ilibc.a`にフック定義がなく、既定実装は`ilibc_hooks.a`にある。
- [ ] TC-HD-06、07、16、17などが期待どおりに実施・合格する。

## 10. 設計決定と改版履歴

| ID | 決定内容 | 根拠 |
|---|---|---|
| D-AS1 | `<cassert>`、`<assert.h>`に包含ガードを設けない | ILIBC-HDR-001 H1、第4章 |
| D-AS2 | 式は可変長マクロで受け取り、`NDEBUG`により評価を切り替える | ILIBC-HDR-001 第4.1〜4.2節 |
| D-AS3 | 失敗フックは利用者提供を優先し、既定弱定義を別アーカイブに置く | ILIBC-HDR-001 第4.3節、第9章 |
| D-AS4 | `ILIBC_ASSERT`は`ILIBC_DEBUG`のみで制御する | ILIBC-HDR-001 第4.4節 |

| 版 | 日付 | 変更内容 |
|---|---|---|
| 0.1 | 2026-10-12 | 初版。既存ヘッダ仕様から`cassert`の実装・リンク・試験規則を整理 |
