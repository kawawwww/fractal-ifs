# fractal-ifs

[English](README.md) | 日本語

IFS（Iterated Function System：反復関数系）によるフラクタル画像を、パラメータのランダム探索で生成するコードです。
FractalDB のパラメータ探索（[hirokatsukataoka16/FractalDB-Pretrained-ResNet-PyTorch](https://github.com/hirokatsukataoka16/FractalDB-Pretrained-ResNet-PyTorch)）をもとにしています。

このリポジトリには、以下の論文で使用したフラクタル画像生成コードが含まれています。

> R. Kawaguchi, T. Minagawa, K. Hori, T. Hashimoto,
> "Application of deep learning with fractal images to sparse-view CT,"
> *International Journal of Computer Assisted Radiology and Surgery*, 2025.
> https://doi.org/10.1007/s11548-025-03378-1

論文では、Python 版で 20,000 枚のフラクタル画像を生成しました（`rate = 0.2`、`category = 20000`、`numof_point = 100000`、seed 1）。
事前学習には 256×256 画素にリサイズして使いました。

実装は2つあります。

| ディレクトリ | 言語 | 状態 | 出力 |
|---|---|---|---|
| [`python/`](python) | Python | **論文で使用**（変更なし） | 8bit PNG 画像 ＋ IFS パラメータ（CSV） |
| [`c/`](c) | C | 開発中（論文では未使用） | 512×512 の float32 raw 画像（[-1000, 4000] にスケーリング） |

## アルゴリズム

各カテゴリについて、次の手順で画像を作ります。

1. アフィン変換の数 `N ∈ {2, …, 7}` を乱数で決める。
2. 各変換のパラメータ `a, b, c, d, e, f` を `U(-1, 1)` から引く。
   各変換は `(x, y) → (a x + b y + e, c x + d y + f)` です。
3. 各変換の選択確率を `|a d − b c|` とし、合計が 1 になるよう正規化する。
4. `(0, 0)` から始めて、カオスゲームを `numof_point`（= 100,000）点ぶん反復する。
5. 点列を 6 画素の余白付きで 512×512 の画像にスケーリングし、画素ごとのヒット数を数える。
6. 充填率（非ゼロ画素数 / 全画素数）が `rate`（= 0.2）以上なら採用する。
   満たさなければ破棄して、パラメータを引き直す。

乱数シードはどちらの実装も 1 です。

## Python 版（論文版）

必要なもの：Python 3、`numpy`、`opencv-python`、`Pillow`、`matplotlib`

```bash
cd python
jupyter notebook ifs_search.ipynb
```

`ifs_search.ipynb` は `new3.py` の `ifs_function` クラスを読み込みます。
`rate`、`category`、`numof_point`、`save_dir` は notebook の先頭で変更できます。
出力は次のとおりです。

- `rate{rate}_category{category}/NNNNN.png`（フラクタル画像）
- `csv_rate{rate}_category{category}/NNNNN.csv`（IFS パラメータ。1行が1変換で `a,b,c,d,e,f,p`）

## C 版（開発中）

```bash
cd c
gcc -O2 -o ifs_fractal ifs_fractal_v1.c -lm
mkdir test
./ifs_fractal
```

MSVC の場合は `cl /O2 /utf-8 ifs_fractal_v1.c` です。
`mersenne_twister.c` は `ifs_fractal_v1.c` から直接 include されるので、別にコンパイルしないでください。

設定は `ifs_fractal_v1.c` の先頭にあるグローバル変数（`rate`、`category`、`numof_point`、`save_dir`）で変更します。
出力先のディレクトリは、実行する前に作っておく必要があります。
画像は `save_dir/{n}.img` として保存されます。形式は 512×512、リトルエンディアンの `float32` で、ヘッダはありません。
画素ごとのヒット数を `[0, max]` から `[-1000, 4000]` へ線形にスケーリングしています。

## 2つの実装の違い

アルゴリズムは同じですが、生成される画像は一致しません。

- **乱数**：どちらも MT19937（seed 1）ですが、乱数の消費の仕方が違います。
  numpy は 53bit 精度の double と `randint` を使い、C は 32bit 精度の `genrand_real2` を使います。
- **充填率**：Python は画像を 8bit に量子化した**後で**非ゼロ画素を数えるため、ヒット数の少ない画素は数に入りません。
  C はヒットが1回以上ある画素をすべて数えます。
- **発散したとき**：Python は NaN の点（とその手前100点）を削除します。
  C は |x| か |y| が 1e7 を超えたら、そのパラメータセットを破棄します。
- **画素座標**：Python は切り捨て、C は四捨五入です。

## 既知の問題（Python 版。論文のときのまま残しています）

- `new3.py` の `__rescale` は、最小値が負のときだけ最小値を引きます。
  座標がすべて正だと `min` を引かずに `(max − min)` で割るので、フラクタルの位置がずれたり、一部が画像の外に出たりすることがあります。
  C 版は `(x − min) / (max − min)` で正しく正規化しています。

## 引用

```bibtex
@article{kawaguchi2025fractal,
  title   = {Application of deep learning with fractal images to sparse-view {CT}},
  author  = {Kawaguchi, Ren and Minagawa, Tomoya and Hori, Kensuke and Hashimoto, Takeyuki},
  journal = {International Journal of Computer Assisted Radiology and Surgery},
  year    = {2025},
  doi     = {10.1007/s11548-025-03378-1}
}
```

## ライセンス

MIT License です。詳しくは [LICENSE](LICENSE) を見てください。
Python のコードは FractalDB（© 2020 AIST、MIT License）をもとにしています。
`c/mersenne_twister.c` は Matsumoto・Nishimura による mt19937ar.c をもとにしています（BSD 3-Clause License。ライセンス表記はファイル内にあります）。
