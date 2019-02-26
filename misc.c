/*�����炭�G���[�`�F�b�N&�������m�ۗp*/

#define _CRT_SECURE_NO_DEPRECATE

/*
�G���[�ł��炷�����ʃ��b�p�[�Ƃ�
�Ȃ񂩂��낢��
*/
#if 0
#include "memory_leak.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <float.h>
#include <stddef.h>

#include "misc.h"

//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>

#ifdef MISC_DEBUG_ALLOC_ON
static FILE *fp_dbgalc = NULL;
void dbg_fp_init(void)
{
	if (!fp_dbgalc) {
		fp_dbgalc = fopen_errorcheck("dbgalc.txt", "wt");
		/* fclose�͂߂�ǂ��̂ł��Ȃ� */
	}
}
#endif


void *calloc_errorcheck_(size_t nmemb, size_t size, const char *f, int l)
{
	void *x = calloc(nmemb, size);
	if (x == NULL) {
		fprintf(stderr, "calloc error\n");
		exit(EXIT_FAILURE);
	}
#ifdef MISC_DEBUG_ALLOC_ON
	dbg_fp_init();
	fprintf(fp_dbgalc, "%s %d alloced %p\n", f, l, x);
#endif
	return x;
}

void free_nullcheck_(void *x, const char *f, int l)
{
	if (x != NULL) {
		free(x);
		x = NULL;
#ifdef MISC_DEBUG_ALLOC_ON
		dbg_fp_init();
		fprintf(fp_dbgalc, "%s %d freed %p\n", f, l, (void *)x);
#endif
	}
}

/*�`�F�b�N���t�@�C���I�[�v��*/
FILE *fopen_errorcheck(const char *path, const char *mode)
{
	FILE *fp = fopen(path, mode);
	if (fp == NULL) {
		fprintf(stderr, "fopen error: %s\n", path);
		exit(EXIT_FAILURE);
	}
	return fp;
}
/* �t�@�C�����J���Ȃ��Ƃ���NULL��returen(����static_design�p) */
FILE *fopen_no_errorcheck(const char *path, const char *mode)
{
	FILE *fp = fopen(path, mode);
	return fp;
}

/*�f�B���N�g���w��@�\���`�F�b�N���t�@�C���I�[�v��*/
FILE *fopen_errorcheck2(const char *path, const char *mode, char *dir)
{
	FILE *fp;
	char *tmp = (char *)calloc_errorcheck(strlen(path) + strlen(dir) + 1, sizeof(char));
	strcpy(tmp, dir);
	strcat(tmp, path);
	fp = fopen(tmp, mode);
	if (fp == NULL) {
		fprintf(stderr, "fopen error: %s%s\n",dir ,path);
		exit(EXIT_FAILURE);
	}
	free_nullcheck(tmp);
	return fp;
}

/*�A�ԋ@�\���`�F�b�N���t�@�C���I�[�v��*/
FILE *fopen_errorcheck3(const char *path, const char *mode, int idx , char *extensions)
{
	FILE *fp;
	char *idx_c =(char *)calloc_errorcheck(strlen("64") + 1, sizeof(char));
	char *tmp = (char *)calloc_errorcheck(strlen(path) + strlen("64")+ strlen(extensions) + 1, sizeof(char));
	sprintf(idx_c,"%.2d",idx);

	strcpy(tmp, path);
	strcat(tmp, idx_c);
	strcat(tmp, extensions);
	fp = fopen(tmp, mode);
	if (fp == NULL) {
		fprintf(stderr, "fopen error : %s%s%s\n", path, idx_c, extensions);
		exit(EXIT_FAILURE);
	}
	free_nullcheck(tmp);
	return fp;
}

/* sprintf�������ʂ��t�@�C�����Ƃ��ėp����fopen */
FILE *fopenprintf_errorcheck(const char *mode, const char *format, ...)
{
#define BUFSIZE 1500
	char buf[BUFSIZE];
	va_list ap;
	va_start(ap, format);
	vsprintf(buf, format, ap); /* ToDo: �o�b�t�@�I�[�o�t���[�H */
	va_end(ap);
	return fopen_errorcheck(buf, mode);
#undef BUFSIZE
}

