#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mersenne_twister.c"

// 充填率
double rate = 0.2;
int    category = 40000;

// 反復回数
int    numof_point = 100000;

// 保存場所
char   save_dir[100] = "./test";

// 画像のゼロでない画素の割合算出
// float* gray;  // 画像領域
// int    size;  // 画像の画素数
double cal_pix(float* gray, int size)
{
	int     nonzero = 0; // ゼロでない画素数
	double  num_pixels;  // ゼロでない割合

	for (int i = 0; i < size; i++) {
		if (gray[i] != (float)0) nonzero++;
	}
	num_pixels = nonzero / (double)size;
	return num_pixels;
}

// 乱数画像の計算
// double* xs;         // 結果の座標列（x座標）
// double* ys;         // 結果の座標列（y座標）
// int     iteration;  // 座標列の数（繰り返し回数）
// double* function;   // パラメータ配列
// double* temp_proba; // probaの順和
// int     p_size;     // temp_probaの数（パラメータセット数）
int calculate(double *xs, double *ys, int iteration, double* function, double *select_function, int p_size)
{
	double *rand;
	double prev_x = 0;
	double prev_y = 0;
	double next_x = 0;
	double next_y = 0;

	// ランダムシードの変更
	rand = (double*)malloc((size_t)iteration * sizeof(double));
	for (int i = 0; i < iteration; i++)
		rand[i] = genrand_real2();

	// アフィン変換（シフトや回転）
	for (int i = 0; i < iteration - 1; i++) {
		for (int j = 0; j < p_size; j++) {
			if (rand[i] <= select_function[j]) {
				// 回転シフト
				next_x = prev_x * function[j * 7 + 0] + prev_y * function[j * 7 + 1] + function[j * 7 + 4];
				next_y = prev_x * function[j * 7 + 2] + prev_y * function[j * 7 + 3] + function[j * 7 + 5];
				break;
			}
		}
		xs[i] = next_x;
		ys[i] = next_y;
		prev_x = next_x;
		prev_y = next_y;
		if (fabs(prev_x) > 1e+7 || fabs(prev_y) > 1e+7) {
			printf("Error: overflow (prev_x = %f, prev_y = %f)\n", prev_x, prev_y);
			free(rand);
			return 1;
		}
	}
	free(rand);
	return 0;
}

// 乱数画像の作成
// float*  img;   // 乱数画像
// int     nx;    // 画像の幅
// int     ny;    // 乱数の高さ
// int     pad_x; // 画像の余白（x方向）
// int     pad_y; // 画像の余白（y方向）
// double* xs;    // 乱数座標列（x座標）
// double* ys;    // 乱数座標列（y座標）
void draw_point(float* img, int nx, int ny, int pad_x, int pad_y, double* xs, double* ys)
{
	double xmax, xmin, ymax, ymin;

	// 画像の初期化
	for (int i = 0; i < nx * ny; i++) {
		img[i] = 0;
	}

	// スケールの調整
	xmax = xmin = xs[0];
	ymax = ymin = ys[0];
	for (int i = 1; i < numof_point; i++) {
		if (xmax < xs[i]) xmax = xs[i];
		if (xmin > xs[i]) xmin = xs[i];
		if (ymax < ys[i]) ymax = ys[i];
		if (ymin > ys[i]) ymin = ys[i];
	}
	// 座標点の加算
	for (int i = 0; i < numof_point; i++) {
		int ix = (int)((xs[i] - xmin) / (xmax - xmin) * (nx - 2 * pad_x) + pad_x + 0.5);
		int iy = (int)((ys[i] - ymin) / (ymax - ymin) * (ny - 2 * pad_y) + pad_y + 0.5);
		if (ix >= 0 && ix < nx && iy >= 0 && iy < ny) {
			img[iy * nx + ix] += 1;
		}
	}
}

