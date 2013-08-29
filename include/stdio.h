#ifndef STDIO_H
#define STDIO_H

#ifndef NULL
#define NULL ((void *)0)
#endif

#define OPEN_MAX 20
#define BUFSIZ 1000
#define EOF (-1)

struct _IO_FILE {
    char        *buf;
    char        *ptr;
    int         cnt;
    short       flag;
    short       bufmode;
    int         fd;
};

typedef struct _IO_FILE FILE;

extern FILE _iob[OPEN_MAX];

#define stdin   (&_iob[0])
#define stdout  (&_iob[1])
#define stderr  (&_iob[2])

#define _IOFBF 0 		/* Fully buffered.  */
#define _IOLBF 1		/* Line buffered.  */
#define _IONBF 2		/* No buffering.  */

enum _flag {_READ = 01, _WRITE = 02, _EOF = 010, _ERR = 020};



#define feof(f)     (((f)->flag & _EOF)     != 0)
#define ferror(f)   (((f)->flag & _ERROR)   != 0)
#define fileno(f)   ((f)->fd)
#define clearerr(f) ((f)->flag &= ~(_EOF|_ERR))

#define getchar()   fgetc(stdin)
#define putchar(x)  fputc((x), stdout)

FILE *fopen(char *name, char *mode);
void fclose(FILE *fp);
void fflush(FILE *fp);
int fputc(char c, FILE *fp);
int fill(FILE *fp);
int fgetc(FILE *fp);
char *fgets(char *s, int len, FILE *fp);
int fputs(char *s, FILE *fp);
void goto_xy(int, int);
void tty_color(int, int);

#include "stdarg.h"
int vsprintf(char *buf, const char *fmt, va_list args);
int printf(const char *fmt, ...);
int fprintf(FILE *, const char *, ...);
#endif // STDIO_H
