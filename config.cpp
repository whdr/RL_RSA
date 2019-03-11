
#define _CRT_SECURE_NO_DEPRECATE
#if 0
#include "memory_leak.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cfgfile.h"
#include "config.h"
#define _CRT_SECURE_NO_WARNINGS

Config *Config_new(const char *config) {
	Config *cfg;
	char *outdir;
	ConfigFileReader *cfr = ConfigFileReader_new(config);
	cfg = (Config *)calloc/*_errorcheck*/(1, sizeof(Config));
	outdir = (char *)ConfigFileReader_get_str(cfr, "OUTDIR");


	/*ネットワークConfig*/
	cfg->topology = ConfigFileReader_get_int(cfr, "TOPOLOGY");
	cfg->hop_slug = ConfigFileReader_get_int(cfr, "HOP_SLUG");
	cfg->fiber_distribution = ConfigFileReader_get_int(cfr, "FIBER_DISTRIBUTION");
	//if (cfg->fiber_distribution == 0)
	//	cfg->max_fiber_num = 1;
	//else if (cfg->fiber_distribution == 1)
	//	cfg->max_fiber_num = 3;
	cfg->freq_num = ConfigFileReader_get_int(cfr, "FREQ_NUM");
	cfg->selectFreq_mode = ConfigFileReader_get_int(cfr, "SELECTFREQ_MODE");
	cfg->select_priority = ConfigFileReader_get_int(cfr, "SELECT_PRIORITY");

	/*動的*/
	cfg->time = ConfigFileReader_get_int(cfr, "PATH_TIME");/*実験時間*/
	cfg->real_time = ConfigFileReader_get_int(cfr, "REAL_TIME");/*測定開始時間*/
	cfg->flu_proportion = ConfigFileReader_get_double(cfr, "FLU_PROPORTION");/*初期トラフィック期待値の変動割合*/


																			 /*トラフィックConfig*/
	cfg->demand_pattern = ConfigFileReader_get_int(cfr, "DEMAND_PATTERN");
	cfg->central_node = ConfigFileReader_get_int(cfr, "CENTRAL_NODE");
	cfg->central_node_2 = ConfigFileReader_get_int(cfr, "CENTRAL_NODE2");
	cfg->central_degree = ConfigFileReader_get_int(cfr, "CENTRAL_DEGREE");
	cfg->route_candidate = ConfigFileReader_get_int(cfr, "ROUTE_CANDIDATE");

	/*試行Config*/
	cfg->sim_num = ConfigFileReader_get_int(cfr, "SIMULATION_NUM");
	cfg->evaluate_time = ConfigFileReader_get_int(cfr, "EVALUATE_SIM_NUM");

	/*NN用*/
	cfg->hidden_num = ConfigFileReader_get_int(cfr, "HIDDEN_NUM");/*隠れ層のニューロン数*/
	cfg->hidden_layer_num = ConfigFileReader_get_int(cfr, "HIDDEN_LAYER_NUM");/*隠れ層のニューロン数*/
	cfg->hidden_neuron_type = ConfigFileReader_get_int(cfr, "HIDDEN_NEURON");
	cfg->output_neuron_type = ConfigFileReader_get_int(cfr, "OUTPUT_NEURON");
	cfg->block = ConfigFileReader_get_int(cfr, "BLOCK");/*平滑化するときの単位*/
	cfg->main_train_time = ConfigFileReader_get_int(cfr, "MAINTRAIN_TIME");/*全体の学習回数*/
	cfg->alpha = ConfigFileReader_get_double(cfr, "ALPHA");/*学習係数*/
	cfg->alpha_normalization = ConfigFileReader_get_int(cfr, "ALPHA_NORMALIZATION");/*ノルムで正規化*/
	cfg->alpha_attenuation = ConfigFileReader_get_double(cfr, "ALPHA_ATTENUATION");/*時間とともに減衰*/
	cfg->leak = ConfigFileReader_get_double(cfr, "LEAK");/*LeakyReLUの漏れ分*/
	cfg->epsilon = ConfigFileReader_get_double(cfr, "EPSILON");
	cfg->gamma = ConfigFileReader_get_double(cfr, "GAMMA");
	cfg->lambda = ConfigFileReader_get_double(cfr, "LAMBDA");
	cfg->reward = ConfigFileReader_get_double(cfr, "REWARD");

	/*プログラム上のConfig*/
	cfg->output_accuracy = ConfigFileReader_get_int(cfr, "OUTPUT_ACCURACY");
	cfg->output_capacity_graph = ConfigFileReader_get_int(cfr, "OUTPUT_CAPACITY_TRANSITION");
	cfg->outputStatus = ConfigFileReader_get_int(cfr, "OUTPUT_STATUS");
	cfg->read_weight = ConfigFileReader_get_int(cfr, "READ_WEIGHT");


	//cfg->route_k = ConfigFileReader_get_int(cfr, "ROUTE_K");/*入力ニューロン数*/
	char topology_file_name[1024];
	if (cfg->topology == 0) {
		cfg->route_k = 50;
		sprintf(topology_file_name, "short_route/3x3_k%d_hop%d.csv", cfg->route_k, cfg->hop_slug);
		cfg->node_num = 9;
	}
	else if (cfg->topology == 1) {
		cfg->route_k = 30;
		sprintf(topology_file_name, "short_route/jpn12_k%d_hop%d.csv", cfg->route_k, cfg->hop_slug);
		cfg->node_num = 12;

	}
	else if (cfg->topology == 2) {
		cfg->route_k = 100;
		sprintf(topology_file_name, "short_route/jpn25_k%d_hop%d.csv", cfg->route_k, cfg->hop_slug);
		cfg->node_num = 25;

	}
	else if (cfg->topology == 3) {

		cfg->route_k = 100;
		sprintf(topology_file_name, "short_route/NSF16_k%d_hop%d.csv", cfg->route_k, cfg->hop_slug);
		cfg->node_num = 16;
	}
	else if (cfg->topology == 4) {

		cfg->route_k = 100;
		sprintf(topology_file_name, "short_route/cost239_k%d_hop%d.csv", cfg->route_k, cfg->hop_slug);
		cfg->node_num = 19;
	}
	cfg->route_table_file = fopen_errorcheck(topology_file_name, "r");
	cfg->read_weight_i_file = fopen_errorcheck(ConfigFileReader_get_str(cfr, "READ_WEIGHT_I_FILE"), "r");
	cfg->read_weight_h_file = fopen_errorcheck(ConfigFileReader_get_str(cfr, "READ_WEIGHT_H_FILE"), "r");
	cfg->read_weight_o_file = fopen_errorcheck(ConfigFileReader_get_str(cfr, "READ_WEIGHT_O_FILE"), "r");

	cfg->fiber_demand = ConfigFileReader_get_int(cfr, "FIBER_DEMAND");

	char fiber_file_name[1024];
	/*node-num から トポロジ番号に変更する*/
	sprintf(fiber_file_name, "fiber/%d/%d_%d_uni.txt", cfg->freq_num, cfg->central_node, cfg->fiber_demand);
	cfg->fiber_file = fopen_errorcheck(fiber_file_name, "r");
	cfg->accuracy_learn_file = fopen_errorcheck2(ConfigFileReader_get_str(cfr, "ACCURACY_MAINLEARN_FILE"), "w", outdir);
	cfg->capacity_transition_file = fopen_errorcheck2(ConfigFileReader_get_str(cfr, "CAPACITY_TRANSITION_FILE"), "w", outdir);
	cfg->action_process_file = fopen_errorcheck2(ConfigFileReader_get_str(cfr, "ACTION_PROCESS_FILE"), "w", outdir);
	cfg->output_file = fopen_errorcheck2(ConfigFileReader_get_str(cfr, "OUTPUT_FILE"), "w", outdir);
	cfg->weight_i_file = fopen_errorcheck2(ConfigFileReader_get_str(cfr, "WEIGHT_I_FILE"), "w", outdir);
	cfg->weight_h_file = fopen_errorcheck2(ConfigFileReader_get_str(cfr, "WEIGHT_H_FILE"), "w", outdir);
	cfg->weight_o_file = fopen_errorcheck2(ConfigFileReader_get_str(cfr, "WEIGHT_O_FILE"), "w", outdir);
	cfg->step_num_hist_file = fopen_errorcheck2(ConfigFileReader_get_str(cfr, "STEP_NUM_HISTOGRAM_FILE"), "w", outdir);
	cfg->blocking_link_hist_file = fopen_errorcheck2(ConfigFileReader_get_str(cfr, "BLOCKING_LINK_HIST_FILE"), "w", outdir);

	return cfg;
}
