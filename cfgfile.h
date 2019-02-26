#pragma once
/*
#comment
label1=value
label2=value
label3=value
のようなテキストファイルを読み込んで
"label"で参照できるようにする
*/

#ifndef CFGFILE_H
#define CFGFILE_H

#include "misc.h"
/*
ToDo: ラベルの大文字小文字区別する？しない？未定
*/

typedef struct ConfigKey {
	char *label;
	char *rstring;
} ConfigKey;

typedef struct ConfigFileReader {
	ConfigKey *keys;
	int keycount;
} ConfigFileReader;

ConfigFileReader *ConfigFileReader_new(const char *filename);
void ConfigFileReader_delete(ConfigFileReader *cfr);

int ConfigFileReader_get_int(ConfigFileReader *cfr, const char *label);
long ConfigFileReader_get_long(ConfigFileReader *cfr, const char *label);
double ConfigFileReader_get_double(ConfigFileReader *cfr, const char *label);
char *ConfigFileReader_get_str_dup(ConfigFileReader *cfr, const char *label);
char *ConfigFileReader_get_str_dup2(ConfigFileReader *cfr, const char *label, char *dir);
const char *ConfigFileReader_get_str(ConfigFileReader *cfr, const char *label);

#endif /* CFGFILE_H */
