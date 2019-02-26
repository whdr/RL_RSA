#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "misc.h"
#include "config.h"

typedef struct Path {
	int PathID;
	int src;
	int dst;
	int freq;
	int route_index;
	int assign;// 0: 1:Š„‚è“–‚Ä -1: –¢Š„“–

	struct Path *next;
	struct Path *before;
}Path;

typedef struct PathList {
	long path_num;
	long path_num_all;
	struct Path ***path_head;
	int **path_num_sd;
	
}PathList;

struct PathList *PathList_New(Config *cfg);
struct Path *Path_New();
struct Path *Path_Add(int aSrc, int aDst, int aFreq, int route_index, PathList *aPathL, Config *cfg);
void PathCopy(Path *aSrc, Path *aCopy);
void PathlDelete(PathList *aPathl, Config *cfg);
void Path_Delete(Path *aPath, PathList *aPathL, Config *cfg);

struct Path *select_onePath(struct PathList *aPathL, Config *cfg);

