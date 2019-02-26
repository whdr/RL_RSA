#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "agent.h"

Agent *init_agent(MATRIX *aRoute_table, MATRIX *aFiber_data, Config *cfg) {

	Agent *agent = (Agent *)malloc(sizeof(Agent));

	agent->fiber_data = aFiber_data;
	agent->shortest_path_num = (int*)calloc(1, sizeof(int));
	agent->channel_num = cfg->fiber_basis_num * cfg->max_fiber_num;
	agent->route_table_index = (int**)calloc(cfg->node_num, sizeof(int*));
	for (int i = 0; i < cfg->node_num; i++)
		agent->route_table_index[i] = (int*)calloc(cfg->node_num, sizeof(int));
	agent->route_table = aRoute_table;
	make_routeTableIndex(agent->route_table, agent->route_table_index, cfg);
	return agent;
}

void learn(NN *nn, Agent *agent, int learn_mode, Config *cfg) {


	int path_sum = 0;/*割当パス総数　エピソードをまたがって合計する*/
	double *capa;/*現状態、次状態のリンクキャパシティ*/
	double e_total = 0.0;/*誤差　収束の確認用*/
	int *traffic = (int*)calloc(2, sizeof(int));/*パス設立要求　始端ノード番号と終端ノード番号*/
	double alpha = cfg->alpha;/*学習率　α減衰on設定のためにAlpha独立*/
	double value_current, value_next;
	int reward, train_time, route_index;
	PathList *tPathL = PathList_New(cfg);

	/*main_learn*/
	for (long int sim = 0; sim < cfg->main_train_time; sim++) {

		/*残余容量データ　要素数はトポロジのリンク数*/
		capa = (double*)malloc(cfg->input_num * sizeof(double));
		/*残余容量の初期化
		現在はファイバ一様のためすべて1*/
		for (int i = 0; i < cfg->input_num; i++)
			capa[i] = 1;

		/*トラフィック生成*/
		make_traffic(traffic, cfg);


		int counter_for_delete = 0;
		/*ブロッキングが生じるまでパスを一本ずつ割り当てる*/
		while ((route_index = select_route(traffic[0], traffic[1], capa, cfg->epsilon, nn, agent, learn_mode, 0, 0, cfg)) != -1) {//割当ができなくなるまで
			/*割当*/
			Path_Add(traffic[0], traffic[1], 0, route_index, tPathL, cfg);

			//printf("%d, %d\n", traffic[0], traffic[1]);
			for (int i = 0; i < cfg->input_num; i++) {
				capa[i] = capa[i] - agent->route_table->val[route_index][i + 2] / agent->channel_num;//次状態候補
			}
			counter_for_delete++;
			path_sum++;
			/*順方向の計算*//*した二つの順番を変えるnn->hiの挙動が変わり良くない*/
			value_next = forward(nn, capa, cfg);
			value_current = forward(nn, capa, cfg);
			if (cfg->reward == 0)
				reward = get_shortestHop(agent, traffic[0], traffic[1], cfg);
			else
				reward = cfg->reward;

			/*学習の準備*/
			double delta;
			if (cfg->alpha_normalization == 0)
				delta = reward + cfg->gamma * value_next - value_current;
			else/*状態価値で正規化する場合*/
				delta = (reward + cfg->gamma * value_next - value_current) / (value_current + 1);
			e_total += delta * delta;
			/*Alphaの減衰*/
			alpha *= cfg->alpha_attenuation;
			if (alpha < 0.0001)
				alpha = 0.0001;
			/*収束具合のファイル出力*/
			if (path_sum % 1000 == 0) {
				fprintf(cfg->accuracy_learn_file, "%lf, %lf\n", e_total / 1000, alpha);
				e_total = 0;
			}

			/*学習*/
			olearn(nn, value_current + alpha * delta, value_current, cfg);
			if (cfg->hidden_layer_num >= 2)
				hlearn(nn, value_current + alpha * delta, value_current, cfg);
			ilearn(nn, capa, value_current + alpha * delta, value_current, cfg);

			/*削除ルーチン*/
			if (counter_for_delete % cfg->delete_frequency == 0) {
				/*削除するパスを決定*/
				Path *deletePath = select_onePath(tPathL, cfg);

				/*Capaから削除*/
				for (int i = 0; i < cfg->input_num; i++) {
					if (agent->route_table->val[deletePath->route_index][i + 2] != 0)
						capa[i] = capa[i] + agent->route_table->val[deletePath->route_index][i + 2] / agent->channel_num;
				}
				/*pathListから削除*/
				Path_Delete(deletePath, tPathL, cfg);
				/*トータルパスに+1*/
				path_sum--;
			}

			/*トラフィック生成*/
			make_traffic(traffic, cfg);
		}
		free(capa);
	}
	PathlDelete(tPathL, cfg);
	free(traffic);
}

