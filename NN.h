#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "load_matrix.h"
#include "config.h"

typedef struct NN {
	//double e[MAXNO][cfg->input_no + 1];/*�w�K�f�[�^�Z�b�g*/
	int i, sim;/*�J��Ԃ��̐���*/
	int n_of_e;/*�w�K�f�[�^�̌�*/
	double **wi;
	double ***wh;
	double *wo;
	double *bi;
	double **bh;
	double bo;

	double **hi;/*�w�K�Ɏg���@NN�����̏o�̓A���C*/	/*�덷�`���ɂ��w�K�ŗ��p���鏇�����̃j���[�����ւ̓��͕ێ��p�z��*/

	double delta_o;
	double **delta_h;
}NN;

/*�d��*/
struct NN *NN_new(Config *cfg);
void NN_copy(NN *nn_src, NN *nn, Config *cfg);
void NN_delete(NN *nn, Config *cfg);
void init_NN(NN *nn, Config *cfg);
double forward(NN *nn, double *e, Config *cfg); /*�������̌v�Z*/
void olearn(NN *nn, double target_data, double o, Config *cfg); /*�o�͑w�̏d�݂̊w�K*/
void hlearn(NN *nn, double t, double o, Config *cfg); /*���ԑw�̏d�݂̊w�K*/
void ilearn(NN *nn, double *e, double t, double o, Config *cfg); /*���ԑw�̏d�݂̊w�K*/
void Delete_weight(double ***wh, double *wo, Config *cfg); /*���ʂ̏o��*/

											  /*�j���[����*/
double sigmoid(double u);
double diff_s(double u);
double s_approx(double u);
double diff_s_approx(double u);
double ReLU(double u);
double LeakyReLU(double u, Config *cfg);
double diff_LeakyReLU(double u, Config *cfg);
void output_weight(NN *nn, Config *cfg);
void read_weight(NN *nn, MATRIX *i_table, MATRIX *o_table, Config *cfg);

/*���̑�*/
double drand(void);/*-1����1�̊Ԃ̗����𐶐� */
