#pragma once
/*おそらくエラーチェック&メモリ確保用*/

#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <assert.h>
#include <float.h>
#include <time.h>

/* #include "memory_leak.h" */

FILE *fopen_errorcheck(const char *path, const char *mode);
/*ディレクトリ指定機能付きfopen_check*/
FILE *fopen_errorcheck2(const char *path, const char *mode, char *dir);
/* ファイルが開けないときはNULLをreturen(小川static_design用) */
FILE *fopen_no_errorcheck(const char *path, const char *mode);
/*連番機能つき*/
FILE *fopen_errorcheck3(const char *path, const char *mode, int idx, char *extensions);
/* 改行コード変換機能つきfgets */
char *fgets_bin(char *s, int size, FILE *stream);
int fseek_errorcheck(FILE *stream, long offset, int whence);

/* 2箇所にfprintf */
int ffprintf(FILE *fp1, FILE *fp2, const char *format, ...);
/* 3箇所にfprintf */
int fffprintf(FILE *fp1, FILE *fp2, FILE *fp3, const char *format, ...);

char *strdup1(const char *s);

long lmax(long a, long b);
long lmin(long a, long b);

double strtod_errorcheck(const char *nptr, const char **endptr);

void *calloc_errorcheck_(size_t nmemb, size_t size, const char *f, int l);
#define calloc_errorcheck(nmemb, size) calloc_errorcheck_(nmemb, size, __FILE__, __LINE__)

void free_nullcheck_(void *x, const char *f, int l);
#define free_nullcheck(x) free_nullcheck_(x, __FILE__, __LINE__)

/* 動的な2次元配列(もどき)の確保 */
void **alloc2D_(size_t size, unsigned int n, unsigned int m, const char *f, int l);
#define alloc2D(size, n, m) alloc2D_(size, n, m, __FILE__, __LINE__)
/* 動的な2次元配列(もどき)の解放 */
void free2D_(void **a, int n, const char *f, int l);
#define free2D(a, n) free2D_(a, n, __FILE__, __LINE__)

#endif /* MISC_H */
