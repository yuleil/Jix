#include "include/stdio.h"
#include "usr.h"


FILE _iob[OPEN_MAX] = {
    {NULL, NULL, 0, _READ, _IOLBF, 0},
    {NULL, NULL, BUFSIZ, _WRITE, _IOLBF, 1},
    {NULL, NULL, BUFSIZ, _WRITE, _IONBF, 2},
};


FILE *
fopen(char *name, char *mode)
{
    int fd;
    FILE *fp;

    if (*mode != 'r' && *mode != 'w')
        return NULL;

    for (fp = _iob; fp < _iob + OPEN_MAX; fp++)
        if ((fp->flag & (_READ | _WRITE)) == 0)
            break;
    if (fp == _iob + OPEN_MAX)
        return NULL;

    switch (*mode) {
        case 'w':
            fd = open(name, O_WRONLY | O_CREATE);
            break;
        case 'r':
            fd = open(name, O_RDONLY);
            break;
    }
    if (fd < 0)
        return NULL;

    fp->fd = fd;
    fp->cnt = *mode == 'r' ? 0 : BUFSIZ;
    fp->buf = NULL;
    fp->flag = *mode == 'r' ? _READ : _WRITE;
    fp->bufmode = _IOFBF;
    fp->ptr = fp->buf = NULL;

    return fp;
}

void
fclose(FILE *fp)
{
    close(fp->fd);
    if (fp->buf)
        free(fp->buf);
    fp->flag = 0;
}

void
fflush(FILE *fp)
{
    if (!fp->buf) {
        fp->ptr = fp->buf = malloc(1024);
        fp->cnt = 0;
        return;
    }

    if (fp->flag & _WRITE && fp->cnt)
        fp->flag |= write(fp->fd, fp->buf, fp->cnt) == fp->cnt ? 0 : _ERR;
    fp->cnt = 0;
    fp->ptr = fp->buf;
}


int
fputc(char c, FILE *fp)
{
    if ((fp->flag & (_ERR|_EOF|_WRITE)) != _WRITE)
        return -1;

    switch (fp->bufmode) {
        case _IONBF:
            fp->flag |= write(fp->fd, &c, 1) == 1 ? 0 : _ERR;
            break;
        case _IOLBF:
            if (fp->cnt >= BUFSIZ)
                fflush(fp);
            fp->cnt++;
            *fp->ptr++ = c;
            if (c == '\n')
                fflush(fp);
            break;
        case _IOFBF:
            if (fp->cnt >= BUFSIZ)
                fflush(fp);
            fp->cnt++;
            *fp->ptr++ = c;
            break;
        default:
            fp->flag |= _ERR;
    }

    return (unsigned char)c;
}


int
ffill(FILE *fp)
{
    if (!fp->buf)
        fp->ptr = fp->buf = malloc(BUFSIZ);

    if (!(fp->flag & _READ))
        return EOF;
    if ((fp->cnt = read(fp->fd, fp->buf, BUFSIZ)) < 0)
        fp->flag |= _ERR;
    else if (fp->cnt == 0)
        fp->flag |= _EOF;

    fp->ptr = fp->buf;

    return fp->cnt-- > 0 ? (unsigned char)*fp->ptr++ : EOF;
}

int
fgetc(FILE *fp)
{
    if (fp == stdin && stdout->cnt)
        fflush(stdout);


    if ((fp->flag & (_ERR|_EOF|_READ)) != _READ)
        return EOF;

    if (fp->cnt <= 0)
        return ffill(fp);

    fp->cnt--;
    return (unsigned char)*fp->ptr++;
}

char *
fgets(char *s, int len, FILE *fp)
{
    int n;

    for (n = 0; n < len; n++)
        if ((s[n] = fgetc(fp)) == '\n' || s[n] == EOF)
            break;


    if (s[0] == EOF)
        return NULL;

    s[s[n] == EOF ? n : n + 1] = '\0';

    return s;
}

int
fputs(char *s, FILE *fp)
{
    int n;

    if (fp->bufmode == _IONBF) {
        for (n = 0; s[n]; n++)
            ;
        fp->flag |= write(fp->fd, s, n) == n ? _ERR : 0;
        return fp->flag & _ERR ? -1 : n;
    }

    for (n = 0; s[n]; n++)
        if (fputc(s[n], fp) == -1)
            return n;

    return n;
}

#include "stdarg.h"

int
fprintf(FILE *fp, const char *fmt, ...)
{
    va_list ap;
    char buf[BUFSIZ/2];

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

    return fputs(buf, fp);
}

int
printf(const char *fmt, ...)
{
    va_list ap;
    char buf[BUFSIZ/2];

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

    return fputs(buf, stdout);
}


void
panic(char *s)
{
    fprintf(stderr,s);
    exit(-1);
}

void
goto_xy(int x, int y)
{
    fflush(stdout);
    gotoxy(x, y);
}

void
tty_color(int back, int front)
{
    fflush(stdout);
    ttycolor(back, front);
}
