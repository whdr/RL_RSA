#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "misc.h"

typedef struct Config {
	int node_num;

	int freq_num;
	int fiber_demand;
	
	int route_k;

	int select_priority;

	/*動的処理用パラメータ*/
	int time; /*実験時間*/
	int real_time;/*測定開始時間*/
	double flu_proportion;/*初期トラフィック期待値の変動割合*/

	int topology;
	int hop_slug;
	int outputStatus;
	int route_candidate;

	int sim_num;
	int evaluate_time;

	//demand
	int demand_pattern;
	int central_node;
	int central_degree;

	int fiber_distribution;
	int selectFreq_mode;

	//related on NN
	int input_num;
	int hidden_num;
	int hidden_layer_num;
	double alpha;
	int alpha_normalization;
	double alpha_attenuation;
	double leak;
	double epsilon;
	double reward;
	double gamma;
	double lambda;

	int hidden_neuron_type;
	int output_neuron_type;

	int block;
	long main_train_time;

	int output_accuracy;
	int output_capacity_graph;
	int read_weight;

	FILE *accuracy_learn_file;
	FILE *capacity_transition_file;
	FILE *route_table_file;
	FILE *fiber_file;
	FILE *action_process_file;
	FILE *weight_i_file;
	FILE *weight_h_file;
	FILE *weight_o_file;
	FILE *step_num_hist_file;
	FILE *blocking_link_hist_file;
	FILE *read_weight_i_file;
	FILE *read_weight_h_file;
	FILE *read_weight_o_file;
	FILE *output_file;

}Config;

Config *Config_new(const char *config);