#include "fs.h"
#include "def.h"
#include "proc.h"
#include "const.h"
#include "global.h"

static int
fdalloc()
{
    int fd;
    for (fd = 0; fd < NOFILE; fd++)
        if (fs_current->ofile[fd] == NULL) {
            fs_current->ofile[fd] = (struct file *)1;
            return fd;
        }
    return -1;
}


int
do_seek(int fd, u32 off)
{
    if (fd < 0 || fd >= NOFILE || !fs_current->ofile[fd])
        return -1;
    if (off <= fs_current->ofile[fd]->ip->size) {
        fs_current->ofile[fd]->off = off;
        return 0;
    }
    return -1;
}

int
do_write(int fd, void *dst, u32 cnt)
{
    if (fd < 0 || fd >= NOFILE || !fs_current->ofile[fd])
        return -1;
    return filewrite(fs_current->ofile[fd], dst, cnt);
}

int
do_read(int fd, void *src, u32 cnt)
{
    if (fd < 0 || fd >= NOFILE || !fs_current->ofile[fd])
        return -1;
    return fileread(fs_current->ofile[fd], src, cnt);
}

int
do_stat(int fd, struct stat *sp)
{
    if (fd < 0 || fd >= NOFILE || !fs_current->ofile[fd])
        return -1;
    return filestat(fs_current->ofile[fd], sp);
}

int
do_open(const char *path, int mode)
{
    int fd;
    struct file *f;
    struct inode *ip;

    if ((mode & O_CREATE) &&!(ip =
            icreate(path, T_FILE, 0, 0)))
        return -1;
    else
        if (!(ip = namei(path)))
            return -1;
    if (ip->type == T_DIR && mode != O_RDONLY) {
        iput(ip);
        return -1;
    }

    if (!(f = filealloc()) || (fd = fdalloc()) < 0) {
        if (f)
            fileclose(f);
        iput(ip);
        return -1;
    }


    f->type = FD_INODE;
    f->ip = ip;
    f->off = 0;
    f->readable = !(mode & O_WRONLY);
    f->writable = (mode & O_WRONLY) || (mode & O_RDWR);

    fs_current->ofile[fd] = f;
    return fd;
}

int
do_close(int fd)
{
    if (fd < 0 || fd >= NOFILE || !fs_current->ofile[fd])
        return -1;
    fileclose(fs_current->ofile[fd]);
    fs_current->ofile[fd] = NULL;
    return 0;
}

int
do_mkdir(char *path)
{
    struct inode *ip;
    if (!(ip = icreate(path, T_DIR, 0, 0)))
        return -1;

    iput(ip);
    return 0;
}


int
do_mknod(char *path, short major, short minor)
{
    struct inode *ip;

    if ((ip = icreate(path, T_DEV, major, minor)) == NULL)
        return -1;

    iput(ip);
    return 0;
}

int
do_link(char *new, char *target)
{
    char name[NAMELEN];
    struct inode *dp, *ip;

    if ((ip = namei(target)) == NULL)
        return -1;

    if (ip->type == T_DIR) {
        iput(ip);
        return -1;
    }

    ip->nlink++;
    iupdate(ip);

    if ((dp = nameiparent(new, name)) == 0)
        goto bad;
    if (dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0) {
        iput(dp);
        goto bad;
    }
    iput(dp);
    iput(ip);

    return 0;

bad:
    ip->nlink--;
    iupdate(ip);
    iput(ip);
    return -1;
}

// Is the directory dp empty except for "." and ".." ?
static int
isdirempty(struct inode *dp)
{
    int off;
    struct dirent de;

    for (off = 2 * sizeof(de); off < dp->size; off += sizeof(de)) {
        if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
            panic("isdirempty: readi");
        if (de.inum != 0)
            return 0;
    }
    return 1;
}

int
do_unlink(char *path)
{
    struct inode *ip, *dp;
    struct dirent de;
    char name[NAMELEN];
    u32 off;
    if ((dp = nameiparent(path, name)) == NULL)
        return -1;
    // Cannot unlink "." or "..".
    if (namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
        goto bad;

    if ((ip = dirlookup(dp, name, &off)) == NULL)
        goto bad;

    if (ip->nlink < 1)
            panic("unlink: nlink < 1");
    if (ip->type == T_DIR && !isdirempty(ip)) {
        iput(ip);
        goto bad;
    }

    memset(&de, 0, sizeof(de));

    if (writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
        panic("unlink: writei");
    if (ip->type == T_DIR) {
        dp->nlink--;  // for ..
        iupdate(dp);
    }
    iput(dp);

    ip->nlink--;
    iupdate(ip);
    iput(ip);

    return 0;

bad:
    iput(dp);
    return -1;
}



int
do_dup(int fd)
{
    int nfd;

    if ((nfd = fdalloc()) < 0)
        return -1;
    filedup(fs_current->ofile[fd]);
    fs_current->ofile[nfd] = fs_current->ofile[fd];
    return nfd;
}

int
do_chdir(char *path)
{
    struct inode *ip;
    if (!(ip = namei(path)))
        return -1;
    if (ip->type != T_DIR) {
        iput(ip);
        return -1;
    }

    if (fs_current->cwd)
        iput(fs_current->cwd);
    fs_current->cwd = ip;
    return 0;
}

int
do_pipe(int *fd)
{
    struct file *rf, *wf;
    int fd0, fd1;

    if (pipealloc(&rf, &wf) < 0)
        return -1;

    if ((fd0 = fdalloc()) < 0 || (fd1 = fdalloc()) < 0) {
        fileclose(rf);
        fileclose(wf);
        if (fd0 > 0)
            current->ofile[fd0] = NULL;
        return -1;
    }

    fs_current->ofile[fd0] = rf;
    fs_current->ofile[fd1] = wf;
    fd[0] = fd0;
    fd[1] = fd1;
    return 0;
}


int
do_gotoxy(u32 x, u32 y)
{
    return tty_gotoxy(x, y);
}

int
do_ttymode(u32 mode)
{
    tty_setmode(mode);
    return 0;
}

int
do_ttycolor(int back, int front)
{
    tty_setcolor(back, front);
    return 0;
}


int
do_cls()
{
    tty_cls();
    return 0;
}

int
do_fsfork(struct proc *op, struct proc *np)
{
    int i;
    for(i = 0; i < NOFILE; i++)
        np->ofile[i] = op->ofile[i] ?
                filedup(op->ofile[i]) : NULL;
    np->cwd = op->cwd ? idup(op->cwd) : NULL;
    return 0;
}

int
do_fsexit(struct proc *p)
{
    int i;

    for(i = 0; i < NOFILE; i++)
        if(p->ofile[i]) {
            fileclose(p->ofile[i]);
            p->ofile[i] = NULL;
        }
    if (p->cwd) {
        iput(p->cwd);
        p->cwd = NULL;
    }
    return 0;
}