//パラメータ更新しない
double evaluate(Agent *agent, NN *nn, int mode_forDicision, Config *cfg) {
	long int path_sum = 0;
	int path_num;/*終了判定用*/
	int *traffic = (int*)malloc(2 * sizeof(int));
	int counter_forDelete = 0;
	double **capa;

	double *capa_total = (double*)malloc(cfg->input_num * sizeof(double));
	PathList *tPathL;
	for (int sim = 0; sim < cfg->evaluate_sim_num; sim++) {
		tPathL = PathList_New(cfg);
		path_num = 0;

		capa = (double**)malloc(cfg->freq_num * sizeof(double*));
		for (int freq = 0; freq < cfg->freq_num; freq++) {
			capa[freq] = (double*)malloc(cfg->input_num * sizeof(double));
			for (int i = 0; i < cfg->input_num; i++)
				capa[freq][i] = 1;// /*agent->fiber_data->val[i][0]*/ / cfg->max_fiber_num;
		}

		/*トラフィック生成*/
		make_traffic(traffic, cfg);
		int blocking_num = 0;
		while (true) {
			if ((select_freq(traffic[0], traffic[1], capa, nn, agent, tPathL, mode_forDicision, cfg)) == NULL) {//割当ができなくなるまで
				blocking_num++;/*ブロッキング*/
				if (blocking_num >= 1 + path_num / 100)/*終了判定*/
					break;
				make_traffic(traffic, cfg);
				continue;
			}
			counter_forDelete++;

			/*ファイル出力　デバッグ用*/
			if (cfg->output_capacity_graph == 1) {
				for (int i = 0; i < cfg->input_num; i++) {
					capa_total[i] = 0;
					for (int freq = 0; freq < cfg->freq_num; freq++)
						capa_total[i] += capa[freq][i];
				}
				if (sim == 0) {
					for (int i = 0; i < cfg->input_num; i++)
						fprintf(cfg->capacity_transition_file, "%lf,", capa_total[i]);
					fprintf(cfg->capacity_transition_file, "\n");
				}
			}


			/*削除ルーチン*/
			if (counter_forDelete % cfg->delete_frequency == 0) {
				/*削除するパス（何番目か）を決定*/
				Path *deletePath = select_onePath(tPathL, cfg);

				/*Capaから削除*/
				for (int i = 0; i < cfg->input_num; i++) {
					if (agent->route_table->val[deletePath->route_index][i + 2] != 0)
						capa[deletePath->freq][i] = capa[deletePath->freq][i] + agent->route_table->val[deletePath->route_index][i + 2] / agent->channel_num;
				}
				/*pathListから削除*/
				Path_Delete(deletePath, tPathL, cfg);
			}
			else {
				path_sum++;
				path_num++;
			}
			make_traffic(traffic, cfg);
		}
		PathlDelete(tPathL, cfg);
	}
	free(traffic);
	/*画面出力*/
	if (cfg->outputStatus == 1)printf("path -> %lf\n", (double)path_sum / cfg->evaluate_sim_num);
	return (double)path_sum / cfg->evaluate_sim_num;

}



