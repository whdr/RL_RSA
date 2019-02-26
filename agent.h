#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "NN.h"
#include "path.h"
#define MAIN_LEARN 0
#define SHORTEST_ONLY 1
#define INITIAL_LEARN 2


typedef struct Agent {

	int *shortest_path_num;/*�ŒZ�o�H���I�΂ꂽ��*/
	int channel_num;/*train_data��decode�p�@�����N�L���p�V�e�B��1�Ő��K�����邽��*/
	int **route_table_index;/*�o�H���e�[�u���ǂݎ�莞�A�T���͈͂����߂邽�߂ɗ��p*/

	MATRIX *route_table;
	MATRIX *fiber_data;
}Agent;

struct Agent *init_agent(MATRIX *aRoute_table, MATRIX *aFiber_data, Config *acfg);
double evaluate(Agent *agent, NN *nn, int select_shortest_flag, Config *cfg);
void learn(NN *nn, Agent *agent, int learn_mode, Config *cfg);
int select_route(int s, int d, double *capa, float epsilon,
	NN *nn, Agent *agent, int is_shortestHop_only, int output_flag, int route_counter, Config *cfg);
double get_value_dijkstra(double *capa, double *route, Config *cfg);
int get_shortestHop(Agent *agent, int s, int d, Config *cfg);
void make_routeTableIndex(MATRIX *route_table, int **route_table_index, Config *cfg);
struct MATRIX *reduce_routeCandidate(MATRIX *route_table, Config *cfg);
void make_traffic(int *traffic, Config *cfg);
double **select_freq(int s, int d, double **capa, NN *nn, Agent *agent, PathList *path_list, int mode_forDicision, Config *cfg);
