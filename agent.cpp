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


	int path_sum = 0;/*�����p�X�����@�G�s�\�[�h���܂������č��v����*/
	double *capa;/*����ԁA����Ԃ̃����N�L���p�V�e�B*/
	double e_total = 0.0;/*�덷�@�����̊m�F�p*/
	int *traffic = (int*)calloc(2, sizeof(int));/*�p�X�ݗ��v���@�n�[�m�[�h�ԍ��ƏI�[�m�[�h�ԍ�*/
	double alpha = cfg->alpha;/*�w�K���@������on�ݒ�̂��߂�Alpha�Ɨ�*/
	double value_current, value_next;
	int reward, train_time, route_index;
	PathList *tPathL = PathList_New(cfg);

	/*main_learn*/
	for (long int sim = 0; sim < cfg->main_train_time; sim++) {

		/*�c�]�e�ʃf�[�^�@�v�f���̓g�|���W�̃����N��*/
		capa = (double*)malloc(cfg->input_num * sizeof(double));
		/*�c�]�e�ʂ̏�����
		���݂̓t�@�C�o��l�̂��߂��ׂ�1*/
		for (int i = 0; i < cfg->input_num; i++)
			capa[i] = 1;

		/*�g���t�B�b�N����*/
		make_traffic(traffic, cfg);


		int counter_for_delete = 0;
		/*�u���b�L���O��������܂Ńp�X����{�����蓖�Ă�*/
		while ((route_index = select_route(traffic[0], traffic[1], capa, cfg->epsilon, nn, agent, learn_mode, 0, 0, cfg)) != -1) {//�������ł��Ȃ��Ȃ�܂�
			/*����*/
			Path_Add(traffic[0], traffic[1], 0, route_index, tPathL, cfg);

			//printf("%d, %d\n", traffic[0], traffic[1]);
			for (int i = 0; i < cfg->input_num; i++) {
				capa[i] = capa[i] - agent->route_table->val[route_index][i + 2] / agent->channel_num;//����Ԍ��
			}
			counter_for_delete++;
			path_sum++;
			/*�������̌v�Z*//*������̏��Ԃ�ς���nn->hi�̋������ς��ǂ��Ȃ�*/
			value_next = forward(nn, capa, cfg);
			value_current = forward(nn, capa, cfg);
			if (cfg->reward == 0)
				reward = get_shortestHop(agent, traffic[0], traffic[1], cfg);
			else
				reward = cfg->reward;

			/*�w�K�̏���*/
			double delta;
			if (cfg->alpha_normalization == 0)
				delta = reward + cfg->gamma * value_next - value_current;
			else/*��ԉ��l�Ő��K������ꍇ*/
				delta = (reward + cfg->gamma * value_next - value_current) / (value_current + 1);
			e_total += delta * delta;
			/*Alpha�̌���*/
			alpha *= cfg->alpha_attenuation;
			if (alpha < 0.0001)
				alpha = 0.0001;
			/*������̃t�@�C���o��*/
			if (path_sum % 1000 == 0) {
				fprintf(cfg->accuracy_learn_file, "%lf, %lf\n", e_total / 1000, alpha);
				e_total = 0;
			}

			/*�w�K*/
			olearn(nn, value_current + alpha * delta, value_current, cfg);
			if (cfg->hidden_layer_num >= 2)
				hlearn(nn, value_current + alpha * delta, value_current, cfg);
			ilearn(nn, capa, value_current + alpha * delta, value_current, cfg);

			/*�폜���[�`��*/
			if (counter_for_delete % cfg->delete_frequency == 0) {
				/*�폜����p�X������*/
				Path *deletePath = select_onePath(tPathL, cfg);

				/*Capa����폜*/
				for (int i = 0; i < cfg->input_num; i++) {
					if (agent->route_table->val[deletePath->route_index][i + 2] != 0)
						capa[i] = capa[i] + agent->route_table->val[deletePath->route_index][i + 2] / agent->channel_num;
				}
				/*pathList����폜*/
				Path_Delete(deletePath, tPathL, cfg);
				/*�g�[�^���p�X��+1*/
				path_sum--;
			}

			/*�g���t�B�b�N����*/
			make_traffic(traffic, cfg);
		}
		free(capa);
	}
	PathlDelete(tPathL, cfg);
	free(traffic);
}

