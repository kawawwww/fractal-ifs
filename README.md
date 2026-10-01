# fractal-ifs

English | [日本語](README.ja.md)

Random-search generation of IFS (Iterated Function System) fractal images,
based on the FractalDB parameter search
([hirokatsukataoka16/FractalDB-Pretrained-ResNet-PyTorch](https://github.com/hirokatsukataoka16/FractalDB-Pretrained-ResNet-PyTorch)).

This repository contains the fractal image generation code used in:

> R. Kawaguchi, T. Minagawa, K. Hori, T. Hashimoto,
> "Application of deep learning with fractal images to sparse-view CT,"
> *International Journal of Computer Assisted Radiology and Surgery*, 2025.
> https://doi.org/10.1007/s11548-025-03378-1

In the paper, 20,000 fractal images were generated with the Python version
(`rate = 0.2`, `category = 20000`, `numof_point = 100000`, seed 1) and resized to 256×256 pixels
for pre-training.

The repository has two implementations:

| Directory | Language | Status | Output |
|---|---|---|---|
| [`python/`](python) | Python | **Used in the paper** (kept unchanged) | 8-bit PNG images + IFS parameters (CSV) |
| [`c/`](c) | C | Under development (not used in the paper) | 512×512 float32 raw images scaled to [-1000, 4000] |

## Algorithm

For each category:

1. Draw the number of affine maps `N ∈ {2, …, 7}`.
2. Draw the map parameters `a, b, c, d, e, f ~ U(-1, 1)`.
   Each map is `(x, y) → (a x + b y + e, c x + d y + f)`.
3. Set the selection probability of each map to `|a d − b c|`, normalized to sum to 1.
4. Start at `(0, 0)` and iterate the chaos game for `numof_point` (= 100,000) points.
5. Rescale the points to a 512×512 image with a 6-pixel margin, and count the hits per pixel.
6. Keep the image only if the filling rate (non-zero pixels / all pixels) is at least `rate` (= 0.2).
   Otherwise discard it and draw new parameters.

The random seed is 1 in both implementations.

## Python (paper version)

Requirements: Python 3, `numpy`, `opencv-python`, `Pillow`, `matplotlib`.

```bash
cd python
jupyter notebook ifs_search.ipynb
```

`ifs_search.ipynb` imports the `ifs_function` class from `new3.py`.
Edit `rate`, `category`, `numof_point` and `save_dir` at the top of the notebook.
Output:

- `rate{rate}_category{category}/NNNNN.png` (fractal images)
- `csv_rate{rate}_category{category}/NNNNN.csv` (IFS parameters, one row per map: `a,b,c,d,e,f,p`)

## C (under development)

```bash
cd c
gcc -O2 -o ifs_fractal ifs_fractal_v1.c -lm
mkdir test
./ifs_fractal
```

With MSVC: `cl /O2 /utf-8 ifs_fractal_v1.c`.
`mersenne_twister.c` is included directly by `ifs_fractal_v1.c`, so don't compile it separately.

Edit the global variables (`rate`, `category`, `numof_point`, `save_dir`) at the top of
`ifs_fractal_v1.c`. The output directory must exist before you run the program.
Each image is written as `save_dir/{n}.img`: 512×512 little-endian `float32`, with no header.
The per-pixel hit counts are linearly scaled from `[0, max]` to `[-1000, 4000]`.

## Differences between the two implementations

The two implementations follow the same algorithm, but they don't produce identical images:

- **Random numbers**: both use MT19937 with seed 1, but they consume the random numbers differently
  (numpy uses 53-bit doubles and `randint`; the C code uses 32-bit `genrand_real2`).
- **Filling rate**: Python counts non-zero pixels *after* quantizing the image to 8 bits,
  so pixels with low hit counts are excluded. C counts all pixels with at least one hit.
- **Divergence**: Python removes NaN points (and the 100 points before them).
  C discards the parameter set when |x| or |y| exceeds 1e7.
- **Pixel coordinates**: Python truncates; C rounds to the nearest pixel.

## Known issues (Python, kept as in the paper)

- In `new3.py` (`__rescale`), the minimum is subtracted only when it is negative. When all coordinates
  are positive, they are divided by `(max − min)` without subtracting `min`, so the fractal can be
  shifted or fall partly outside the image. The C version normalizes with `(x − min) / (max − min)`.

## Citation

```bibtex
@article{kawaguchi2025fractal,
  title   = {Application of deep learning with fractal images to sparse-view {CT}},
  author  = {Kawaguchi, Ren and Minagawa, Tomoya and Hori, Kensuke and Hashimoto, Takeyuki},
  journal = {International Journal of Computer Assisted Radiology and Surgery},
  year    = {2025},
  doi     = {10.1007/s11548-025-03378-1}
}
```

## License

MIT License. See [LICENSE](LICENSE).
The Python code is derived from FractalDB (© 2020 AIST, MIT License).
`c/mersenne_twister.c` is derived from mt19937ar.c by M. Matsumoto and T. Nishimura (BSD 3-Clause License;
the notice is in the file).
