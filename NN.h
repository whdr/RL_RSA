#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "load_matrix.h"
#include "config.h"

typedef struct NN {
	//double e[MAXNO][cfg->input_no + 1];/*学習データセット*/
	int i, sim;/*繰り返しの制御*/
	int n_of_e;/*学習データの個数*/
	double **wi;
	double ***wh;
	double *wo;
	double *bi;
	double **bh;
	double bo;

	double **hi;/*学習に使う　NN内部の出力アレイ*/	/*誤差伝搬による学習で利用する順方向のニューロンへの入力保持用配列*/

	double delta_o;
	double **delta_h;
}NN;

/*重み*/
struct NN *NN_new(Config *cfg);
void NN_copy(NN *nn_src, NN *nn, Config *cfg);
void NN_delete(NN *nn, Config *cfg);
void init_NN(NN *nn, Config *cfg);
double forward(NN *nn, double *e, Config *cfg); /*順方向の計算*/
void olearn(NN *nn, double target_data, double o, Config *cfg); /*出力層の重みの学習*/
void hlearn(NN *nn, double t, double o, Config *cfg); /*中間層の重みの学習*/
void ilearn(NN *nn, double *e, double t, double o, Config *cfg); /*中間層の重みの学習*/
void Delete_weight(double ***wh, double *wo, Config *cfg); /*結果の出力*/

											  /*ニューロン*/
double sigmoid(double u);
double diff_s(double u);
double s_approx(double u);
double diff_s_approx(double u);
double ReLU(double u);
double LeakyReLU(double u, Config *cfg);
double diff_LeakyReLU(double u, Config *cfg);
void output_weight(NN *nn, Config *cfg);
void read_weight(NN *nn, MATRIX *i_table, MATRIX *o_table, Config *cfg);

/*その他*/
double drand(void);/*-1から1の間の乱数を生成 */
