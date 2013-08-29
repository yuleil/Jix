#ifndef USR_H
#define USR_H

#include "msg.h"
#include "const.h"
#include "def.h"



int     open(char *path, int mode);
void    close(int fd);
int     read(int fd, void *dst, u32 cnt);
int     write(int fd, void *src, u32 cnt);
int     chdir(char *path);
int     dup(int fd);
int     link(char *n, char *target);
int     unlink(char *path);
int     mkdir(char *path);
int     mknod(char *path, short major, short minor);
int     pipe(int *fd);
struct  stat;
int     fstat(int fd, struct stat *sp);
int     seek(int fd, u32 off);
void    gotoxy(u32 x, u32 y);
void    ttymode(int mode);
void    ttycolor(int back, int front);
void    cls(void);

int fork(void);
int exec(char *path, char *argv[]);
void exit(int ret);
int wait(int *state, int nohang);
int sbrk(u32);

void *malloc(u32);
void free(void *);

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

#define TTY_ECHO 0x1
#define TTY_LINE 0x2

#endif // USR_H
