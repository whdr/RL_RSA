#if 0
#include "memory_leak.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "misc.h"
#include "load_matrix.h"
#include "config.h"
/* #include "memory_leak.h" */

#define MAX_ROW	150
#define MAX_COL 20000

//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>

/* topology.txt����[���Q�����z��ɏ����o�� */
MATRIX *Load_Matrix(FILE *fp){
	/*�ǂݍ��ݗp�̕ϐ�*/
	char temp[4096];

	struct MATRIX read_mtx;
	struct MATRIX *out_mtx = (MATRIX *)calloc(1, sizeof(MATRIX));

	int i,j=0;
	int m=0,n=0;
	int row_num=0,col_num=0;


	read_mtx.val = (double **)alloc2D(sizeof(double), MAX_COL , MAX_ROW);

	/*�����N�R�X�g�̓ǂݏo��*/
	while(fgets(temp,2023,fp)!=NULL){
		for(i=0;i<2023;i++){
			if((temp[i]==',')||(temp[i]=='\n')||((temp[i]=='\r')&&(temp[i+1]=='\n'))){
				/* �����A���s�E�R���}������΁A�����܂ł̒l��read_mtx.val�Ɋi�[ */
				read_mtx.val[m][n]=strtod(&temp[i-j], NULL);
				j=0;
				/*���̗�v�f�֑J��*/
				n++;

				/* �����A�R���}�Ŗ�����΁A���s�Ƃ݂Ȃ��āA���f */
				if(temp[i]!=','){
					i=2024;
				}
			}
			else if((('0'<=temp[i])&&(temp[i]<='9'))||(temp[i]=='.')||(temp[i]=='-')){
				/* ���l��󔒂╉�����ł�������A���̕�����ǂ� */
				j++;
			}
		}
		/*���̍s�֑J��*/
		m++;
		/*��(�m�[�h��)���i�[*/
		if (row_num < n)
		row_num=n;
		if (col_num < m)
			col_num=m;
		/*��v�f��擪�֑J��*/
		n=0;
	}

	read_mtx.row_num = row_num;
	read_mtx.col_num = col_num;

	out_mtx->row_num = read_mtx.row_num;
	out_mtx->col_num = read_mtx.col_num;

	out_mtx->val = (double **)alloc2D(sizeof(double), out_mtx->col_num , out_mtx->row_num);

	for(i=0;i<read_mtx.col_num;i++){
		for(j=0;j<read_mtx.row_num;j++){
			out_mtx->val[i][j]=read_mtx.val[i][j];
		}
	}

	free2D((void **)read_mtx.val, MAX_COL);

	return out_mtx;
}


/* topology.txt����[���Q�����z��ɏ����o�� */
MATRIX *Load_Matrix_big(long int s, long int d, FILE *fp) {
	/*�ǂݍ��ݗp�̕ϐ�*/
	char temp[4096];

	struct MATRIX read_mtx;
	struct MATRIX *out_mtx = (MATRIX *)calloc(1, sizeof(MATRIX));

	int i, j = 0;
	long int m = 0, n = 0;
	int row_num = 0, col_num = 0;


	read_mtx.val = (double **)alloc2D(sizeof(double), MAX_COL, MAX_ROW);

	/*�����N�R�X�g�̓ǂݏo��*/
	while (fgets(temp, 1023, fp) != NULL) {
		if (m > d)
			break;
		if (m < s)
			continue;
		for (i = 0; i<1023; i++) {
			if ((temp[i] == ',') || (temp[i] == '\n') || ((temp[i] == '\r') && (temp[i + 1] == '\n'))) {
				/* �����A���s�E�R���}������΁A�����܂ł̒l��read_mtx.val�Ɋi�[ */
				read_mtx.val[m - s][n] = strtod(&temp[i - j], NULL);
				j = 0;
				/*���̗�v�f�֑J��*/
				n++;
				/* �����A�R���}�Ŗ�����΁A���s�Ƃ݂Ȃ��āA���f */
				if (temp[i] != ',') {
					i = 1024;
				}
			}
			else if ((('0' <= temp[i]) && (temp[i] <= '9')) || (temp[i] == ' ') || (temp[i] == '-')) {
				/* ���l��󔒂╉�����ł�������A���̕�����ǂ� */
				j++;
			}
		}
		/*���̍s�֑J��*/
		m++;
		/*��(�m�[�h��)���i�[*/
		row_num = n;
		col_num = m;
		/*��v�f��擪�֑J��*/
		n = 0;
	}

	read_mtx.row_num = row_num;
	read_mtx.col_num = col_num;

	out_mtx->row_num = read_mtx.row_num;
	out_mtx->col_num = read_mtx.col_num;

	out_mtx->val = (double **)alloc2D(sizeof(double), out_mtx->col_num, out_mtx->row_num);

	for (i = 0; i<read_mtx.col_num; i++) {
		for (j = 0; j<read_mtx.row_num; j++) {
			out_mtx->val[i][j] = read_mtx.val[i][j];
		}
	}

	//free2D((void **)read_mtx.val, MAX_COL);

	return out_mtx;
}
MATRIX *MATRIX_new(int row, int col)
{
	MATRIX *mtx;
	mtx = (MATRIX *)calloc(1, sizeof(MATRIX));
	mtx->val = (double **)alloc2D(sizeof(double), row, col);
	mtx->row_num = row;
	mtx->col_num = col;

	return mtx;
}

D_MTX *D_MTX_new(int row, int col)
{
	D_MTX *d_mtx;
	d_mtx = (D_MTX *)calloc(1, sizeof(D_MTX));
	d_mtx->val = (double **)alloc2D(sizeof(double), row, col);
	d_mtx->row_num = row;
	d_mtx->col_num = col;

	return d_mtx;
}

/* m2��m1�ɃR�s�[ */
void Copy_MATRIX(struct MATRIX *m1, struct MATRIX *m2)
{

	int i,j;

	m1->col_num = m2->col_num;
	m1->row_num = m2->row_num;

	for(i=0; i<m1->col_num; i++){
		for(j=0; j<m1->col_num; j++){

			m1->val[i][j] = m2->val[i][j];
		}
	}

}

/* m2��m1�ɃR�s�[ */
void Copy_D_MTX(struct D_MTX *d1, struct D_MTX *d2)
{

	int i,j;

	d1->col_num = d2->col_num;
	d1->row_num = d2->row_num;

	for(i=0; i<d1->col_num; i++){
		for(j=0; j<d1->col_num; j++){

			d1->val[i][j] = d2->val[i][j];
		}
	}

}

/*��Еt��*/
void DeleteMatrix(struct MATRIX *mtx){
	free2D((void **)mtx->val, mtx->col_num);
	free_nullcheck(mtx);
	mtx=NULL;
}	

void DeleteD_MTX(struct D_MTX *mtx){
	free2D((void **)mtx->val, mtx->col_num);
	free_nullcheck(mtx);
}

void print_Matrix(struct MATRIX *mtx) {
	for (int col = 0; col < mtx->col_num; col++) {
		for (int row = 0; row < mtx->row_num; row++) {
			fprintf(stderr, "%.2lf ", mtx->val[col][row]);
		}
		printf("\n");
	}
}