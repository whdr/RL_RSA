#if 0
#include "memory_leak.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "demand.h"

struct Demand *Demand_New()
{
	struct Demand *dmnd;
	dmnd = (struct Demand *)calloc(1, sizeof(struct Demand));

	dmnd->src = -1;
	dmnd->start_t = -1545;
	dmnd->end_t = -1;
	dmnd->next = NULL;
	dmnd->assign = 0;
	dmnd->FiberID = -1;
	dmnd->freq = -1;
	return dmnd;
}

/*動的デマンドリスト*/
struct DemandList *DemandList_New()
{
	struct DemandList *dmnd_list;
	dmnd_list = (struct DemandList *)calloc(1, sizeof(struct DemandList));
	dmnd_list->dmnd_num = 0;/*OLSPの数は0*/
	dmnd_list->dmnd_head = Demand_New();/*先頭のダミーOLSPを作る*/

	return dmnd_list;
}

/*待ち時間作成関数*/
int MyRequest_Interval(double traffic_val, int ave_holdtime, int limit)
{
	int t;
	double a, l, y, time = 0.0;
	a = (double)(rand() / ((double)RAND_MAX + 1.0f));/* ランダムに0<x<1な数字を出す */
	l = traffic_val / (double)ave_holdtime;
	for (t = 1; t <= limit; t++) {
		time = (double)t;
		y = 1 - (float)exp(-l*time);
		if (y > a) {
			break;
		}
	}
	if (time == limit)time = limit + 1;
	/*if(time==nw_cfg->time)time=nw_cfg->time*2;*/
	return (int)time;
}

/*デマンド構造体作成関数*/
void Demand_Add(DemandList *aDemandL, int src, int dst, int start_t, int end_t, int freq, int route_index, Config *cfg)
{
	struct Demand *dmnd;

	/* 領域確保 */
	dmnd = Demand_New();
	/* 要素の追加 */
	dmnd->src = src;
	dmnd->dst = dst;
	//dmnd->asn_num = all_l->dmnd_num;/*分割しても、わかる目印*/
	dmnd->start_t = start_t;
	dmnd->end_t = end_t;
	dmnd->assign = 0;
	dmnd->freq = freq;
	dmnd->route_index = route_index;

	/* 新しいdmndをdmndリストの最後尾へ */
	if (aDemandL->dmnd_head->next != NULL) {
		dmnd->before = aDemandL->dmnd_head->before;
		aDemandL->dmnd_head->before->next = dmnd;
	}
	if (aDemandL->dmnd_num == 0) {
		dmnd->before = aDemandL->dmnd_head;
		aDemandL->dmnd_head->next = dmnd;
	}
	aDemandL->dmnd_head->before = dmnd;


	aDemandL->dmnd_num++;
}




/*デマンド到着・保留時間作成関数*/
void Make_DemandList(DemandList *dmnd_l, double traffic, Config *cfg)
{
	int t = 0, start = 0, end = 0, holdtime = 0;
	int ave_holdtime = 5000;/*単位時間をいくつに刻むか*/
	double traffic_val;

	int **tDemand = (int**)calloc(cfg->node_num, sizeof(int*));
	for (int i = 0; i < cfg->node_num; i++)
		tDemand[i] = (int*)calloc(cfg->node_num, sizeof(int));

	for (int dst = 0; dst < cfg->node_num; dst++) {
		for (int src = 0; src < cfg->node_num; src++) {
			if (src == dst)
				tDemand[dst][src] = 0;
			else if (src == cfg->central_node || dst == cfg->central_node)
				tDemand[dst][src] = cfg->central_degree;
			else
				tDemand[dst][src] = 1;
		}
	}

	for (int src = 0; src < cfg->node_num; src++) {
		for (int dst = 0; dst < cfg->node_num; dst++) {
			if (tDemand[dst][src] != 0) {
				traffic_val = tDemand[dst][src] * cfg->flu_proportion;/*実質的なλ(頻度)*/
				while (start < cfg->time) {
					t = MyRequest_Interval(traffic_val, ave_holdtime, cfg->time - start);
					start += t;
					holdtime = MyRequest_Interval(1.0, ave_holdtime, cfg->time - start);
					end = start + holdtime;
					if (start <= cfg->time)
						Demand_Add(dmnd_l, src, dst, start, end, -1, -1, cfg);
				}
				start = 0;
			}
		}
	}

	for (int dst = 0; dst < cfg->node_num; dst++) {
		for (int src = 0; src < cfg->node_num; src++)
			tDemand[dst][src] = 0;
		free(tDemand[dst]);
	}
	free(tDemand);
}

