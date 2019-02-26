/*
�R���}��؂��cvs�����荞�ނƂ��̒���

�ʏ�R���}�ƃR���}�̊Ԃɒl��������Ă��Ȃ��ꍇ��0�ƌ��Ȃ��܂��D
�]���āA

,3,5,2,6,
,6,5,4,1,
,3,3,4,2,
,5,5,6,7,

�̂悤�ȋL�q�������

0 3 5 2 6 0
0 6 5 4 1 0
0 3 3 4 2 0
0 5 5 6 7 0

�Ƃ��Ĉ����܂��D

�ŏ��̃R���}�𖳌��ɂ���ꍇ�ɂ�
#define FIRST_COMMA���L�q���Ă�������
�Ō�̃R���}�𖳌��ɂ���ꍇ�ɂ�
#define END_COMMA���L�q���Ă�������
*/
#pragma once
#ifndef LOAD_MATRIX_H
#define LOAD_MATRIX_H

#include <stdio.h>
typedef struct MATRIX{
	double **val;
	int col_num;
	int row_num;
} MATRIX;

typedef struct D_MTX{
	double **val;
	int col_num;
	int row_num;
}D_MTX;


struct MATRIX *Load_Matrix(FILE *fp);
struct MATRIX *Load_Matrix_big(long int s, long int d, FILE *fp);
struct MATRIX *MATRIX_new(int row, int col);

void Copy_MATRIX(struct MATRIX *m1, struct MATRIX *m2);
void Copy_D_MTX(struct D_MTX *d1, struct D_MTX *d2);
void DeleteMatrix(struct MATRIX *mtx);

struct D_MTX *D_MTX_new(int row, int col);
void DeleteD_MTX(struct D_MTX *mtx);
void print_Matrix(struct MATRIX *mtx);
#endif /* LOAD_MATRIX_H */