/*
次の状態を得る関数
option:
epsilon εグリーディー法のε
mode_forDicision 0:NNの価値の最大値、1:最短経路からFF、2: 残余容量ダイクストラ
output_flag 評価を行う時(evaluate)、output_flagがオンの時,action_process.csv ファイルに出力する
*/
int select_route(int s, int d, double *capa, float epsilon, NN *nn, Agent *agent, int mode_forDicision, int output_flag, int route_counter, Config *cfg) {
	int routing_candidate_counter = cfg->route_k;
	int route_index = 0;/*capa_copyに使うindex 経路候補を管理する*/
	int max_value_index = -1;/*価値最大となる行動のindex*/
	double max_value = -100000.0;/*価値最大となる行動の価値*/
	int escape_flag = 0;/*ある１つの経路候補において空きがなかった時に速やかに他の経路候補へ移動するためのフラグ*/
	int existing_flag = 0;/*全ての経路工法において割当可能なパスが一つもない時　ブロックし、関数から出るためのフラグ*/
	int shortest_index = 0;/*optionのshortest_route_onlyを行うために保持しておく*/
	int shortest_hop = 100;

	/*route_select*/
	double *route_value_list = (double*)calloc(cfg->route_k, sizeof(double));


	/*お試し　random選択用の配列
	0のリンクは割り当て不可経路
	*/
	int *for_random_select = (int*)calloc(cfg->route_k, sizeof(int));//one-hot

	double *capa_tmp = (double*)malloc(cfg->input_num * sizeof(double));//最後の要素はホップ数

																		   /*Routeファイルにおける、要求される始終端を持つ経路をそれぞれで価値判定を行う*/
	for (int table_index = agent->route_table_index[s][d]; table_index < agent->route_table_index[s][d] + cfg->route_k; table_index++) {//すべての行動選択肢を比較して選択する
		if (table_index >= agent->route_table->col_num)
			break;
		if (agent->route_table->val[table_index][0] != s || agent->route_table->val[table_index][1] != d)
			break;

		int hop = 0;
		for (int i = 0; i < cfg->input_num; i++) {
			hop += agent->route_table->val[table_index][i + 2];//ホップ数カウント
			capa_tmp[i] = capa[i] - agent->route_table->val[table_index][i + 2] / agent->channel_num;//次状態候補
			if (capa_tmp[i] < 0) {/*割り当て可能か チェック*/
				escape_flag = 1;
				route_value_list[route_index] = -99999;
				route_index++;
				break;
			}
		}

		if (escape_flag == 1) {/*割り当て不能の場合　別経路探索*/
			routing_candidate_counter--;
			escape_flag = 0;
			continue;
		}
		/*そのrouteIDに割り当て可能*/
		for_random_select[route_index]++;

		/*全ての経路候補において　どれか一つでも割り当てが可能か*/
		if (existing_flag == 0) {
			existing_flag = 1;
			max_value_index = route_index;
		}

		/*FirstFitに選択*/
		route_value_list[route_index] = (cfg->route_k - route_index) - hop * 30;


		/*価値判定*/
		double res_tmp = 0;
		if (mode_forDicision == 0)//NN
			res_tmp = forward(nn, capa_tmp, cfg);
		else if (mode_forDicision == 2)//残余容量ダイクストラ
			res_tmp = get_value_dijkstra(capa_tmp, agent->route_table->val[table_index], cfg);

		route_value_list[route_index] = res_tmp;


		//最大かどうかを判定
		if (res_tmp > max_value) {
			max_value = res_tmp;
			max_value_index = route_index;

		}
		route_index++;

	}
	/*割り当て候補が一つもない*/
	if (existing_flag == 0) {//cfg->max_or_min == 0 && output_value == -1000.0 || cfg->max_or_min != 0 && output_value == 1000.0 ||
		//片付け
		free(capa_tmp);
		free(for_random_select);
		return -1;
	}
	int output_index = 0;

	/*εグリーディー法を行う*/
	int b = rand();
	float a = (float)b / 32767;

	if (epsilon <= a) { //greedy
		/*route_counter番目の経路の経路番号を出力*/
		for (int i_flag = 0; i_flag < route_counter - 1; i_flag++) {
			int max_route_value = 0;
			int max_route_index_t = -1;
			for (int r = 0; r < cfg->route_k; r++) {
				if (route_value_list[r] > max_route_value) {
					max_route_index_t = r;
					max_route_value = route_value_list[r];
				}
			}
			route_value_list[max_route_index_t] = -10000;
		}
		int max_route_value = 0;
		int max_route_index_t2 = -1;
		for (int r = 0; r < cfg->route_k; r++) {
			if (route_value_list[r] > max_route_value) {
				max_route_index_t2 = r;
				max_route_value = route_value_list[r];
			}
		}
		output_index = agent->route_table_index[s][d] + max_value_index;
	}
	else {//random
		output_index = agent->route_table_index[s][d] + rand() % cfg->route_k;
		while (for_random_select[output_index - agent->route_table_index[s][d]] == 0)
			output_index = agent->route_table_index[s][d] + rand() % cfg->route_k;
	}
	free(capa_tmp);
	free(for_random_select);
	return output_index;
}

