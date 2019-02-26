#pragma once
/*�����炭�G���[�`�F�b�N&�������m�ۗp*/

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
/*�f�B���N�g���w��@�\�t��fopen_check*/
FILE *fopen_errorcheck2(const char *path, const char *mode, char *dir);
/* �t�@�C�����J���Ȃ��Ƃ���NULL��returen(����static_design�p) */
FILE *fopen_no_errorcheck(const char *path, const char *mode);
/*�A�ԋ@�\��*/
FILE *fopen_errorcheck3(const char *path, const char *mode, int idx, char *extensions);
/* ���s�R�[�h�ϊ��@�\��fgets */
char *fgets_bin(char *s, int size, FILE *stream);
int fseek_errorcheck(FILE *stream, long offset, int whence);

/* 2�ӏ���fprintf */
int ffprintf(FILE *fp1, FILE *fp2, const char *format, ...);
/* 3�ӏ���fprintf */
int fffprintf(FILE *fp1, FILE *fp2, FILE *fp3, const char *format, ...);

char *strdup1(const char *s);

long lmax(long a, long b);
long lmin(long a, long b);

double strtod_errorcheck(const char *nptr, const char **endptr);

void *calloc_errorcheck_(size_t nmemb, size_t size, const char *f, int l);
#define calloc_errorcheck(nmemb, size) calloc_errorcheck_(nmemb, size, __FILE__, __LINE__)

void free_nullcheck_(void *x, const char *f, int l);
#define free_nullcheck(x) free_nullcheck_(x, __FILE__, __LINE__)

/* ���I��2�����z��(���ǂ�)�̊m�� */
void **alloc2D_(size_t size, unsigned int n, unsigned int m, const char *f, int l);
#define alloc2D(size, n, m) alloc2D_(size, n, m, __FILE__, __LINE__)
/* ���I��2�����z��(���ǂ�)�̉�� */
void free2D_(void **a, int n, const char *f, int l);
#define free2D(a, n) free2D_(a, n, __FILE__, __LINE__)

#endif /* MISC_H */