// フラクタル画像の作成
// float*  img;     // 乱数画像
// int     nx;      // 画像の幅
// int     ny;      // 乱数の高さ
// double* params;  // パラメータが格納された配列（psize*7）
// int     p_size;  // パラメータセットの個数
int generator(float* img, int nx, int ny, double* params, int p_size)
{
	double* temp_proba;
	double* xs;  // 乱数画像の座標列（x座標）
	double* ys;  // 乱数画像の座標列（y座標）
	int     pad_x = 6; // 画像の余白（x方向）
	int     pad_y = 6; // 画像の余白（y方向）

	temp_proba = (double*)malloc((size_t)p_size * sizeof(double));
	temp_proba[0] = params[6]; // 6番目がproba
	for (int i = 1; i < p_size; i++) {
		temp_proba[i] = temp_proba[i - 1] + params[i * 7 + 6]; // 6番目がproba
	}

	// 乱数座標列の作成
	xs = (double*)malloc((size_t)numof_point * sizeof(double));
	ys = (double*)malloc((size_t)numof_point * sizeof(double));
	for (int i = 0; i < numof_point; i++) {
		xs[i] = ys[i] = 0;
	}
	if (calculate(xs, ys, numof_point, params, temp_proba, p_size) == 1) {
		free(temp_proba);
		free(xs);
		free(ys);
		return 1;
	}

	// 乱数画像の作成
	draw_point(img, nx, ny, pad_x, pad_y, xs, ys);

	free(temp_proba);
	free(xs);
	free(ys);
	return 0;
}

// 画像の保存
// char*  fi;   // ファイル名
// float* img;  // 画像領域
// int    size; // 画像サイズ
void write_data(char* fi, float* img, int size)
{
	FILE* fp;
	if ((fp = fopen(fi, "wb")) == NULL) {
		fprintf(stderr, "Error: file open [%s].\n", fi);
		exit(1);
	}
	fwrite(img, sizeof(float), size, fp);
	fclose(fp);
}

int main()
{
	int     class_num = 0;  // 繰り返す数
	int     nx = 512;   // 画像の幅
	int     ny = 512;   // 画像の高さ
	float*  img;        // 画像領域
	int     param_size; // パラメータセット数
	double* params;     // パラメータ配列
	double  sum_proba = 0;
	double  prob;
	char    fi[256];    // ファイル名
	double  threshold = rate;
	double  pixels;     // 非ゼロ画素の割合

	init_genrand(1); // 乱数の初期化

	// 画像の領域確保
	img = (float*)malloc((size_t)nx * ny * sizeof(float));

	while (class_num < category) {
		param_size = (int)(6 * genrand_real2() + 2);
		if (param_size > 7) param_size = 7; // genrand_real2() は 1 を返しうるため 2〜7 に制限
		params = (double*)malloc((size_t)param_size * 7 * sizeof(double));
		sum_proba = 0;

		for (int i = 0; i < param_size * 7; i++)
			params[i] = 0;

		for (int i = 0; i < param_size; i++) {
			for (int j = 0; j < 6; j++)
				params[i * 7 + j] = 2 * genrand_real2() - 1;
			// 選択確率 = |det A| = |a*d - b*c|
			prob = fabs(params[i * 7 + 0] * params[i * 7 + 3] - params[i * 7 + 1] * params[i * 7 + 2]);
			sum_proba += prob;
			params[i * 7 + 6] = prob;
		}
		if (sum_proba == 0) {
			free(params);
			continue;
		}
		for (int i = 0; i < param_size; i++) {
			params[i * 7 + 6] /= sum_proba;
		}

		// フラクタル画像の作成
		if (generator(img, nx, ny, params, param_size) == 1) {
			free(params);
			continue;
		}

		// 画像内の非ゼロ画素の割合
		pixels = cal_pix(img, nx * ny);

		if (pixels >= threshold) {

			float max_val = 0;
			int po_max = 4000;
			int po_min = -1000;

			for (int i = 0; i < nx * ny; i++) {
				if (img[i] > max_val){
					max_val = img[i];
				}
			}

			if (max_val > 0) {
				for (int i = 0; i < nx * ny; i++) {
					img[i] = (img[i] / max_val) * (po_max - po_min) + po_min;
				}
			}

			// 画像の保存
			printf("class_num = %d\n", class_num);
			sprintf(fi, "%s/%d.img", save_dir, class_num);
			write_data(fi, img, nx * ny);
			class_num++;
		}
		else {
			printf("Error: out of threshold (pixels = %f)\n", pixels);
		}
		free(params);
	}

	free(img);
	return 0;
}
