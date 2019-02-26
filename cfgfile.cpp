#define _CRT_SECURE_NO_DEPRECATE
#if 0

#include "memory_leak.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "cfgfile.h"

//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>

static char *skip_space(char *p)
{
	while (*p == '\t' || *p == ' ') {
		p++;
	}
	return p;
}


static int is_comment_line(char *p)
{
	p = skip_space(p);
	if (*p == '#') {
		return 1;
	}
	else {
		return 0;
	}
}

static int is_blank_line(char *p)
{
	p = skip_space(p);
	if (strchr("\r\n", *p) != NULL) {
		return 1;
	}
	else {
		return 0;
	}
}


static char *get_label_dup(char *p)
{
	size_t ofs = 0;
	p = skip_space(p);
	while (*p != '\0') {
		if (strchr("\t =", p[ofs]) != NULL) {
			char *tmp = (char *)calloc(ofs + 1, sizeof(char));
			strncpy(tmp, p, ofs);
			return tmp;
		}
		else {
			ofs++;
		}
	}
	return NULL;
}


/*
=の右まで進める
*/
static char *skip_label_equal(char *p)
{
	while (*p != '\0') {
		if (*p == '=') {
			p++;
			return p;
		}
		else {
			p++;
		}
	}
	return p;
}
/*
空文字列もあり
ToDo: ""で囲まれた文字列の処理
*/
static char *get_rstring_dup(char *p)
{
	size_t ofs = 0;
	char *tmp;
	/* 左側スペースは飛ばす */
	p = skip_space(p);
	while (*p != '\0' && strchr("\t \r\n", p[ofs]) == NULL) {
		/* 非空白文字数を数える */
		ofs++;
	}
	tmp = (char *)calloc(ofs + 1, sizeof(char));
	strncpy(tmp, p, ofs);
	tmp[ofs] = '\0';
	return tmp;
}

static char *str_valid_check(char *str, char *line_buf)
{
	if (str == NULL || *str == '\0') {
		fprintf(stderr, "str_valid_check failed:\n---------\n%s\n", line_buf);
		exit(EXIT_FAILURE);
	}
	return str;
}

#define BUFSIZE 1024


ConfigFileReader *ConfigFileReader_new(const char *filename)
{
	int i = 0;
	char buf[BUFSIZE];
	ConfigFileReader *cfr = (ConfigFileReader *)calloc(1, sizeof(ConfigFileReader));
	FILE *fp = fopen_errorcheck(filename, "rb");

	/* まず有効行数を数える */
	while (fgets_bin(buf, BUFSIZE, fp) != NULL) {
		char *p = buf;
		p = skip_space(p);
		if (is_comment_line(p) || is_blank_line(p)) {
			continue;
		}
		else {
			cfr->keycount++;
		}
	}

	fseek_errorcheck(fp, 0L, SEEK_SET);
	cfr->keys = (ConfigKey *)calloc(cfr->keycount, sizeof(ConfigKey));

	while (fgets_bin(buf, BUFSIZE, fp) != NULL) {
		char *p = buf;
		p = skip_space(p);
		if (is_comment_line(p) || is_blank_line(p)) {
			continue;
		}
		else {
			char *tmp;
			/* 左辺 */
			tmp = get_label_dup(p);
			str_valid_check(tmp, buf);
			cfr->keys[i].label = tmp;
			/* Todo: ハッシュ */
			p = skip_label_equal(p);
			p = skip_space(p);
			/* 右辺 */
			tmp = get_rstring_dup(p);
			str_valid_check(tmp, buf);
			cfr->keys[i].rstring = tmp;
			/*
			printf("\"%s\"=\"%s\"\n", cfr->keys[i].label, cfr->keys[i].rstring);
			*/
			i++;
		}
	}
	if (cfr->keycount != i) {
		fprintf(stderr, "ConfigFileReader_new: line count mismatch\n");
		exit(EXIT_FAILURE);
	}
	fclose(fp);
	return cfr;
}