/*
次の状態を得る関数
Evaluateで利用
各波長レイヤにおいてselect_routeにより経路選択したのち　最大価値となる波長を選択する
option:
epsilon εグリーディー法のε
mode_forDicision 0:NNの価値の最大値、1:最短経路からFF、2: 残余容量ダイクストラ
output_flag 評価を行う時(evaluate)、output_flagがオンの時,action_process.csv ファイルに出力する
*/
double **select_freq(int s, int d, double **capa, NN *nn, Agent *agent, PathList *path_list, int mode_forDicision, Config *cfg) {
	int target_freq = -1, route_index = -1, target_route_index = -1;
	double min_value = 1000000, max_value = -1000000, value;
	double *capa_tmp = (double*)malloc(cfg->input_num * sizeof(double));
	for (int freq = 0; freq < cfg->freq_num; freq++) {
		double value_current = forward(nn, capa[freq], cfg);
		/*この波長に割り当て可能か*/
		if ((route_index = select_route(s, d, capa[freq], 0, nn, agent, mode_forDicision, 0, 0, cfg)) == -1)
			continue;
		/*FF＆ダイクストラ　-　割当可能波長が見つかればすぐ割り当る*/
		if (mode_forDicision == 1 || mode_forDicision == 2) {
			target_route_index = route_index;
			target_freq = freq;
			break;
		}
		for (int i = 0; i < cfg->input_num; i++) {
			capa_tmp[i] = capa[freq][i] - agent->route_table->val[route_index][i + 2] / agent->channel_num;//次状態候補
		}
		/*価値の最大を選択*/
		if (cfg->selectFreq_mode == 0) {
			value = forward(nn, capa_tmp, cfg);
			if (max_value < value) {
				max_value = value;
				target_freq = freq;
				target_route_index = route_index;
			}
		}
		else {/*価値の低減の最小を選択*/
			value = value_current - forward(nn, capa_tmp, cfg);
			if (min_value > value) {
				min_value = value;
				target_freq = freq;
				target_route_index = route_index;
			}
		}
	}
	free(capa_tmp);
	if (target_freq == -1)
		return NULL;
	/*割当*/
	Path_Add(s, d, target_freq, target_route_index, path_list, cfg);
	for (int i = 0; i < cfg->input_num; i++)
		capa[target_freq][i] = capa[target_freq][i] - agent->route_table->val[target_route_index][i + 2] / agent->channel_num;
	return capa;
}



double get_value_dijkstra(double *capa, double *route, Config *cfg) {
	double res = 0.0;
	for (int i = 0; i < cfg->input_num; i++) {
		if (route[i + 2] != 0.0)
			res += (1000 / capa[i]);
	}
	return -res;
}

int get_shortestHop(Agent *agent, int s, int d, Config *cfg) {

	int table_index = 0;
	while (agent->route_table->val[table_index][0] != s || agent->route_table->val[table_index][1] != d)
		table_index++;
	int hop = 0;
	for (int i = 0; i < cfg->input_num; i++)
		hop += agent->route_table->val[table_index][i + 2];
	return hop;
}

void make_routeTableIndex(MATRIX *route_table, int **route_table_index, Config *cfg) {
	int s = -1, d = -1;
	for (int table_index = 0; table_index < route_table->col_num; table_index++) {
		if (s == route_table->val[table_index][0] && d == route_table->val[table_index][1])
			continue;
		s = route_table->val[table_index][0];
		d = route_table->val[table_index][1];
		route_table_index[s][d] = (double)table_index;
		//printf("%d, %d, %d\n", s, d, route_table_index[s][d]);
	}
}



