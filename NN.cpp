
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "NN.h"

#define SIGMOID 1
#define SIGMOID_APPROX 2
#define LEAKY_RELU 3


struct NN *NN_new(Config *cfg) {
	NN *nn = (NN *)calloc(1, sizeof(NN));

	nn->wi = (double**)calloc(cfg->hidden_num, sizeof(double*));
	for (int h = 0; h < cfg->hidden_num; ++h)
		nn->wi[h] = (double*)calloc(cfg->input_num, sizeof(double));

	nn->bi = (double*)calloc(cfg->hidden_num, sizeof(double));

	/*隠れ層ー隠れ層*/
	if (cfg->hidden_layer_num > 1) {
		nn->wh = (double***)calloc(cfg->hidden_layer_num - 1, sizeof(double**));
		for (int layer = 0; layer < cfg->hidden_layer_num - 1; layer++) {
			nn->wh[layer] = (double**)calloc(cfg->hidden_num, sizeof(double*));
			for (int h2 = 0; h2 < cfg->hidden_num; h2++)
				nn->wh[layer][h2] = (double*)calloc(cfg->hidden_num, sizeof(double));

		}
		nn->bh = (double**)calloc(cfg->hidden_layer_num - 1, sizeof(double*));
		for (int layer = 0; layer < cfg->hidden_layer_num - 1; ++layer) {
			nn->bh[layer] = (double*)calloc(cfg->hidden_num, sizeof(double));
		}
	}

	/*隠れ層ー出力層*/
	nn->wo = (double*)calloc(cfg->hidden_num + 1, sizeof(double));
	nn->bo = drand();



	/*学習に使う　NN内部の出力アレイ*/
	nn->hi = (double**)calloc(cfg->hidden_layer_num, sizeof(double*));
	for (int layer = 0; layer < cfg->hidden_layer_num; ++layer)
		nn->hi[layer] = (double*)calloc(cfg->hidden_num, sizeof(double));

	/*学習に使う　delta*/
	nn->delta_h = (double**)calloc(cfg->hidden_layer_num, sizeof(double*));
	for (int layer = 0; layer < cfg->hidden_layer_num; ++layer)
		nn->delta_h[layer] = (double*)calloc(cfg->hidden_num, sizeof(double));

	nn->delta_o = 0.0;
	return nn;
}

void NN_copy(NN *nn_src, NN *nn, Config *cfg) {

	for (int h = 0; h < cfg->hidden_num; ++h)
		for (int i = 0; i < cfg->input_num; ++i)
			nn->wi[h][i] = nn_src->wi[h][i];
	for (int h = 0; h < cfg->hidden_num; h++)
		nn->bi[h] = nn_src->bi[h];


	/*隠れ層ー隠れ層*/
	if (cfg->hidden_layer_num > 1) {
		for (int layer = 0; layer < cfg->hidden_layer_num - 1; layer++)
			for (int h2 = 0; h2 < cfg->hidden_num; h2++)
				for (int h1 = 0; h1 < cfg->hidden_num; h1++)
					nn->wh[layer][h2][h1] = nn_src->wh[layer][h2][h1];//-0.0001;//
		for (int layer = 0; layer < cfg->hidden_layer_num - 1; ++layer)
			for (int h = 0; h < cfg->hidden_num; ++h)
				nn->bh[layer][h] = nn_src->bh[layer][h];
	}

	/*隠れ層ー出力層*/
	for (int i = 0; i < cfg->hidden_num; ++i)
		nn->wo[i] = nn_src->wo[i];
	nn->bo = nn_src->bo;
}

void NN_delete(NN *nn, Config *cfg) {
	for (int h = 0; h < cfg->hidden_num; ++h)
		free(nn->wi[h]);
	free(nn->wi);
	free(nn->bi);
	for (int layer = 0; layer < cfg->hidden_layer_num - 1; layer++) {
		for (int h2 = 0; h2 < cfg->hidden_num; h2++)
			free(nn->wh[layer][h2]);
		free(nn->wh[layer]);
	}
	free(nn->wh);
	free(nn->wo);

	for (int layer = 0; layer < cfg->hidden_layer_num; ++layer)
		free(nn->hi[layer]);
	free(nn->hi);

	for (int layer = 0; layer < cfg->hidden_layer_num; ++layer)
		free(nn->delta_h[layer]);
	free(nn->delta_h);
}

