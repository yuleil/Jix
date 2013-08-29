#ifndef DEF_H
#define DEF_H

#define LEN(x)  (sizeof(x)/sizeof((x)[0])) // length of a array
#define offsetof(t, m)  ((int)&((t *)0)->m)
#define max(x, y)       ((x) > (y) ? (x) : (y))
#define min(x, y)       ((x) < (y) ? (x) : (y))

#include "types.h"

struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct stat;
struct superblock;
struct msg;



//panic.h
void            kputs(const char *);
void            panic(char *);
// vm.c
void            seginit(void);
void            kmeminit(void);
void            vmenable(void);
pde_t*          setupkvm(char* (*alloc)());
char*           uva2ka(pde_t*, char*);
u32             allocuvm(pde_t*, u32, u32);
int             deallocuvm(pde_t*, u32, u32);
void            freevm(pde_t*);
void            inituvm(pde_t*, char*, u32);
int             loaduvm(pde_t*, char*, int, u32, u32);
void            switchkvm(void);
int             copyout(pde_t*, u32, void*, u32);
void            clearpteu(pde_t *pgdir, char *uva);
void 			kpageinit(void);
void 			kallocinit();
char 			*kalloc();
void 			kfree(char *);
char 			*kalloc1();
void 			kfree1(char *);
pte_t *         walkpgdir(pde_t *pgdir, const void *va, char *(*alloc)(void));
int             mappages(pde_t *pgdir, void *va, u32 size, u32 pa,
                    int perm, char *(*alloc)(void));

// string.c
int             memcmp(const void*, const void*, u32);
void*           memmove(void*, const void *, u32);
void*           memcpy(void *, const void *, u32);
void*           memset(void*, int, u32);
char*           safestrcpy(char*, const char*, int);
int             strlen(const char*);
int             strncmp(const char*, const char*, u32);
char*           strncpy(char*, const char*, int);
char *          strcpy(char *, const char *);

//trap.c
void    init_8259A();
void    idtinit(void);
void    restart();
void    swtch(struct context **, struct context *);
void    intrinit(void);
//proc.h
struct proc *allocproc(void);
void schedule();
void sched();
void sleep_on(void *);
void wake_up(void *);
void sleep_on1(void *);
void wake_up1(void *);
void    userinit(void);
void    taskinit(void);
//tasks

void task_idle();
void task_sys();
void task_mm();
void task_fs();
//kbd.c
int kbdgetc(void);

//msg.c
int sendrecv(int, struct msg *, int);
int sys_msg(int function, struct msg *, int dest);
int sendmsg(struct msg *, int);
int recvmsg(struct msg *, int);

#define assert(exp) if (!(exp))\
    panic(#exp)

// i8259
void enable_irq(u32);
void disable_irq(u32);
void eoi();
void eoi_s();



// cache.c
struct buf* bread(u32 dev, u32 sector);
void bwrite(struct buf *b);
void brelse(struct buf *b);
void binit();


// ide.c
void ideintr(void);
void iderw(struct buf *);
void ideinit(void);

// tty.c
void tty_init(void);
int tty_gotoxy(u32 x, u32 y);
void tty_setmode(u32 mode);
int tty_write(char *src, u32 n);
int tty_read(char *dst, u32 n);
void ttyintr(void);
void tty_cls(void);
void tty_setcolor(int, int);

// kbd.c
int kbdgetc(void);

//vsprintf.c
#include "stdarg.h"
int vsprintf(char *buf, const char *fmt, va_list args);

// inode.c
void stati(struct inode *, struct stat *);
int readi(struct inode *, char *, u32 , u32);
int writei(struct inode *, char *, u32, u32);
struct inode *icreate(const char *path,
        short type, short major, short minor);
struct inode *namei(const char *path);
struct inode *nameiparent(const char *path, char *name);
int dirlink(struct inode *dp, char *name, u32 inum);
void iput(struct inode *ip);
void iupdate(struct inode *ip);
struct inode * ialloc(u32 dev, short type);
int namecmp(const char *, const char *);
struct inode *dirlookup(struct inode *dp, char *name, u32 *poff/* offset in dir*/);
struct inode *idup(struct inode *);

// pipe.c
int pipealloc(struct file **, struct file **);
void pipeclose(struct pipe *, int writable);
int pipewrite(struct pipe *, char *, u32);
int piperead(struct pipe *, char *, u32);

// file.c
int     filewrite(struct file *f, void *addr, u32 n);
int     fileread(struct file *f, void *addr, u32 n);
int     filestat(struct file *f, struct stat *st);
void    fileclose(struct file *f);
struct file *filealloc(void);
struct file *filedup(struct file *f);




#ifndef NULL
#define NULL ((void *)0)
#endif

#endif // DEF_H
