/*
�u�����w�K�Ɛ[�w�w�K C����ɂ��V�~�����[�V�����v�����Ƃɐ݌v
NN���������ăp�X�̌o�H��ݒ�
�i�t�@�C�o���݂Ȃ��A�����I�A1stBlocking�܂ł̃p�X���ŕ]���j
-------------------------------------
NN_ver1
3�w��NN���쐬
�����w�K�̎���
-------------------------------------
NN_ver2
���[�g���e�[�u���̓ǂݍ���
NN�̏d�݂̕]���֐��F�e�X�g�p�@���ʂ��ǂ��Ȃ�
NN�R���t�B�O�̓ǂݍ���
-------------------------------------
NN_ver3
���l���p�X�{���łȂ��A�ŒZ�z�b�v���̍��v�ɂ���
�w�K�f�[�^��ۑ�����R���t�B�O�ƒ��g�i�������j
-------------------------------------
NN_ver4
�������񊮐��i�g���ϊ�����j
-------------------------------------
NN_ver5
�g���ϊ��Ȃ�����������
�w�K���s�����o�@�C��
�w�K�X�V�@
-------------------------------------
NN_ver7
���t�@�N�^�����O
�E�����w�K�Ȃ�
�Eevaluate����
�j���[�����̊֐��̐ݒ���ȒP�ɂ���
-------------------------------------
NN_ver8
�K�i�x�g���[�X���������(�L�����Z��)
�������Ǘ�
���t�@�N�^�����O
-------------------------------------
NN_ver9
���w��
�g���t�B�b�N�s�ψ�i��ɏW���̂݁j
�X�V�̕�V���P�Œ�łȂ��A�ŒZ�o�H���Ƃ���
-------------------------------------
NN_ver10
������ route_table_index�̓��� select_next_state�p
���w�������̖��̔����y�яC��
�������̂��߁A�������̃p�����[�^�𑀍삵�₷�������B
�X�e�b�v�T�C�Y�p�����[�^�̃m�������K���A���Ԍ����̓���
���t�@�N�^�����O
-------------------------------------
NN_ver11
�g�|���W�@JPN12
�d�݂��t�@�C���ɏo�́A�܂��͓��͂Ƃ��� read_file
���t�@�N�^�����O
-------------------------------------
NN_ver12
�g�|���W�@JPN25
�_�C�N�X�g���@�𓱓��@
DijkNN�i��@D�j�̒ǉ�
-------------------------------------
NN_ver13
�o�H�̊Ԉ�����ǉ�
���t�@�N�^�����O
�EAgent�����ɂ��ANN��R&A�𕪗�
-------------------------------------
NN_ver14
���t�@�N�^�����O
1�g��/�t�@�C�o�̔g�����C����ςݏd�˂�݌v�v�z�Ɉڍs
�e�g�����C���ɂ�����e�ʂ́u�ȑO�܂ł�Freq�v���@�u�t�@�C�o�t�@�C���̒l�v

�[�[�[�[�[�[�[�[�[�[�[�[�[�[
NN_ver15
���t�@�N�^�����O

�[�[�[�[�[�[�[�[�[�[�[�[�[�[
NN_ver16
�g�����C���@�\�ǉ�

�[�[�[�[�[�[�[�[�[�[�[�[�[�[
NN_ver17
NSFNET16
1/100�u���b�L���O
�_�C�N�X�g���@�g���I�𕔂̏C��
���t�@�N�^�����O

�[�[�[�[�[�[�[�[�[�[�[�[�[�[
NN_ver18
NN95�̍������\�̗��R��
�˔g���I��@������Ă���
�@�C��
 Dijk�̔g���I��@�����l��i�~�j����FF�ɂ���
 
�[�[�[�[�[�[�[�[�[�[�[�[�[�[
NN_ver20
�����Ɍo�H�I�����Ă���g���I��������
Ver�P�WBase
�[�[�[�[�[�[�[�[�[�[�[�[�[�[
NN_ver21
�폜�̓���
�[�[�[�[�[�[�[�[�[�[�[�[�[�[
NN_ver22
���t�@�N�^�����O
��������
�[�[�[�[�[�[�[�[�[�[�[�[�[�[
NN_ver23
�폜�̍�����
�[�[�[�[�[�[�[�[�[�[�[�[�[
NN_ver24~
Git�ɂ��Ǘ����n�߂�


�[�[�[�[�[�[�[�[�[�[�[�[�[�[
����
�ÃO���[�f�B�[�ˊm�����z�@�A�N�^�[�N���e�B�b�N
random traffic
�o�C�A�X�̋����̊m�F�@����
�w�K�ߒ��o��
�g�����i�K�I�w�K�@

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

	srand((unsigned int)time(NULL) / 2);

	Config *cfg;
	const char *config;
	if (argc > 1)
		config = argv[1];
	else
		config = "config.txt";
	cfg = Config_new(config);

	/*�t�@�C���ǂݍ���*/
	MATRIX *fiber_data = Load_Matrix(cfg->fiber_file);
	MATRIX *route_table = Load_Matrix(cfg->route_table_file);
	cfg->input_num = route_table->row_num - 3;
	/*�o�H���̊Ԉ���*/
	route_table = reduce_routeCandidate(route_table, cfg);
	Agent *agent = init_agent(route_table, fiber_data, cfg);
	NN *nn = NN_new(cfg);
	NN *nn_save = NN_new(cfg);
	/*���ʂ�ێ����鏀��*/
	double res_before = 0.0, res_initial = 0.0, res_NN = 0.0, res_NN_dijk = 0.0, res_conv = 0.0, res_dijk = 0.0, tmp_before = 0.0, tmp_initial = 0.0, tmp_NN = 0.0, tmp_NN_dijk = 0.0, tmp_dijk = 0.0, tmp_conv = 0.0;
	
	/*�m�F�p*/
	if (cfg->read_weight == 1) {
		MATRIX *i_table = Load_Matrix(cfg->read_weight_i_file);
		MATRIX *h_table = Load_Matrix(cfg->read_weight_h_file);
		MATRIX *o_table = Load_Matrix(cfg->read_weight_o_file);
		/*�Ƃ肠�����A�B��w1�̂�*/
		read_weight(nn, i_table, o_table, cfg);
		//double tmp = evaluate(agent, nn, 0, cfg);
		getchar();
		return 0;
	}

	/*���ʂ̕��ω��̂��߁ASim_num��@�s������ max �����ʂƂ���*/
	/*��@C*/
	for (int sim = 0; sim < cfg->sim_num; sim++) {
		/*�d�݂̏�����*/
		init_NN(nn, cfg);
		if (cfg->outputStatus == 1)printf("NN         - ");
		//learn(nn, agent, MAIN_LEARN, cfg);		/*�{�w�K*/
		tmp_NN = evaluate(agent, nn, 0, cfg);
		if (cfg->outputStatus == 0)printf("%d, %.3lf\n", sim, tmp_NN);
		if (res_NN > tmp_NN) {
			res_NN = tmp_NN;
			NN_copy(nn, nn_save, cfg);
		}
		/*��Еt��*/
		fprintf(cfg->action_process_file, "\n");
	}
	if (cfg->outputStatus == 1)printf("res   -   %.3lf\n\n", res_NN);
	output_weight(nn, cfg);


	if (cfg->outputStatus == 1)printf("convention - ");
	res_conv = evaluate(agent, nn, 1, cfg);
	if (cfg->outputStatus == 1)printf("dijkstra   - ");
	res_dijk = evaluate(agent, nn, 2, cfg);
	if (cfg->outputStatus == 1)printf("\n");

	/*�t�@�C���Еt��*/
	fclose(cfg->action_process_file);
	fclose(cfg->blocking_link_hist_file);
	fclose(cfg->capacity_transition_file);
	fclose(cfg->accuracy_learn_file);

	/*�o��*/
	if (cfg->outputStatus == 1)printf("random        - %.3lf\ninitial_learn - %.3lf\nmain_learn    - %.3lf\nconvention    - %.3lf\n", res_before / cfg->sim_num, res_initial / cfg->sim_num, res_NN, res_conv);
	else {
		fprintf(stderr, "%d, %d, %ld, %d, %d, %d, %d, %d, %d, %d, %d, "
			, cfg->topology, cfg->sim_num, cfg->main_train_time, cfg->fiber_basis_num, cfg->freq_num, cfg->central_node, cfg->hidden_layer_num, cfg->hidden_num, cfg->hop_slug, cfg->route_candidate, cfg->delete_frequency);
		fprintf(stderr, "%.1lf, %.1lf, %.1lf, %.3lf, %.3lf, %.3lf\n"
			, res_conv, res_dijk, res_NN, res_dijk / res_conv, res_NN / res_conv, res_NN / res_dijk);
	}
	fprintf(cfg->output_file, "%d, %d, %ld, %d, %d, %d, %d, %d, %d, %d, %d, "
		, cfg->topology, cfg->sim_num, cfg->main_train_time, cfg->fiber_basis_num, cfg->freq_num, cfg->central_node, cfg->hidden_layer_num, cfg->hidden_num, cfg->hop_slug, cfg->route_candidate, cfg->delete_frequency);
	fprintf(cfg->output_file, "%.3lf, %.3lf, %.3lf, %lf, %lf, %lf\n"
		, res_conv, res_dijk, res_NN, res_dijk / res_conv, res_NN / res_conv, res_NN / res_dijk);

	getchar();
	NN_delete(nn, cfg);
	NN_delete(nn_save, cfg);

	return 0;
}


