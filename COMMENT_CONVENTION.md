# COMMENT_CONVENTION.md — コメント規約

## 1. 目的と適用範囲

- リポジトリ内のソースコード・テストに共通するコメントの書き方を定める。
- AGENTS.md の全レイヤー共通ルールと併用する。本規約は書式・配置のルールを追加するもので、AGENTS.md の内容 (コメントは不変条件・順序・安全性の理由を説明すること 等) を覆さない。
- 既存ファイル (mkfs.rb 等) に残る英語ヘッダや既存コメントはそのまま維持してよい。新規に書くコメントから本規約を適用する。

## 2. 基本方針

- コメントは日本語で書く。識別子・定数名・仕様参照 (boundary-tests.md §3 など) は ASCII / 英語のまま残す。
- 自明な行にコメントを付けない。意味が一読で判らない値・制約・理由 (バイト順、上限値、失敗時の挙動、レイヤー間の責務の境界など) に付ける。
- 行コメント `//` のみを使う。複数行に及ぶときも各行を `// ` で継続する。
- 原則、各関数・各処理ブロック・説明が必要な宣言にはコメントを付ける。コメントのない関数は認めない。

## 3. 種別別のルール

### 3.1 ファイルヘッダ

- 1行目: `// <ファイル名> — <役割。>`
- 2行目以降: `// <検証対象・仕様参照>`

```cpp
// serialization_test.cpp — 直列化レイヤー(spec/01-serialization.md)のテスト。
// spec/boundary-tests.md §3 と AGENTS.md §18 の serialization 項目を検証する。
```

### 3.2 宣言の前 (グローバル・定数・マクロ・ヘルパー関数)

- 宣言 / 定義の直上に1行 (間に空行を入れない)。
- 補足情報は半角括弧 `( )` で括る。
- 述語関数は「〜か」、値や動作の説明は「〜する」「〜のこと」で書く。

```cpp
// バイト列 a と b の先頭 n バイトが一致するか
static bool bytes_match(const uint8_t* a, const uint8_t* b, uint32_t n) {
```

```cpp
// 通過したチェック数と失敗数(失敗が1つでもあれば終了コード1)
static int g_checks = 0;
```

### 3.3 テスト関数の前

- 検証内容を1行で書く。形式は「〜を検証」か「〜か」の2種。
- 複数条件は「、」や「/」で列挙し、前提・補足は `( )` に入れる。

```cpp
// スーパーブロックの encode が既知バイト列になるか、decode で復元されるか
static void test_superblock_known_vector() {
```

```cpp
// 非アラインなバッファ位置(オフセット1/3/5)でも3レコード型が動作するか
static void test_unaligned() {
```

### 3.4 セクション区切り

- `// ----…---- <単語>` の形 (単語は行末に置く)。
- 直下にセクションの狙いを1行置いてもよい。

```cpp
// -------------------------------------------------------------- unaligned

// 非アラインなバッファ位置(オフセット1/3/5)でも3レコード型が動作するか
static void test_unaligned() {
```

### 3.5 処理ブロックの前

- 論理的な処理のまとまりの直前に1行 (ブロックと同じインデント)。
- 理由・前提を添えたいときは自然な句切れで2行に折り返す。
- レイヤー間の責務の境界や正当性の前提は明記する。

```cpp
// decode方向の既知ベクトル
const uint8_t raw[32] = {...};
```

```cpp
// 空名はシリアライズ上は正当。使用中direntとしての拒否はdirectory層の責務。
```

### 3.6 式・代入の行末

- `文;  // <意味>` の形。値の意味・暗黙の前提・境界値に付ける。
- 連続する関連行は `//` の桁を揃える。

```cpp
sb.total_blocks = 0x01FFFFFF;  // 32bitぎりぎりの大きい値
sb.data_blocks = 65535;        // 上限値
```

```cpp
in.mode = ROOT_MODE;       // ルートディレクトリの権限(0o040777)
in.nlink = ROOT_NLINK;     // ルートの参照数は2
in.size = FS_BLOCK_SIZE;   // 初期サイズはディレクトリ容量1クラスタ分
```

### 3.7 リテラル配列の行末

- 行末に `// <フィールド名> = <解釈>` を付けて生値と意味を対応付ける。導出過程も書いてよい。
- 特殊な意味 (phase 1 で下位32bitのみ等) は括弧や `only` で注記する。

```cpp
0x78, 0x56, 0x34, 0x12, 0x00, 0x00, 0x00, 0x00,  // atime low only
```

```cpp
0xFF, 0x41,  // mode = ROOT_MODE 0o040777 -> 0x41FF -> FF 41
```

## 4. 句読点・記号

- スタンドアロン行で完全文の場合、行末に `。`。
- 行末注記や名詞句は `。` なし (例: `// 上限値`、`// 2^30`)。
- 括弧は半角 `( )`、記号は半角 (`/` `→` など) で統一。

## 5. 空行の運用

- コメントは対象に密着させる (間に空行を入れない)。
- 論理グループ間・セクション間は空行で分離。

## 6. 例集 (serialization_test.cpp より)

```cpp
// 領域 p の先頭 n バイトが全て 0 か
static bool region_all_zero(const uint8_t* p, uint32_t n) {
    for (uint32_t i = 0; i < n; ++i) {
        if (p[i] != 0) {
            return false;
        }
    }
    return true;
}

// 予約領域は encode で必ず0になり、decode で非ゼロなら ECORRUPT になるか
static void test_reserved_zero() {
    uint8_t raw[SUPERBLOCK_SIZE];
    const uint32_t superblock_reserved_positions[] = {
        44, (44 + SUPERBLOCK_SIZE - 1) / 2, SUPERBLOCK_SIZE - 1,
    };

    // 予約領域の先頭・中央・末尾を個別に壊して検出する。
    for (uint32_t pos : superblock_reserved_positions) {
        superblock_encode(make_superblock(), raw);
        raw[pos] = 0x01;
        superblock_t sb;
        CHECK(superblock_decode(raw, &sb) == FsStatus::ECORRUPT);
    }
}
```