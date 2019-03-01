#pragma once
#pragma once
#pragma once
#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>




// TODO: �v���O�����ɕK�v�Ȓǉ��w�b�_�[�������ŎQ�Ƃ��Ă�������

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "misc.h"
#include "config.h"


/*���I�����p�\����*/
typedef struct Demand {/*���I�p�g���p�X*/
	int start_t;
	int end_t;
	short src;
	short dst;
	int FiberID;
	int freq;
	int assign;/* 0:���� 1:���蓖�� 2: �폜�ς݁@5;�u���b�L���O�ɂ�芄�蓖�Ď��s�@*/
	
	int route_index;

	struct Demand *next;
	struct Demand *before;
} Demand;

typedef struct DemandList {
	long dmnd_num;/*���ۂɑ��݂���p�X��(�������������܂�)*/
	struct Demand *dmnd_head;	/*origin_olsp_list*/
} DemandList;


/*�\�[�g�p*/
typedef struct SortedDemand {
	long dmnd_num;
	Demand **dmnd;
	int *sort_idx;
}SortedDemand;

struct Demand *Demand_New();
struct DemandList *DemandList_New();
int MyRequest_Interval(double traffic_val, int ave_holdtime, int limit);
void Demand_Add(DemandList *aDemandL, int src, int hop, int start_t, int end_t, int freq, int route_index, Config *cfg);
void printDemand(DemandList *aDemandl, Config *cfg);
void Demand_AllDel(struct Demand *dmnd);
void Delete_DemandList(struct DemandList *aDemandL);
void Make_DemandList(DemandList *dmnd_l, double traffic, int time, Config *cfg);

struct DemandList *setPartOfDemand(DemandList *aDemandl, DemandList *aDemandlAll, int t, Config *cfg);
void AddDemand(DemandList *aDemandl, Demand *aDemand);

void Set_SortedDemand(SortedDemand *s_dmnd, Demand *d, long n);
struct SortedDemand *SortedDemand_New(DemandList *all_l);
void Put_Start_Time_To_SortIdx(SortedDemand *s_dmnd);
void Put_End_Time_To_SortIdx(SortedDemand *s_dmnd);
void Sort_Demand_Index_Small(SortedDemand *s_dmnd);
struct Demand *End_Search(int t, SortedDemand *end_l, int *end, int *another);
struct Demand *New_Search(int t, SortedDemand *arvl_l, int *st, int *another);
void Delete_SortedDemand(struct SortedDemand *s_dmnd);
void Delete_SortedDemand2(struct SortedDemand *s_dmnd);

short *compress_DemandL(SortedDemand *s_dmnd, SortedDemand *e_dmnd, Config *cfg);