void init_NN(NN *nn, Config *cfg) {

	/*入力層ー隠れ層*/
	for (int h = 0; h < cfg->hidden_num; ++h)
		for (int i = 0; i < cfg->input_num; ++i)
			nn->wi[h][i] = drand();
	for (int h = 0; h < cfg->hidden_num; h++)
		nn->bi[h] = drand();

	/*隠れ層ー隠れ層*/
	if (cfg->hidden_layer_num > 1) {
		for (int layer = 0; layer < cfg->hidden_layer_num - 1; layer++)
			for (int h2 = 0; h2 < cfg->hidden_num; h2++)
				for (int h1 = 0; h1 < cfg->hidden_num; h1++)
					nn->wh[layer][h2][h1] = drand();//-0.0001;//
		for (int layer = 0; layer < cfg->hidden_layer_num - 1; ++layer)
			for (int h = 0; h < cfg->hidden_num; ++h)
				nn->bh[layer][h] = drand();
	}

	/*隠れ層ー出力層*/
	for (int i = 0; i < cfg->hidden_num; ++i)
		nn->wo[i] = drand();
	nn->bo = drand();
}


/**********************/
/*  forward()関数     */
/*  順方向の計算      */
/**********************/
double forward(NN *nn, double *x, Config *cfg)
{
	double u;/*重み付き和の計算*/
	double o;/*出力の計算*/

			 /*入力層ー隠れ層0の計算*/
	for (int h = 0; h < cfg->hidden_num; h++) {
		u = 0;/*重み付き和を求める*/
		for (int i = 0; i < cfg->input_num; ++i)
			u += x[i] * nn->wi[h][i];
		u -= nn->bi[h];/*しきい値の処理*/
		switch (cfg->hidden_neuron_type) {
		case SIGMOID:
			nn->hi[0][h] = sigmoid(u);
			break;
		case SIGMOID_APPROX:
			nn->hi[0][h] = s_approx(u);
			break;
		case LEAKY_RELU:
			nn->hi[0][h] = LeakyReLU(u, cfg);
			break;
		}
	}

	if (cfg->hidden_layer_num > 1) {
		/*隠れ層0-隠れ層kの計算*/
		for (int layer = 0; layer < cfg->hidden_layer_num - 1; layer++) {
			for (int h2 = 0; h2 < cfg->hidden_num; ++h2) {
				u = 0;/*重み付き和を求める*/
				for (int h1 = 0; h1 < cfg->hidden_num; ++h1)
					u += nn->hi[layer][h2] * nn->wh[layer][h2][h1];
				u -= nn->bh[layer][h2];/*しきい値の処理*/
				switch (cfg->hidden_neuron_type) {
				case SIGMOID:
					nn->hi[layer + 1][h2] = sigmoid(u);
					break;
				case SIGMOID_APPROX:
					nn->hi[layer + 1][h2] = s_approx(u);
					break;
				case LEAKY_RELU:
					nn->hi[layer + 1][h2] = LeakyReLU(u, cfg);
					break;
				}
			}
		}
	}
	/*出力oの計算*/
	o = 0;
	for (int h = 0; h < cfg->hidden_num; ++h)
		o += nn->hi[cfg->hidden_layer_num - 1][h] * nn->wo[h];
	o -= nn->bo;/*しきい値の処理*/
			   //printf("o-%lf\n", o);

	switch (cfg->output_neuron_type) {
	case SIGMOID:
		return sigmoid(o);
	case SIGMOID_APPROX:
		return s_approx(o);
	case LEAKY_RELU:
		return LeakyReLU(o, cfg);
	}
	exit(11);
	return 0;
}

/**********************/
/*  olearn()関数      */
/*  出力層の重み学習  */
/**********************/
/**/
void olearn(NN *nn, double target_data, double o, Config *cfg)
{
	switch (cfg->output_neuron_type) {
	case SIGMOID:
		nn->delta_o = (target_data - o) * diff_s(o);
		break;
	case SIGMOID_APPROX:
		nn->delta_o = (target_data - o) * diff_s_approx(o);
		break;
	case LEAKY_RELU:
		nn->delta_o = (target_data - o) * diff_LeakyReLU(o, cfg);
		break;
	}

	for (int h = 0; h < cfg->hidden_num; ++h) {
		nn->wo[h] += nn->hi[cfg->hidden_layer_num - 1][h] * nn->delta_o;/*重みの学習*/
	}
	nn->bo += (-1.0) * nn->delta_o;/*しきい値の学習*/
}