int fseek_errorcheck(FILE *stream, long offset, int whence)
{
	int i;
	i = fseek(stream, offset, whence);
	if (i != 0) {
		fprintf(stderr, "fseek error\n");
		exit(EXIT_FAILURE);
	}
	return i;
}

char *fgets_bin(char *s, int size, FILE *stream)
{
	int i = 0;
	int c;
	c = getc(stream);
	if (c == EOF) {
		return NULL;
	}
	do {
		if (i >= size - 1) {
			fprintf(stderr, "fgets_bin buffer overflow\n");
			exit(EXIT_FAILURE);
		}
		if (c == 0x0a) {
			/* LF */
			s[i++] = '\n';
			break;
		} else if (c == 0x0d) {
			/* CR */
			c = getc(stream);
			if (c == 0x0a) {
				/* CRLF */
			} else {
				/* CR */
				ungetc(c, stream);
			}
			s[i++] = '\n';
			break;
		} else {
			s[i++] = (char)c;
		}
	} while ((c = getc(stream)) != EOF);
	s[i] = '\0';
	return s;
}


int ffprintf(FILE *fp1, FILE *fp2, const char *format, ...)
{
	int i;
	va_list ap;
	va_start(ap, format);
	vfprintf(fp1, format, ap);

	/* ���L��2�s���Ȃ���Opteron�Ńo�O�� */
	va_end(ap);
	va_start(ap, format);

	i = vfprintf(fp2, format, ap);
	va_end(ap);
	return i;
}


int fffprintf(FILE *fp1, FILE *fp2, FILE *fp3, const char *format, ...)
{
	int i;
	va_list ap;

	va_start(ap, format);
	vfprintf(fp1, format, ap);
	va_end(ap);

	va_start(ap, format);
	vfprintf(fp2, format, ap);
	va_end(ap);

	va_start(ap, format);
	i = vfprintf(fp3, format, ap);
	va_end(ap);

	return i;
}


char *strdup1(const char *s)
{
	char *dst = (char *)calloc_errorcheck(strlen(s) + 1, sizeof(char));
	strcpy(dst, s);
	return dst;
}

long lmax(long a, long b)
{
	return a > b? a: b;
}

long lmin(long a, long b)
{
	return a < b? a: b;
}

/*
nptr����double���X�L����
����������endptr�̓X�L������̈ʒu�������悤�ɕύX�����
*/
double strtod_errorcheck(const char *nptr, const char **endptr)
{
	double x;
	x = strtod(nptr, (char **)endptr); /* �R���p�C����ق点��(�����̂��H) */
	if (nptr == *endptr) {
		fprintf(stderr, "strtod error\n");
		exit(EXIT_FAILURE);
	}
	/* TODO: �X���b�h�Z�[�t�H */
	if (errno == ERANGE) {
		fprintf(stderr, "strtod overflow\n");
		exit(EXIT_FAILURE);
	}
	return x;
}


/* ���I��2�����z��(���ǂ�)�̊m�� */
void **alloc2D_(size_t size, unsigned int n, unsigned int m, const char *f, int l)
{
	unsigned int i;
	void **temp;
	temp = (void **)calloc/*_errorcheck_*/(n, sizeof(void *)/*, f, l*/);
	for (i = 0; i < n; i++) {
		temp[i] = (void *)calloc/*_errorcheck_*/(m, size/*, f, l*/);
	}
	return temp;
}


/* ���I��2�����z��(���ǂ�)�̉�� */
void free2D_(void **a, int n, const char *f, int l)
{
	int i;
	for (i = 0; i < n; i++) {
		free_nullcheck_(a[i], f, l);
	}
	free_nullcheck_(a, f, l);
}


#if 0
#define BUFSIZE 1024
int main(void)
{
	char *hoge;
	FILE *fp;
	char p[BUFSIZE];
	hoge = (char *)calloc_errorcheck(123, 1);
	fp = fopen_errorcheck("test.txt", "rt");

	while (fgets_bin(p, BUFSIZE, fp) != NULL) {
		printf("%s", p);
	}

	fclose(fp);
	free_nullcheck(hoge);

	{
		char **moga = (char **)alloc2D(sizeof(char), 5, 8);
		moga[4][0] = 'u';
		printf("%s", moga[4]);
		free2D((void **)moga, 5);
	}

	return 0;
}
#endif