//�p�����[�^�X�V���Ȃ�
double evaluate(Agent *agent, NN *nn, int mode_forDicision, Config *cfg) {
	long int path_sum = 0;
	int path_num;/*�I������p*/
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

		/*�g���t�B�b�N����*/
		make_traffic(traffic, cfg);
		int blocking_num = 0;
		while (true) {
			if ((select_freq(traffic[0], traffic[1], capa, nn, agent, tPathL, mode_forDicision, cfg)) == NULL) {//�������ł��Ȃ��Ȃ�܂�
				blocking_num++;/*�u���b�L���O*/
				if (blocking_num >= 1 + path_num / 100)/*�I������*/
					break;
				make_traffic(traffic, cfg);
				continue;
			}
			counter_forDelete++;

			/*�t�@�C���o�́@�f�o�b�O�p*/
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


			/*�폜���[�`��*/
			if (counter_forDelete % cfg->delete_frequency == 0) {
				/*�폜����p�X�i���Ԗڂ��j������*/
				Path *deletePath = select_onePath(tPathL, cfg);

				/*Capa����폜*/
				for (int i = 0; i < cfg->input_num; i++) {
					if (agent->route_table->val[deletePath->route_index][i + 2] != 0)
						capa[deletePath->freq][i] = capa[deletePath->freq][i] + agent->route_table->val[deletePath->route_index][i + 2] / agent->channel_num;
				}
				/*pathList����폜*/
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
	/*��ʏo��*/
	if (cfg->outputStatus == 1)printf("path -> %lf\n", (double)path_sum / cfg->evaluate_sim_num);
	return (double)path_sum / cfg->evaluate_sim_num;

}



/*
���̏�Ԃ𓾂�֐�
option:
epsilon �ÃO���[�f�B�[�@�̃�
mode_forDicision 0:NN�̉��l�̍ő�l�A1:�ŒZ�o�H����FF�A2: �c�]�e�ʃ_�C�N�X�g��
output_flag �]�����s����(evaluate)�Aoutput_flag���I���̎�,action_process.csv �t�@�C���ɏo�͂���
*/
int select_route(int s, int d, double *capa, float epsilon, NN *nn, Agent *agent, int mode_forDicision, int output_flag, int route_counter, Config *cfg) {
	int routing_candidate_counter = cfg->route_k;
	int route_index = 0;/*capa_copy�Ɏg��index �o�H�����Ǘ�����*/
	int max_value_index = -1;/*���l�ő�ƂȂ�s����index*/
	double max_value = -100000.0;/*���l�ő�ƂȂ�s���̉��l*/
	int escape_flag = 0;/*����P�̌o�H���ɂ����ċ󂫂��Ȃ��������ɑ��₩�ɑ��̌o�H���ֈړ����邽�߂̃t���O*/
	int existing_flag = 0;/*�S�Ă̌o�H�H�@�ɂ����Ċ����\�ȃp�X������Ȃ����@�u���b�N���A�֐�����o�邽�߂̃t���O*/
	int shortest_index = 0;/*option��shortest_route_only���s�����߂ɕێ����Ă���*/
	int shortest_hop = 100;

	/*route_select*/
	double *route_value_list = (double*)calloc(cfg->route_k, sizeof(double));


	/*�������@random�I��p�̔z��
	0�̃����N�͊��蓖�ĕs�o�H
	*/
	int *for_random_select = (int*)calloc(cfg->route_k, sizeof(int));//one-hot

	double *capa_tmp = (double*)malloc(cfg->input_num * sizeof(double));//�Ō�̗v�f�̓z�b�v��

																		   /*Route�t�@�C���ɂ�����A�v�������n�I�[�����o�H�����ꂼ��ŉ��l������s��*/
	for (int table_index = agent->route_table_index[s][d]; table_index < agent->route_table_index[s][d] + cfg->route_k; table_index++) {//���ׂĂ̍s���I�������r���đI������
		if (table_index >= agent->route_table->col_num)
			break;
		if (agent->route_table->val[table_index][0] != s || agent->route_table->val[table_index][1] != d)
			break;

		int hop = 0;
		for (int i = 0; i < cfg->input_num; i++) {
			hop += agent->route_table->val[table_index][i + 2];//�z�b�v���J�E���g
			capa_tmp[i] = capa[i] - agent->route_table->val[table_index][i + 2] / agent->channel_num;//����Ԍ��
			if (capa_tmp[i] < 0) {/*���蓖�ĉ\�� �`�F�b�N*/
				escape_flag = 1;
				route_value_list[route_index] = -99999;
				route_index++;
				break;
			}
		}

		if (escape_flag == 1) {/*���蓖�ĕs�\�̏ꍇ�@�ʌo�H�T��*/
			routing_candidate_counter--;
			escape_flag = 0;
			continue;
		}
		/*����routeID�Ɋ��蓖�ĉ\*/
		for_random_select[route_index]++;

		/*�S�Ă̌o�H���ɂ����ā@�ǂꂩ��ł����蓖�Ă��\��*/
		if (existing_flag == 0) {
			existing_flag = 1;
			max_value_index = route_index;
		}

		/*FirstFit�ɑI��*/
		route_value_list[route_index] = (cfg->route_k - route_index) - hop * 30;


		/*���l����*/
		double res_tmp = 0;
		if (mode_forDicision == 0)//NN
			res_tmp = forward(nn, capa_tmp, cfg);
		else if (mode_forDicision == 2)//�c�]�e�ʃ_�C�N�X�g��
			res_tmp = get_value_dijkstra(capa_tmp, agent->route_table->val[table_index], cfg);

		route_value_list[route_index] = res_tmp;


		//�ő傩�ǂ����𔻒�
		if (res_tmp > max_value) {
			max_value = res_tmp;
			max_value_index = route_index;

		}
		route_index++;

	}
	/*���蓖�Č�₪����Ȃ�*/
	if (existing_flag == 0) {//cfg->max_or_min == 0 && output_value == -1000.0 || cfg->max_or_min != 0 && output_value == 1000.0 ||
		//�Еt��
		free(capa_tmp);
		free(for_random_select);
		return -1;
	}
	int output_index = 0;

	/*�ÃO���[�f�B�[�@���s��*/
	int b = rand();
	float a = (float)b / 32767;

	if (epsilon <= a) { //greedy
		/*route_counter�Ԗڂ̌o�H�̌o�H�ԍ����o��*/
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
���̏�Ԃ𓾂�֐�
Evaluate�ŗ��p
�e�g�����C���ɂ�����select_route�ɂ��o�H�I�������̂��@�ő剿�l�ƂȂ�g����I������
option:
epsilon �ÃO���[�f�B�[�@�̃�
mode_forDicision 0:NN�̉��l�̍ő�l�A1:�ŒZ�o�H����FF�A2: �c�]�e�ʃ_�C�N�X�g��
output_flag �]�����s����(evaluate)�Aoutput_flag���I���̎�,action_process.csv �t�@�C���ɏo�͂���
*/
double **select_freq(int s, int d, double **capa, NN *nn, Agent *agent, PathList *path_list, int mode_forDicision, Config *cfg) {
	int target_freq = -1, route_index = -1, target_route_index = -1;
	double min_value = 1000000, max_value = -1000000, value;
	double *capa_tmp = (double*)malloc(cfg->input_num * sizeof(double));
	for (int freq = 0; freq < cfg->freq_num; freq++) {
		double value_current = forward(nn, capa[freq], cfg);
		/*���̔g���Ɋ��蓖�ĉ\��*/
		if ((route_index = select_route(s, d, capa[freq], 0, nn, agent, mode_forDicision, 0, 0, cfg)) == -1)
			continue;
		/*FF���_�C�N�X�g���@-�@�����\�g����������΂������蓖��*/
		if (mode_forDicision == 1 || mode_forDicision == 2) {
			target_route_index = route_index;
			target_freq = freq;
			break;
		}
		for (int i = 0; i < cfg->input_num; i++) {
			capa_tmp[i] = capa[freq][i] - agent->route_table->val[route_index][i + 2] / agent->channel_num;//����Ԍ��
		}
		/*���l�̍ő��I��*/
		if (cfg->selectFreq_mode == 0) {
			value = forward(nn, capa_tmp, cfg);
			if (max_value < value) {
				max_value = value;
				target_freq = freq;
				target_route_index = route_index;
			}
		}
		else {/*���l�̒ጸ�̍ŏ���I��*/
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
	/*����*/
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
	/*��������tmp_mtx��ɍ쐬�@�}�g���b�N�X�T�C�Y���\�z�ł��Ȃ�����*/
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