/**********************/
/*  hlearn()関数      */
/*  中間層の重み学習  */
/**********************/
/*隠れ層数2以上のみ*/
void hlearn(NN *nn, double target_data, double o, Config *cfg)
{
	if (cfg->hidden_layer_num <= 1)
		exit(21);

	for (int h2 = 0; h2 < cfg->hidden_num; h2++) {
		switch (cfg->hidden_neuron_type) {
		case SIGMOID:
			nn->delta_h[cfg->hidden_layer_num - 1][h2] = nn->delta_o * nn->wo[h2] * diff_s(nn->hi[cfg->hidden_layer_num - 1][h2]);
			break;
		case SIGMOID_APPROX:
			nn->delta_h[cfg->hidden_layer_num - 1][h2] = nn->delta_o * nn->wo[h2] * diff_s_approx(nn->hi[cfg->hidden_layer_num - 1][h2]);
			break;
		case LEAKY_RELU:
			nn->delta_h[cfg->hidden_layer_num - 1][h2] = nn->delta_o * nn->wo[h2] * diff_LeakyReLU(nn->hi[cfg->hidden_layer_num - 1][h2], cfg);
			break;
		}
		for (int h1 = 0; h1 < cfg->hidden_num; ++h1)
			nn->wh[cfg->hidden_layer_num - 2][h2][h1] += nn->hi[cfg->hidden_layer_num - 2][h2] * nn->delta_h[cfg->hidden_layer_num - 1][h2];
		nn->bh[cfg->hidden_layer_num - 2][h2] += (-1.0) * nn->delta_h[cfg->hidden_layer_num - 1][h2];/*しきい値の学習*/
	}

	//for layer

		/*現在hidden_layer2以下のみ対応*/
}

void ilearn(NN *nn, double *capa, double target_data, double o, Config *cfg) {


	if (cfg->hidden_layer_num == 1) {
		for (int h = 0; h < cfg->hidden_num; h++) {
			switch (cfg->hidden_neuron_type) {
			case SIGMOID:
				nn->delta_h[0][h] = nn->delta_o * nn->wo[h] * diff_s(nn->hi[0][h]);
				break;
			case SIGMOID_APPROX:
				nn->delta_h[0][h] = nn->delta_o * nn->wo[h] * diff_s_approx(nn->hi[0][h]);
				break;
			case LEAKY_RELU:
				nn->delta_h[0][h] = nn->delta_o * nn->wo[h] * diff_LeakyReLU(nn->hi[0][h], cfg);
				break;
			}

			for (int i = 0; i < cfg->input_num; ++i)
				nn->wi[h][i] += capa[i] * nn->delta_h[0][h];
			nn->bi[h] += (-1.0) * nn->delta_h[0][h];/*しきい値の学習*/
		}
	}
	else {
		for (int h1 = 0; h1 < cfg->hidden_num; h1++) {/*中間層の各セルjを対象*/

			switch (cfg->hidden_neuron_type) {
			case SIGMOID:
				for (int h2 = 0; h2 < cfg->hidden_num; h2++)
					nn->delta_h[0][h1] += nn->delta_h[1][h2] * nn->wh[0][h2][h1] * diff_s(nn->hi[0][h1]);
				break;
			case SIGMOID_APPROX:
				for (int h2 = 0; h2 < cfg->hidden_num; h2++)
					nn->delta_h[0][h1] += nn->delta_h[1][h2] * nn->wh[0][h2][h1] * diff_s_approx(nn->hi[0][h1]);
				break;
			case LEAKY_RELU:
				for (int h2 = 0; h2 < cfg->hidden_num; h2++)
					nn->delta_h[0][h1] += nn->delta_h[1][h2] * nn->wh[0][h2][h1] * diff_LeakyReLU(nn->hi[0][h1], cfg);
				break;
			}
			for (int i = 0; i < cfg->input_num; ++i)
				nn->wi[h1][i] += capa[i] * nn->delta_h[0][h1];
			nn->bi[h1] += (-1.0) * nn->delta_h[0][h1];/*しきい値の学習*/
		}
	}
}

