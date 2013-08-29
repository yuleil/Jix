#include "stdio.h"
#include "usr.h"
#include "string.h"

#define WHITESPACE " \t"

char **
getcmd()
{
    static char buf[200];
    static char *cmd[20];
    int i, c;

    if (fgets(buf, 200, stdin) == NULL ||
            (buf[0] == '\n' && buf[1] == '\0'))
        return NULL;

    for (i = c = 0; buf[i]; i++) {
        if (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n') {
            buf[i] = '\0';
            continue;
        }
        if (i == 0 || buf[i - 1] == '\0')
            cmd[c++] = buf + i;
    }
    cmd[c] = NULL;

    return cmd;
}

char ***
parsepipe(char **cmd, int *n)
{
    int i, c;
    static char **pipecmd[10];

    pipecmd[0] = cmd;

    for (c = i = 1; cmd[i]; i++)
        if (cmd[i][0] == '|' && cmd[i][1] == '\0') {
            cmd[i] = NULL;
            pipecmd[c++] = cmd + i + 1;
        }

    if (c == 1)
        return NULL;

    pipecmd[c] = NULL;
    *n = c;

    return pipecmd;

}

int
main(void)
{
    int pid, state, npipe, pipefd[2], fd;
    char **cmd;
    char ***pipecmd;

    while (1) {
        printf("sh:$ ");

        if ((cmd = getcmd()) == NULL) {
            clearerr(stdin);
            continue;
        }


        if (strcmp("cd", cmd[0]) == 0) {
            if (chdir(cmd[1]) < 0)
                fprintf(stderr, "cannot cd %s\n", cmd[1]);
            continue;
        }


        pipecmd = parsepipe(cmd, &npipe);
        if (pipecmd) {
            if (npipe != 2) {
                fprintf(stderr, "only support one pipeline\n");
                continue;
            }

            if ((fd = open(pipecmd[0][0], O_RDONLY)) == -1) {
                fprintf(stderr, "can not open %s\n", pipecmd[0][0]);
                continue;
            }
            close(fd);
            if ((fd = open(pipecmd[1][0], O_RDONLY)) == -1) {
                fprintf(stderr, "can not open %s\n", pipecmd[1][0]);
                continue;
            }
            close(fd);

            pipe(pipefd);

            if (fork() == 0) {
                close(1);
                dup(pipefd[1]);
                close(pipefd[0]);
                close(pipefd[1]);

                exec(pipecmd[0][0], pipecmd[0]);
                fprintf(stderr, "can't execute %s\n", pipecmd[0][0]);
                exit(-1);
            }
            if (fork() == 0) {
                close(0);
                dup(pipefd[0]);
                close(pipefd[0]);
                close(pipefd[1]);

                exec(pipecmd[1][0], pipecmd[1]);
                fprintf(stderr, "can't execute %s\n", pipecmd[1][0]);
                exit(-1);
            }

            close(pipefd[0]);
            close(pipefd[1]);


            pid = wait(&state, 0);
            printf("\nprocess(pid: %d) exited with state %d.\n", pid, state);
            pid = wait(&state, 0);
            printf("\nprocess(pid: %d) exited with state %d.\n", pid, state);

        } else {
            if ((fd = open(cmd[0], O_RDONLY)) == -1) {
                fprintf(stderr, "can not open %s\n", cmd[0]);
                continue;
            }
            close(fd);

            if((pid = fork()) < 0)
                fprintf(stderr, "fork error\n");
            else if(pid == 0) {
                exec(cmd[0], cmd);
                fprintf(stderr, "can't execute %s\n", cmd[0]);
                exit(-1);
            } else {
                pid = wait(&state, 0);
                printf("\nprocess(pid: %d) exited with state %d.\n", pid, state);
            }
        }
    }

    return 0;
}