void printDemand(DemandList *aDemandl, Config *cfg) {
	Demand *tDemand = aDemandl->dmnd_head->next;
	while (tDemand != NULL) {
		printf("%d,%d,%d,%d\n", tDemand->src, tDemand->dst, tDemand->start_t, tDemand->end_t);
		tDemand = tDemand->next;
	}
}


void Demand_AllDel(struct Demand *dmnd)
{
	Demand *temp = dmnd;
	while (dmnd) {
		temp = dmnd->next;
		free(dmnd);
		dmnd = temp;
	}
}

void Delete_DemandList(struct DemandList *aDemandL)
{
	if (aDemandL->dmnd_head != NULL) {
		Demand *demand = aDemandL->dmnd_head;
		Demand *temp;

		while (demand->next != NULL) {
			temp = demand->next;
			free(demand);
			demand = temp;
		}

	}
	free(aDemandL);
}

//高速化用
struct DemandList *setPartOfDemand(DemandList *aDemandl, DemandList *aDemandlAll, int t, Config *cfg) {
	DemandList *tDemandl = NULL;
	tDemandl = DemandList_New();
	Demand *tmp = NULL;
	Demand *tDemandSrc = aDemandlAll->dmnd_head->next;

	//開始時間がある値以降のデマンドを探す
	while (tDemandSrc != NULL) {
		tmp = tDemandSrc->next;
		if (tDemandSrc->end_t > t && tDemandSrc->start_t <= t + 50000) {
			//
			//削除（移行）
			tDemandSrc->before->next = tDemandSrc->next;
			if (tDemandSrc->next != NULL)
				tDemandSrc->next->before = tDemandSrc->before;

			AddDemand(tDemandl, tDemandSrc);

		}
		tDemandSrc = tmp;
	}

	//設立済みで、終了時間が閾値以降のデマンドを追加
	//tDeleteDemandは削除するデマンドのリスト
	DemandList *tDeleteDemandl = NULL;
	tDeleteDemandl = DemandList_New();
	Demand *tDemand = aDemandl->dmnd_head->next;
	if (t != 0) {
		while (tDemand != NULL) {
			tmp = tDemand->next;
			if (tDemand->end_t < t) {
				tDemand->before->next = tDemand->next;
				if (tDemand->next != NULL) {
					tDemand->next->before = tDemand->before;
				}
				else {
					aDemandl->dmnd_head->before = tDemand->before;
				}
				tDemand->next = tDeleteDemandl->dmnd_head->next;
				tDeleteDemandl->dmnd_head->next = tDemand;
			}
			tDemand = tmp;
		}
		//printDemand(aDemandl,cfg);

		Delete_DemandList(tDeleteDemandl);
		//結合
		tDemandl->dmnd_head->before->next = aDemandl->dmnd_head->next;
		aDemandl->dmnd_head->next->before = tDemandl->dmnd_head->before;

		tDemandl->dmnd_head->before = aDemandl->dmnd_head->before;
		tDemandl->dmnd_head->before->next = NULL;

	}
	return tDemandl;
}


void AddDemand(DemandList *aDemandl, Demand *aDemand) {


	/* 新しいdmndをdmndリストの最後尾へ */
	if (aDemandl->dmnd_head->next != NULL) {
		aDemand->before = aDemandl->dmnd_head->before;
		aDemandl->dmnd_head->before->next = aDemand;
	}
	if (aDemandl->dmnd_num == 0) {
		aDemand->before = aDemandl->dmnd_head;
		aDemandl->dmnd_head->next = aDemand;
	}
	aDemandl->dmnd_head->before = aDemand;
	aDemand->next = NULL;

	aDemandl->dmnd_num++;
}


/* DemandListのdmndをSortedDemandListに追加する関数 */
void Set_SortedDemand(SortedDemand *s_dmnd, Demand *d, long n)
{
	Demand *dmnd = d;
	while (dmnd) {
		s_dmnd->dmnd[n] = dmnd;
		n++;
		dmnd = dmnd->next;
	}
	return;
}

/* DemandList(dmnd_l)をstart_timeが小さい順に並び替え ========================*/
struct SortedDemand *SortedDemand_New(DemandList *all_l)
{
	SortedDemand *s_dmnd;
	/* 領域確保 */
	s_dmnd = (SortedDemand *)calloc(1, sizeof(SortedDemand));
	/* 未割り当てなサービス数を取得 */
	s_dmnd->dmnd_num = all_l->dmnd_num;
	/* 未割り当てサービス数分の領域確保 */
	s_dmnd->dmnd = (Demand **)calloc(s_dmnd->dmnd_num, sizeof(Demand *));
	s_dmnd->sort_idx = (int *)calloc(s_dmnd->dmnd_num, sizeof(int));
	/* 未割り当てサービスリストに追加 */
	if (all_l->dmnd_head->next != NULL) {
		Set_SortedDemand(s_dmnd, all_l->dmnd_head->next, 0);
	}