void Delete_weight(double ***wh, double *wo, Config *cfg) {
	for (int layer = 0; layer < cfg->hidden_layer_num; layer++) {
		for (int i = 0; i < cfg->hidden_num; i++)
			free(wh[layer][i]);
		free(wh[layer]);
	}
	free(wh);
	free(wo);
}

double ReLU(double u)
{
	if (u >= 0)
		return u;
	return 0;
}

double LeakyReLU(double u, Config *cfg)
{
	if (u >= 0)
		return u;
	return u * cfg->leak;
}

double diff_LeakyReLU(double u, Config *cfg) {
	if (u >= 0)
		return 1;
	return cfg->leak;
}

double sigmoid(double u)
{
	return 1.0 / (1.0 + exp(-u));
}

double diff_s(double u)
{
	return u * (1 - u);
}

double s_approx(double u)
{
	if (u < -1)
		return -1;
	else if (u > 1)
		return 1;
	return u;
}

double diff_s_approx(double u)
{
	if (u == -1 || u == 1)
		return 0;
	return 1;
}


/*************************/
/* drand()関数           */
/*-1から1の間の乱数を生成*/
/*************************/
double drand(void)
{
	double rndno;/*生成した乱数*/

	while ((rndno = (double)rand() / RAND_MAX) == 1.0);
	rndno = rndno * 2 - 1;/*-1から1の間の乱数を生成*/
	return rndno;
}

void output_weight(NN *nn, Config *cfg) {

	/*入力層ー隠れ層*/
	for (int h = 0; h < cfg->hidden_num; ++h){
		for (int i = 0; i < cfg->input_num; ++i) 
			fprintf(cfg->weight_i_file, "%.12lf, ", nn->wi[h][i]);
			fprintf(cfg->weight_i_file, "\n");

		}


	for (int h = 0; h < cfg->hidden_num; h++)
		fprintf(cfg->weight_i_file, "%.12lf, ", nn->bi[h]);

	/*隠れ層ー隠れ層*/
	if (cfg->hidden_layer_num > 1) {
		for (int layer = 0; layer < cfg->hidden_layer_num - 1; layer++) {
			for (int h2 = 0; h2 < cfg->hidden_num; h2++) {
				for (int h1 = 0; h1 < cfg->hidden_num; h1++)
					fprintf(cfg->weight_h_file, "%.12lf, ", nn->wh[layer][h2][h1]);
				fprintf(cfg->weight_h_file, "\n");
			}
			for (int h = 0; h < cfg->hidden_num; ++h)
				fprintf(cfg->weight_h_file, "%.12lf, ", nn->bh[layer][h]);
			fprintf(cfg->weight_h_file, "\n");
		}
	}

	/*隠れ層ー出力層*/
	for (int i = 0; i < cfg->hidden_num; ++i)
		fprintf(cfg->weight_o_file, "%.12lf, ", nn->wo[i]);
	fprintf(cfg->weight_o_file, "%.12lf\n", nn->bo);
	
	//fclose(cfg->weight_h_file);
	//fclose(cfg->weight_i_file);
	//fclose(cfg->weight_o_file);

}


void read_weight(NN *nn, MATRIX *i_table, MATRIX *o_table, Config *cfg) {

	for (int h = 0; h < cfg->hidden_num; ++h) {
		for (int i = 0; i < cfg->input_num; ++i) {
			nn->wi[h][i] = i_table->val[h][i];
			printf("%lf,", nn->wi[h][i]);
		}

		printf("\n");
	}
	for (int h = 0; h < cfg->hidden_num; h++)
		nn->bi[h] = i_table->val[cfg->hidden_num][h];


	/*隠れ層ー出力層*/
	for (int i = 0; i < cfg->hidden_num; ++i)
		nn->wo[i] = o_table->val[0][i];
	nn->bo = o_table->val[0][cfg->hidden_num];

	//int index = 0;
	//for (int layer = 0; layer < cfg->hidden_layer_num; layer++) {
	//	for (int i = 0; i < cfg->hidden_num; i++) {
	//		for (int j = 0; j < cfg->hidden_num + 1; j++) {
	//			wh[layer][i][j] = weight_table->val[index][j];
	//		}
	//		index++;
	//	}
	//	index++;
	//}
	//for (int j = 0; j < cfg->hidden_num + 1; j++)
	//	wo[j] = weight_table->val[index][j];


}
