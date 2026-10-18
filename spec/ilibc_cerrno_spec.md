# `<cerrno>` 実装仕様書

| 項目 | 内容 |
|---|---|
| 文書ID | ILIBC-ERR-001 |
| ライブラリ名 | ilibc |
| 版 | 0.2（ドラフト） |
| 親文書 | ILIBC-OVR-001 全体概要仕様書 v0.7 |
| 対象 | `<cerrno>`、`<errno.h>`、`errno`、`EDOM`、`ERANGE`、`EINVAL`、内部マクロ、`ilibc::detail::g_errno` |
| 動作の基準 | ISO C89/C90の`errno`の基本的用法およびILIBC-HDR-001 v0.5 第5章 |
| 実装方式 | ヘッダ内マクロと`inline constinit int`変数。`.cpp`は不要 |

---

## 1. 範囲と参照文書

本書は、`<cerrno>`の公開マクロ、内部実体、初期化、リンケージ、他ヘッダとの関係、実装規則、試験観点を整理する。`<cerrno>`が持つのはマクロと変数であり、関数は提供しない。

| 参照文書 | 参照箇所 |
|---|---|
| ILIBC-OVR-001 v0.7 | 第2章（環境前提）、第6章（マクロ・変数）、第8章（設計規則）、第9.2節（`errno`） |
| ILIBC-HDR-001 v0.5 | 第3章（ヘッダ共通規則）、第5章（`<cerrno>`）、第10章（TC-HD） |
| ILIBC-STL-001 v0.3 | 第2.3節、第3.2節、標準整数変換の`errno`への書き込み |
| ILIBC-TST-001 v0.2 | 共通試験方式、実行環境、成果物検査 |
| ILIBC-EXT-001 v0.1 | `errno`を変更しない拡張APIの共通規則 |

**仕様の正本:** 公開動作・定義の正本はILIBC-HDR-001第5章、整数変換の動作はILIBC-STL-001とする。本書は実装者向けの整理であり、既存の動作仕様を変更しない。

## 2. 提供物

| ID | 提供物 | 所在 | 内容 |
|---|---|---|---|
| ER-01 | `errno` | `<cerrno>` | `int`型の変更可能な左辺値へ展開する公開マクロ |
| ER-02 | `EDOM` | `<cerrno>` | 正の整数定数式、値33 |
| ER-03 | `ERANGE` | `<cerrno>` | 正の整数定数式、値34 |
| ER-04 | `EINVAL` | `<cerrno>` | 正の整数定数式、値22（本ライブラリで追加） |
| ER-05 | `ilibc::detail::g_errno` | `<cerrno>` | `errno`の単一の実体 |
| ER-06 | `ILIBC_ERRNO`、`ILIBC_EDOM`、`ILIBC_ERANGE`、`ILIBC_EINVAL` | `<cerrno>` | ライブラリ内部用マクロ |
| ER-07 | `<errno.h>` | 互換ラッパ | `<cerrno>`のみをインクルード |

- `<cerrno>`は`#pragma once`を持ち、`<ilibc/config.hpp>`をインクルードする。
- `<errno.h>`も共通のヘッダ規則に従う。`using`宣言、独自のマクロ、独自変数の定義を持たない。
- `errno`、`EDOM`、`ERANGE`、`EINVAL`は`std::`名前空間の関数・変数としては宣言しない。
- 上記以外の`E*`マクロは、本ライブラリの提供対象外とする。

## 3. 公開APIの動作仕様

### 3.1 `errno`

| 項目 | 仕様 |
|---|---|
| 形式 | オブジェクト形式マクロ |
| 展開先 | `(::ilibc::detail::g_errno)` |
| 式の型 | `int` |
| 値カテゴリ | 変更可能な左辺値 |
| 初期値 | 0 |
| 操作 | 読み取り、代入、インクリメント、`&errno`によるアドレス取得が可能 |
| 共有範囲 | 単一のグローバル実体。スレッドローカルではない |
| 名前空間 | マクロであり、`std::errno`は提供しない |

`errno`はライブラリ内部の可変グローバル状態として唯一許可された実体を参照する。利用者による読み書きも可能である。成功した関数呼び出しが`errno`を0に戻すことは保証しない。

### 3.2 エラーコード

| マクロ | 値 | 用途 |
|---|---:|---|
| `EDOM` | 33 | 宣言互換用。ilibc内では使用しない |
| `ERANGE` | 34 | 整数変換で表現範囲を超えたとき。`_itoa_s`の戻り値にも使用 |
| `EINVAL` | 22 | `ilibc::_itoa_s`の引数エラー |

すべて、プリプロセッサの`#if`で使用可能な正の整数定数式とする。3値は互いに異なる。

### 3.3 `errno`を書き換える関数

| 関数群 | 条件 | 結果 |
|---|---|---|
| `std::strtol`、`std::strtoul`、`std::atoi`、`std::atol` | 範囲外 | `errno = ERANGE` |
| 上記4関数 | 成功時・範囲外でない場合 | `errno`を変更しない |
| `ilibc::strtol`、`strtoul`、`atoi`、`atol`のerr引数版 | すべて | `errno`を変更しない。エラーは`err`引数に報告 |
| `ilibc::_itoa_s` | すべて | `errno`を変更しない。エラーは戻り値に報告 |

