#if 0
#include "memory_leak.h"
#endif
#include <stdio.h>
#include <stdlib.h>

#include "path.h"



struct PathList *PathList_New(Config *cfg) {
	struct PathList *pathl;
	pathl = (struct PathList *)malloc(sizeof(struct PathList));
	pathl->path_head = (struct Path ***)malloc(cfg->node_num * sizeof(struct PathList**));
	for (int i = 0; i < cfg->node_num; i++) {
		pathl->path_head[i] = (struct Path **)malloc(cfg->node_num * sizeof(struct PathList*));
		for (int j = 0; j < cfg->node_num; j++)
			pathl->path_head[i][j] = Path_New();
	}

	pathl->path_num_sd = (int**)malloc(cfg->node_num * sizeof(int*));
	for (int i = 0; i < cfg->node_num; i++) {
		pathl->path_num_sd[i] = (int*)calloc(cfg->node_num, sizeof(int));
	}

	pathl->path_num = 0;
	pathl->path_num_all = 0;

	return pathl;
}



struct Path *Path_New() {
	struct Path *path;
	path = (struct Path *)malloc(sizeof(struct Path));
	path->src = -1;
	path->dst = -1;
	path->freq = -1;
	path->route_index = -1;
	path->assign = -1;


	path->next = NULL;
	path->before = NULL;
	return path;
}
void PathlDelete(PathList *aPathl, Config *cfg) {
	//int i = 0;
	//printf("%d", aPathl->path_num);

	for (int i = 0; i < cfg->node_num; i++) {
		for (int j = 0; j < cfg->node_num; j++) {
			if (aPathl->path_head != NULL) {
				Path *temp, *head, *path;
				head = aPathl->path_head[i][j];
				path = head;
				//printPath(aPathl);
				while (path != NULL) {
					temp = path->next;
					free(path);
					path = temp;
					//i++;
				}
			}
		}
		free(aPathl->path_head[i]);
	}
	free(aPathl->path_head);


	//printf("%d", i);

	free_nullcheck(aPathl);
	aPathl = NULL;
}

/*íœ‚Ì‚‘¬‰»—p*/
struct Path *select_onePath(struct PathList *aPathL, Config *cfg) {

	/*Å‘å”‚ÌSD‚ğ’T‚·*/
	int max_num = 0;
	for (int i = 0; i < cfg->node_num; i++) {
		for (int j = 0; j < cfg->node_num; j++) {
			if (aPathL->path_num_sd[i][j] > max_num)
				max_num = aPathL->path_num_sd[i][j];
		}
	}

	int src, dst;
	while (true) {
		src = rand() % cfg->node_num;
		dst = rand() % cfg->node_num;
		while (src == dst)
			dst = rand() % cfg->node_num;
		if (aPathL->path_num_sd[src][dst] > rand() % max_num)
			break;
	}

	Path *tPath = aPathL->path_head[src][dst]->next;
	int num = rand() % aPathL->path_num_sd[src][dst];
	for (int i = 0; i < num; i++)
		tPath = tPath->next;
	return tPath;

}


struct Path *Path_Add(int aSrc, int aDst, int aFreq, int route_index, PathList *aPathL, Config *cfg) {
	Path *path = Path_New();
	path->PathID = aPathL->path_num_all;
	aPathL->path_num_all++;
	path->src = aSrc;
	path->dst = aDst;
	path->freq = aFreq;
	path->route_index = route_index;

	if (aPathL->path_head[aSrc][aDst]->next != NULL) {
		path->before = aPathL->path_head[aSrc][aDst]->before;
		aPathL->path_head[aSrc][aDst]->before->next = path;
	}
	if (aPathL->path_num_sd[aSrc][aDst] == 0) {
		path->before = aPathL->path_head[aSrc][aDst];
		aPathL->path_head[aSrc][aDst]->next = path;
	}
	aPathL->path_head[aSrc][aDst]->before = path;

	aPathL->path_num++;
	aPathL->path_num_sd[aSrc][aDst]++;
	return path;
}


void Path_Delete(Path *aPath, PathList *aPathL, Config *cfg) {
	aPath->before->next = aPath->next;
	if (aPath->next != NULL)
		aPath->next->before = aPath->before;
	else {
		aPathL->path_head[aPath->src][aPath->dst]->before = aPath->before;
	}
	aPathL->path_num--;
	aPathL->path_num_sd[aPath->src][aPath->dst]--;

	free(aPath);
	return;
}


void PathCopy(Path *aSrc, Path *aCopy) {
	aCopy->PathID = aSrc->PathID;
	aCopy->src = aSrc->src;
	aCopy->dst = aSrc->dst;
	aCopy->freq = aSrc->freq;
	aCopy->assign = aSrc->assign;
	aCopy->next = NULL;
}

/*void print_pathList(PathList *aPathL, Config *cfg) {
	Path *tPath = aPathL->path_head->next;
	while (tPath != NULL) {

		printf("%d, %d, %d, %d\n", tPath->PathID, tPath->src, tPath->dst, tPath->route_index);

		tPath = tPath->next;
	}

}
*/