struct MATRIX *reduce_routeCandidate(MATRIX *route_table, Config *cfg) {

	if (cfg->route_candidate == 0)
		return route_table;

	/*make_matrix*/
	/*いったんtmp_mtx上に作成　マトリックスサイズが予想できないため*/
	MATRIX *tmp_mtx = (MATRIX *)calloc(1, sizeof(MATRIX));
	MATRIX *res_mtx = (MATRIX *)calloc(1, sizeof(MATRIX));
	tmp_mtx->row_num = route_table->row_num;
	tmp_mtx->col_num = route_table->col_num;

	tmp_mtx->val = (double **)calloc(tmp_mtx->col_num, sizeof(double *));
	for (int i = 0; i < tmp_mtx->col_num; i++)
		tmp_mtx->val[i] = (double *)calloc(tmp_mtx->row_num, sizeof(double));

	int tmp_index = 0;
	int s_index = 0;
	int s_candidate = 0;
	int s_startCol = 0;

	for (int src = 0; src < route_table->col_num; src++) {
		for (int dst = 0; dst < route_table->row_num; dst++) {
			if (src == dst)
				continue;

			/*count_candidate_num*/
			s_candidate = 0;
			for (int col = 0; col < route_table->col_num; col++) {
				if (route_table->val[col][0] == src && route_table->val[col][1] == dst) {
					if (s_candidate == 0)
						s_startCol = col;
					s_candidate++;
				}
				else
					if (s_candidate != 0)
						break;
			}

			/*reduce_candidate*/
			if (s_candidate < cfg->route_candidate) {
				for (int i = 0; i < s_candidate; i++) {
					for (int row = 0; row < route_table->row_num; row++) {
						tmp_mtx->val[i + tmp_index][row] = route_table->val[s_startCol + i][row];
						//printf("%.0lf, ", res_mtx->val[i + res_index][row]);
					}
					//printf("\n");
				}
				tmp_index += s_candidate;
			}
			else {/*reduce_candidate*/

				for (int row = 0; row < route_table->row_num; row++) {
					tmp_mtx->val[tmp_index][row] = route_table->val[s_startCol][row];
					//printf("%.0lf, ", res_mtx->val[res_index][row]);
				}

				for (int num = 1; num < cfg->route_candidate; num++) {

					for (int row = 0; row < route_table->row_num; row++) {
						tmp_mtx->val[tmp_index + num][row] = route_table->val[s_startCol + ((int)s_candidate / cfg->route_candidate * num) + 1][row];
						//printf("%.0lf, ", res_mtx->val[res_index][row]);
					}

				}
				tmp_index += cfg->route_candidate;

			}
		}
	}

	res_mtx->col_num = tmp_index;
	res_mtx->row_num = route_table->row_num;

	res_mtx->val = (double **)calloc(res_mtx->col_num, sizeof(double *));
	for (int i = 0; i < res_mtx->col_num; i++)
		res_mtx->val[i] = (double *)calloc(res_mtx->row_num, sizeof(double));

	for (int col = 0; col < res_mtx->col_num; col++) {
		for (int row = 0; row < res_mtx->row_num; row++) {
			res_mtx->val[col][row] = tmp_mtx->val[col][row];
			//printf("%.0lf, ", res_mtx->val[col][row]);
		}
		//printf("\n");
	}


	DeleteMatrix(route_table);
	DeleteMatrix(tmp_mtx);
	return res_mtx;
}

void make_traffic(int *traffic, Config *cfg) {

	if (cfg->demand_pattern == 0)
		cfg->central_degree = 1;

	int s = rand() % (cfg->node_num + cfg->central_degree - 1);
	if (s >= cfg->node_num)
		s = 0;

	int d = rand() % (cfg->node_num + cfg->central_degree - 1);
	if (d >= cfg->node_num)
		d = 0;
	while (s == d) {
		d = rand() % (cfg->node_num + cfg->central_degree - 1);
		if (d >= cfg->node_num)
			d = 0;
	}

	s = (s + cfg->central_node) % cfg->node_num;
	d = (d + cfg->central_node) % cfg->node_num;

	traffic[0] = s;
	traffic[1] = d;
	//printf("%d, %d\n", s, d);

}