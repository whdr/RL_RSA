/*
「強化学習と深層学習 C言語によるシミュレーション」をもとに設計
NNをもちいてパスの経路を設定
（ファイバ増設なし、準動的、1stBlockingまでのパス数で評価）
-------------------------------------
NN_ver1
3層のNNを作成
初期学習の実装
-------------------------------------
NN_ver2
ルート候補テーブルの読み込み
NNの重みの評価関数：テスト用　結果が良くない
NNコンフィグの読み込み
-------------------------------------
NN_ver3
価値をパス本数でなく、最短ホップ数の合計にする
学習データを保存するコンフィグと中身（未実装）
-------------------------------------
NN_ver4
いったん完成（波長変換あり）
-------------------------------------
NN_ver5
波長変換なし実現を検討
学習失敗が発覚　修正
学習更新法
-------------------------------------
NN_ver7
リファクタリング
・初期学習なし
・evaluate統一
ニューロンの関数の設定を簡単にする
-------------------------------------
NN_ver8
適格度トレースを取り入れる(キャンセル)
メモリ管理
リファクタリング
-------------------------------------
NN_ver9
多層化
トラフィック不均一（一極集中のみ）
更新の報酬を１固定でなく、最短経路長とする
-------------------------------------
NN_ver10
高速化 route_table_indexの導入 select_next_state用
多層化処理の問題の発見及び修復
効率化のため、いくつかのパラメータを操作しやすくした。
ステップサイズパラメータのノルム正規化、時間減衰の導入
リファクタリング
-------------------------------------
NN_ver11
トポロジ　JPN12
重みをファイルに出力、または入力とする read_file
リファクタリング
-------------------------------------
NN_ver12
トポロジ　JPN25
ダイクストラ法を導入　
DijkNN（手法D）の追加
-------------------------------------
NN_ver13
経路の間引きを追加
リファクタリング
・Agent導入により、NNとR&Aを分離
-------------------------------------
NN_ver14
リファクタリング
1波長/ファイバの波長レイヤを積み重ねる設計思想に移行
各波長レイヤにおける容量は「以前までのFreq」＊　「ファイバファイルの値」

ーーーーーーーーーーーーーー
NN_ver15
リファクタリング

ーーーーーーーーーーーーーー
NN_ver16
波長レイヤ機能追加

ーーーーーーーーーーーーーー
NN_ver17
NSFNET16
1/100ブロッキング
ダイクストラ　波長選択部の修正
リファクタリング

ーーーーーーーーーーーーーー
NN_ver18
NN95の高い性能の理由解明
⇒波長選択法が誤っていた
　修正
 Dijkの波長選択法を価値基準（×）からFFにした

ーーーーーーーーーーーーーー
NN_ver20
さきに経路選択してから波長選択をする
Ver１８Base
ーーーーーーーーーーーーーー
NN_ver21
削除の導入
ーーーーーーーーーーーーーー
NN_ver22
リファクタリング
微高速化
ーーーーーーーーーーーーーー
NN_ver23
削除の高速化
ーーーーーーーーーーーーー
NN_ver24~
Gitによる管理を始める


ーーーーーーーーーーーーーー
今後
εグリーディー⇒確率分布　アクタークリティック
random traffic
バイアスの挙動の確認　正負
学習過程出力
波長数段階的学習法

*/

#define _CRT_SECURE_NO_WARNINGS

#if 0
#include "memory_leak.h"
#endif

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "agent.h"