細かな境界条件・戻り値・`endptr`・無効な基数の動作はILIBC-STL-001に従う。

## 4. 内部実体と基準実装

### 4.1 `<cerrno>`の基準実装

次のコードはILIBC-HDR-001 v0.3第5.2節の定義を再掲したものであり、同節が正本である。

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

### 4.2 変数の実装上の条件

1. `g_errno`の型は`int`、定義は`inline constinit int g_errno = 0;`とする。
2. 定数初期化のみを行う。動的初期化・静的コンストラクタ・ガード変数を生成しない。
3. 複数翻訳単位が`<cerrno>`をインクルードしても、リンク後の`g_errno`の実体は1個とする。
4. C++リンケージを使い、Cリンカシンボル`errno`は提供しない。
5. `errno`用の`.cpp`ファイルや、`ilibc.a`に追加の定義を設けない。
6. 実際の配置とリンカによる統合は、`nm`、`readelf`等で検証する。

### 4.3 `<errno.h>`の基準実装

```cpp
// <errno.h>
#pragma once
#include <cerrno>
```

独自の定義や`using`宣言は追加しない。

## 5. インクルード順序とマクロの衝突

- ヘッダは単独でインクルード可能であり、他のヘッダより後にインクルードしても動作が変わらない（ILIBC-HDR-001 H2・H3）。
- 通常モードでは、`<cerrno>`より先に別のヘッダが`errno`マクロを定義していた場合、`#error`でビルドを停止する。無断で上書きしない。
- `errno`はオブジェクト形式マクロであり、利用者が同名の変数・メンバ・関数を宣言すると、マクロ展開により妨げられる。この挙動は既存仕様の範囲内とする。
- 公開マクロ集合・内部マクロ集合はILIBC-HDR-001第3.3節に従う。未規定のマクロを増やさない。

## 6. `ILIBC_HOST_TEST`モード

ホストlibcとの比較時には、`ILIBC_HOST_TEST`を定義する。

| 項目 | 通常モード | `ILIBC_HOST_TEST` |
|---|---|---|
| 内部`ILIBC_*`エラーマクロ | 定義する | 定義する |
| `ilibc::detail::g_errno` | 定義する | 定義する |
| 公開`errno`、`EDOM`、`ERANGE`、`EINVAL` | 定義する | 定義しない |
| ライブラリ内部のエラー参照 | `ILIBC_ERRNO`等 | `ILIBC_ERRNO`等 |

ライブラリ内部は常に内部マクロを使用する。ホスト比較用テストも、ilibc側の状態を調べるときは`ILIBC_ERRNO`を使い、ホストlibc側の`errno`と取り違えない。

これはホスト試験用のビルドモードであり、通常の公開APIを変更するものではない。

## 7. `constexpr`・再入可能性・割り込み

### 7.1 `constexpr`との関係

`errno`は変更可能なグローバル変数であり、定数評価中にその値を書き換える経路は定数式として成立しない。

- `std::strtol`等は、範囲内の入力なら`constexpr`評価が成立しうる。
- 範囲外で`ILIBC_ERRNO`へ書き込む場合、定数式としての評価は失敗する。
- `ilibc::`のerr引数版は`errno`を使わず、呼び出し側が用意した局所状態を利用できる。範囲外のケースを含めて定数評価による試験が可能である。

詳細はILIBC-STL-001第3.4節に従う。

### 7.2 再入可能性

`g_errno`は単一の共有変数であり、スレッドローカルではない。主処理と割り込み処理が共有するため、`errno`に書き込む関数を割り込みから使用する際は状態の干渉に注意する。

整数変換を割り込みハンドラから使う場合は、原則として`ilibc::`のerr引数版を選択する（ILIBC-STL-001第5章）。

## 8. 実装上の制約

| 項目 | 規則 |
|---|---|
| 言語 | C++23（C++20でもコンパイル可能な構文に限定） |
| ターゲット | ILP32、ASCII、単一スレッド |
| OS依存 | なし |
| ヒープ・I/O | 使用しない |
| 例外・RTTI | 使用しない |
| asmラベル | 使用しない |
| 独立したオブジェクトファイル | 不要 |
| 識別・設定マクロ | ILIBC-HDR-001第3章・第8章に従う |
| ヘッダの警告 | `-pedantic -Wall -Wextra -Wconversion`で警告なし |

## 9. 試験仕様

本書は新たなTC IDを追加せず、既存の試験を参照する。