void ConfigFileReader_delete(ConfigFileReader *cfr)
{
	int i;
	for (i = 0; i < cfr->keycount; i++) {
		free_nullcheck(cfr->keys[i].label);
		free_nullcheck(cfr->keys[i].rstring);
	}
	free_nullcheck(cfr->keys);
	free_nullcheck(cfr);
}


/*
Todo:ハッシュで検索高速化
でも別に遅くもないかも
*/
static int search_key_index(ConfigFileReader *cfr, const char *label)
{
	int i;
	for (i = 0; i < cfr->keycount; i++) {
		if (strcmp(cfr->keys[i].label, label) == 0) {
			return i;
		}
	}
	fprintf(stderr, "search_key_index: does not have label: %s\n", label);
	exit(EXIT_FAILURE);
}


int ConfigFileReader_get_int(ConfigFileReader *cfr, const char *label)
{
	int idx = search_key_index(cfr, label);
	int v;
	/* ToDo: 0x1111とか */
	if (sscanf(cfr->keys[idx].rstring, "%d", &v) != 1) {
		fprintf(stderr, "ConfigFileReader_get_int: sscanf error\n");
		exit(EXIT_FAILURE);
	}
	return v;
}

long ConfigFileReader_get_long(ConfigFileReader *cfr, const char *label)
{
	int idx = search_key_index(cfr, label);
	int v;
	/* ToDo: 0x1111とか */
	if (sscanf(cfr->keys[idx].rstring, "%d", &v) != 1) {
		fprintf(stderr, "ConfigFileReader_get_long: sscanf error\n");
		exit(EXIT_FAILURE);
	}
	return v;
}

double ConfigFileReader_get_double(ConfigFileReader *cfr, const char *label)
{
	int idx = search_key_index(cfr, label);
	double v;
	if (sscanf(cfr->keys[idx].rstring, "%lf", &v) != 1) {
		fprintf(stderr, "ConfigFileReader_get_double: sscanf error\n");
		exit(EXIT_FAILURE);
	}
	return v;
}

char *ConfigFileReader_get_str_dup(ConfigFileReader *cfr, const char *label)
{
	int idx = search_key_index(cfr, label);
	char *tmp = (char *)calloc(strlen(cfr->keys[idx].rstring) + 1, sizeof(char));
	strcpy(tmp, cfr->keys[idx].rstring);
	return tmp;
}

char *ConfigFileReader_get_str_dup2(ConfigFileReader *cfr, const char *label, char *dir)
{
	int idx = search_key_index(cfr, label);
	char *tmp = (char *)calloc(strlen(cfr->keys[idx].rstring) + strlen(dir) + 1, sizeof(char));
	strcpy(tmp, dir);
	strcat(tmp, cfr->keys[idx].rstring);
	return tmp;
}

const char *ConfigFileReader_get_str(ConfigFileReader *cfr, const char *label)
{
	int idx = search_key_index(cfr, label);
	return cfr->keys[idx].rstring;
}

#if 0
int main(void)
{
	int i;
	ConfigFileReader *cfr = ConfigFileReader_new("test.txt");
	{
		char *tmp;
		printf("%f\n", ConfigFileReader_get_double(cfr, "current_const"));
		tmp = ConfigFileReader_get_str_dup(cfr, "outfile");
		printf("%s\n", tmp);

		for (i = 0; i < 10000; i++) {
			ConfigFileReader_get_str_dup(cfr, "la1998");
		}

		tmp = ConfigFileReader_get_str_dup(cfr, "la1993");
		printf("%s\n", tmp);
		tmp = ConfigFileReader_get_str_dup(cfr, "la1994");
		printf("%s\n", tmp);
		tmp = ConfigFileReader_get_str_dup(cfr, "la1995");
		printf("%s\n", tmp);
		tmp = ConfigFileReader_get_str_dup(cfr, "la1996");
		printf("%s\n", tmp);
		tmp = ConfigFileReader_get_str_dup(cfr, "la1997");
		printf("%s\n", tmp);
		tmp = ConfigFileReader_get_str_dup(cfr, "la1998");
		printf("%s\n", tmp);
	}
	ConfigFileReader_delete(cfr);
	return 0;
}
#endif