int main(int argc, const char *argv[]) {

	Config *cfg;
	const char *config;
	if (argc > 1)
		config = argv[1];
	else
		config = "config.txt";
	cfg = Config_new(config);
	srand((unsigned int)time(NULL) / 2 + cfg->flu_proportion + (unsigned int)cfg->main_train_time / 100 + cfg->central_node);

	/*ファイル読み込み*/
	MATRIX *fiber_data = Load_Matrix(cfg->fiber_file);
	MATRIX *route_table = Load_Matrix(cfg->route_table_file);
	cfg->input_num = route_table->row_num - 3;
	route_table = reduce_routeCandidate(route_table, cfg);/*経路候補の間引き*/
	Agent *agent = init_agent(route_table, fiber_data, cfg);
	NN *nn = NN_new(cfg);
	NN *nn_save = NN_new(cfg);
	/*結果を保持する準備*/
	double res_NN = 1000.0, res_conv = 10000.0, res_dijk = 10000.0, tmp_NN, tmp_dijk, tmp_conv;

	/*確認用*/
	if (cfg->read_weight == 1) {
		MATRIX *i_table = Load_Matrix(cfg->read_weight_i_file);
		MATRIX *h_table = Load_Matrix(cfg->read_weight_h_file);
		MATRIX *o_table = Load_Matrix(cfg->read_weight_o_file);
		/*とりあえず、隠れ層1のみ*/
		read_weight(nn, i_table, o_table, cfg);
		//double tmp = evaluate(agent, nn, 0, cfg);
		getchar();
		return 0;
	}

	/*結果の平均化のため、Sim_num回　行いその max を結果とする*/
	/*手法C*/
	for (int sim = 0; sim < cfg->sim_num; sim++) {
		init_NN(nn, cfg);/*重みの初期化*/
		if (cfg->outputStatus == 1)printf("NN         - ");
		learn(nn, agent, MAIN_LEARN, cfg);	/*本学習*/
		tmp_NN = evaluate(agent, nn, 0, cfg);/*評価*/
		if (cfg->outputStatus == 0)printf("%d, %.2lf, %.6lf\n", sim, cfg->flu_proportion, tmp_NN);
		if (res_NN > tmp_NN) {
			res_NN = tmp_NN;
			NN_copy(nn, nn_save, cfg);
		}
		/*後片付け*/
		fprintf(cfg->action_process_file, "\n");
	}
	if (cfg->outputStatus == 1)printf("res   -   %.3lf\n\n", res_NN);
	output_weight(nn, cfg);

	for (int sim = 0; sim < 3; sim++) {
		if (cfg->outputStatus == 1)printf("convention - ");
		tmp_conv = evaluate(agent, nn, 1, cfg);
		if (res_conv > tmp_conv)
			res_conv = tmp_conv;
	}

	for (int sim = 0; sim < 3; sim++) {
		if (cfg->outputStatus == 1)printf("dijkstra - ");
		tmp_conv = evaluate(agent, nn, 1, cfg);
		if (res_conv > tmp_conv)
			res_conv = tmp_conv;
	}
	if (cfg->outputStatus == 1)printf("dijkstra   - ");
	res_dijk = evaluate(agent, nn, 2, cfg);
	if (cfg->outputStatus == 1)printf("\n");
	/*ファイル片付け*/
	fclose(cfg->action_process_file);
	fclose(cfg->blocking_link_hist_file);
	fclose(cfg->capacity_transition_file);
	fclose(cfg->accuracy_learn_file);

	/*出力*/
	fprintf(stderr, "%d, %d, %ld, %d, %d, %d, %d, %d, %d, %d, %.2lf, "
		, cfg->topology, cfg->sim_num, cfg->main_train_time, cfg->fiber_basis_num, cfg->freq_num, cfg->central_node, cfg->hidden_layer_num, cfg->hidden_num, cfg->hop_slug, cfg->route_candidate, cfg->flu_proportion);
	fprintf(stderr, "%.6lf, %.6lf, %.6lf\n"
		, res_conv, res_dijk, res_NN);

	fprintf(cfg->output_file, "%d, %d, %ld, %d, %d, %d, %d, %d, %d, %d, %.2lf, "
		, cfg->topology, cfg->sim_num, cfg->main_train_time, cfg->fiber_basis_num, cfg->freq_num, cfg->central_node, cfg->hidden_layer_num, cfg->hidden_num, cfg->hop_slug, cfg->route_candidate, cfg->flu_proportion);
	fprintf(cfg->output_file, "%.12lf, %.12lf, %.12lf\n"
		, res_conv, res_dijk, res_NN);

	if (cfg->outputStatus == 1)getchar();
	NN_delete(nn, cfg);
	NN_delete(nn_save, cfg);

	return 0;
}