	return s_dmnd;
}

/*発生時間をソートパラメータに入力*/
void Put_Start_Time_To_SortIdx(SortedDemand *s_dmnd)
{
	int i;

	for (i = 0; i < s_dmnd->dmnd_num; i++) {
		s_dmnd->sort_idx[i] = s_dmnd->dmnd[i]->start_t;
	}
	return;
}

/*終了時刻をソートパラメータに入力*/
void Put_End_Time_To_SortIdx(SortedDemand *s_dmnd)
{
	int i;

	for (i = 0; i < s_dmnd->dmnd_num; i++) {
		s_dmnd->sort_idx[i] = s_dmnd->dmnd[i]->end_t;
	}
	return;
}

/* indexに入力された情報を見て値が小さい順にソートする関数*/
void Sort_Demand_Index_Small(SortedDemand *s_dmnd)
{
	int i, j, sorted;
	Demand *temp_dmnd;
	int temp_idx;

	j = s_dmnd->dmnd_num - 1;
	do {
		sorted = 1;
		j = j - 1;
		for (i = 0; i <= j; i++) {
			if (s_dmnd->sort_idx[i] > s_dmnd->sort_idx[i + 1]) {
				/* olspを並び替え*/
				temp_dmnd = s_dmnd->dmnd[i];
				s_dmnd->dmnd[i] = s_dmnd->dmnd[i + 1];
				s_dmnd->dmnd[i + 1] = temp_dmnd;
				/*インデックスを並び替え*/
				temp_idx = s_dmnd->sort_idx[i];
				s_dmnd->sort_idx[i] = s_dmnd->sort_idx[i + 1];
				s_dmnd->sort_idx[i + 1] = temp_idx;
				sorted = 0;
			}
		}
	} while (!sorted);
	return;
}

/*現在の時間に削除する波長パスが存在するか*/
struct Demand *End_Search(int t, SortedDemand *end_l, int *end, int *another)
{
	int i;
	Demand *dmnd;

	for (i = *end; i < end_l->dmnd_num; i++) {
		dmnd = end_l->dmnd[i];
		if ((t == dmnd->end_t) && (dmnd->assign == 1)) {
			*end = i;
			/* fprintf(stderr,"end_pointer %3d	",*end); */
			*another = 1;
			return dmnd;
		}
		else if (dmnd->end_t > t) {
			*another = 0;
			break;
		}
	}
	return NULL;
}

/*この時間に設立する波長パスが存在するか*/
struct Demand *New_Search(int t, SortedDemand *arvl_l, int *st, int *another)
{
	int i;
	Demand *dmnd;
	for (i = *st; i < arvl_l->dmnd_num; i++) {
		dmnd = arvl_l->dmnd[i];
		if ((t == dmnd->start_t) && (dmnd->assign == 0)) {
			*st = i;
			*another = 1;
			return dmnd;
		}
		else if (dmnd->start_t > t) {
			*another = 0;
			break;
		}
	}
	return NULL;
}

short *compress_DemandL(SortedDemand *s_dmnd, SortedDemand *e_dmnd, Config *cfg) {
	int job_num = 1;
	short *job_table = (short*)malloc(100000 * sizeof(short));
	for (int t = 0; t < cfg->time; t++) {
		int end = 0, st = 0, another = 0;

		if (End_Search(t, e_dmnd, &end, &another) != NULL) {/*削除トラフィックの探索*/
			job_table[job_num] = t;
			job_num++;
			continue;
		}
		if (New_Search(t, s_dmnd, &end, &another) != NULL) {/*削除トラフィックの探索*/
			job_table[job_num] = t;
			job_num++;
			continue;
		}
	}
	job_table[0] = job_num;
	return job_table;
}

void Delete_SortedDemand(struct SortedDemand *s_dmnd)
{
	free_nullcheck(s_dmnd->dmnd);
	free_nullcheck(s_dmnd->sort_idx);
	free_nullcheck(s_dmnd);
}

void Delete_SortedDemand2(struct SortedDemand *s_dmnd)
{
	int i;
	for (i = 0; i < s_dmnd->dmnd_num; i++) {
		free_nullcheck(s_dmnd->dmnd[i]);
	}

	free_nullcheck(s_dmnd->dmnd);
	free_nullcheck(s_dmnd->sort_idx);
	free_nullcheck(s_dmnd);
}
