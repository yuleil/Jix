#include "stdio.h"
#include "types.h"
#include "fs.h"
#include "usr.h"

char *type[] = {
    [T_FILE] = "FILE",
            [T_DEV] = "DEV",
            [T_DIR] = "DIR"
};

char*
fname(char *path)
{
    char *p;

    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    return ++p;
}

void
ls(char *path)
{
    char buf[512], *p;
    int fd, ffd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0) {
        fprintf(stderr,  "ls: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(stderr, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type) {
        case T_FILE:
            printf("%-20s %4s   %d   %5d bytes\n", fname(path), type[st.type], st.ino, st.size);
            break;

        case T_DIR:
            if (strlen(path) + 1 + NAMELEN + 1 > sizeof buf) {
                printf("ls: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';

            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0)
                    continue;
                strncpy(p, de.name, NAMELEN);
                p[NAMELEN] = 0;
                if ((ffd = open(buf, O_RDONLY)) < 0 ||
                        fstat(ffd, &st) < 0) {
                    close(ffd);
                    printf("ls: cannot stat %s\n", buf);
                    continue;
                }
                close(ffd);
                printf("%-20s %4s   %d   %5d bytes\n", fname(buf), type[st.type], st.ino, st.size);
            }
            break;
    }
    close(fd);
}

int
main(int argc, char *argv[])
{
    int i;

    if(argc < 2){
        ls(".");
        exit(0);
    }
    for(i=1; i<argc; i++)
        ls(argv[i]);
    exit(0);
    return 0;
}