| 試験ID | 検証内容 | 期待結果 |
|---|---|---|
| TC-HD-01 | `<cerrno>`、`<errno.h>`単独インクルード | C++20/23で成功 |
| TC-HD-02 | インクルード順序と重複 | 正しい結果で一致 |
| TC-HD-03 | ヘッダ警告 | 警告なし |
| TC-HD-05 | 互換ラッパと公開名 | マクロが両形式のヘッダから使用可能 |
| TC-HD-08 | 公開・内部マクロ集合 | 規定集合と一致 |
| **TC-HD-09** | `errno`左辺値、定数マクロ、初期値、複数翻訳単位の統合、衝突時`#error` | 全件成立 |
| **TC-HD-10** | 静的初期化および成果物 | 動的初期化・ガード変数なし |
| TC-SL-05 | 範囲外変換の定数評価 | 規定どおりコンパイルエラー |
| TC-SL-06 | err引数版と`errno`非変更 | 全件成立 |
| **TC-SL-07** | `std::`整数変換と`errno` | 成功時不変、範囲外で`ERANGE` |
| TC-TL-04 | `inline constinit`のリンク挙動 | ツールチェーン上で期待どおりの実体 |

### 9.1 最小の確認例

```cpp
#include <cerrno>

#if EDOM != 33 || ERANGE != 34 || EINVAL != 22
#error "ilibc: error code mismatch"
#endif

static_assert(EDOM > 0 && ERANGE > 0 && EINVAL > 0);

void check_errno_lvalue() {
    errno = 0;
    ++errno;
    int* p = &errno;
    (void)p;
}
```

初期値、複数翻訳単位での実体統合、範囲外変換によるエラー設定は、上の単一コード例だけでは検証できない。TC-HD-09とTC-SL-07を別途実施する。

## 10. 実装確認チェックリスト

- [ ] `<cerrno>`が単独でインクルードできる。
- [ ] `<errno.h>`は`<cerrno>`のみをインクルードする。
- [ ] 公開エラーコードは33、34、22であり、`#if`で使用できる。
- [ ] `errno`は変更可能な`int`左辺値である。
- [ ] `g_errno`は`inline constinit int`で初期値0となる。
- [ ] 複数翻訳単位で1実体に統合される。
- [ ] 動的初期化・静的コンストラクタが生成されない。
- [ ] 既存`errno`マクロとの衝突を検出する。
- [ ] `ILIBC_HOST_TEST`では公開マクロを定義しない。
- [ ] `std::`版整数変換のみが規定条件で`errno`を書き込む。
- [ ] err引数版と`_itoa_s`は`errno`を変更しない。
- [ ] 既存の関連TC IDを試験し、結果を記録する。

## 11. 設計決定・改版履歴

### 11.1 確定事項

| ID | 判断 | 根拠 |
|---|---|---|
| D-E1 | `errno`は単一の`inline constinit int`変数とする | ILIBC-HDR-001第5章 |
| D-E2 | `EDOM=33`、`ERANGE=34`、`EINVAL=22`とする | ILIBC-HDR-001 D-H2 |
| D-E3 | Cリンカシンボル`errno`を提供しない | ILIBC-HDR-001第5章 |
| D-E4 | ホスト試験中は公開エラーマクロを無効化する | ILIBC-HDR-001第5.2節 |
| D-E5 | 整数変換APIのerr引数版は`errno`に書かない（入力APIの省略版は`detail::g_errno`へ渡す） | ILIBC-STL-001 |
| D-E6 | 初版（v0.1）では新規関数・新規TC IDを追加しない（v0.2で`clear_errno`とCE-01〜03を追加） | 既存仕様の整理を目的とする |

### 11.2 改版履歴

| 版 | 日付 | 内容 |
|---|---|---|
| 0.1 | 2026-10-12 | 初版。ILIBC-HDR-001第5章と関連仕様の実装規則・試験観点を整理 |
| 0.2 | 2026-10-12 | `clear_errno()`および`clear_errno(int&)`を確定 |

---
## 12. `clear_errno`補助API（2026-10-12確定）

`<cerrno>`に、エラー出力先を0にしてポインタとして渡すための**2つのオーバーロード**を追加する。

```cpp
namespace ilibc {
    [[nodiscard]] constexpr int* clear_errno(int& err) noexcept {
        err = 0;
        return &err;
    }

    [[nodiscard]] inline int* clear_errno() noexcept {
        return clear_errno(detail::g_errno);
    }
}
```

- `clear_errno(int&)`は**`int`の変更可能な左辺値への参照**のみ受け取る。`int*`や他の型を受けるオーバーロードは作らない。
- `clear_errno()`はilibcのグローバル`detail::g_errno`をクリアする。ホスト`errno`とは区別する。
- 両者の戻り値は、クリアした変数のアドレス`int*`。
- `clear_errno(int&)`は`constexpr`、引数なし版はグローバル状態を書き換えるので`constexpr`を付けない。両方`noexcept`、`[[nodiscard]]`。
- I/Oに依存せず、整数変換の`err`版にも使用できる。呼び出したAPIは成功時/EOF時にエラー値を変更しないので、直前クリアは呼び出し側の任意選択。
- 入力APIの省略版は`detail::g_errno`を直接渡し、クリアは行わない。

```cpp
int err = 99;
auto* p = ilibc::clear_errno(err); // err == 0, p == &err
auto* q = ilibc::clear_errno();    // q == &ilibc::detail::g_errno
```

試験追加: CE-01（型・アドレス一致と`constexpr`）、CE-02（グローバル版・ホスト分離）、CE-03（入力APIとの結合）。既存のTC-HD試験と併せて実施する。
