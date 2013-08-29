#include "usr.h"
#include "stdio.h"
#include "string.h"
#include "tty.h"



char *
kmp_pp(const char *pattern)
{
    static char jmpt[100];
    int i, last /* matched from pattern's head */;

    last = jmpt[0] = 0;

    for (i = 1; pattern[i]; i++) {
        if (pattern[i] == pattern[last])
           jmpt[i] = ++last;
        else last = 0;
    }

    jmpt[i] = -1; // sentry
    return jmpt;
}

const char *
kmp_match(const char *s, const char *pattern, const char *jmpt)
{
    int i, pos;

    for (i = pos = 0; s[i];) {
        if (pattern[pos] == s[i]) {
            pos++;
            i++;
        } else {
            if (pos)    pos = jmpt[pos-1];
            else        i++;
            continue;
        }

        if (jmpt[pos] == -1) // now pos == len pattern
            return s + i - pos;
    }

    return NULL;
}


void
grep_output(const char *s, const char *find, int patternlen)
{
    char outbuff[200];

    memcpy(outbuff, s, find - s);
    outbuff[find - s] = '\0';
    printf(outbuff);

    memcpy(outbuff, find, patternlen);
    outbuff[patternlen] = '\0';
    tty_color(BLACK, RED);
    printf(outbuff);

    tty_color(BLACK, WHITE);
    printf(find + patternlen);
}

int
main(int argc, char **argv)
{
    char *jmpt, buff[200];
    const char *find, *pattern;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s PATTERN.\n", *argv);
        return -1;
    }

    pattern = argv[1];
    jmpt = kmp_pp(pattern);

    while (fgets(buff, sizeof(buff), stdin) != NULL) {
        if ((find = kmp_match(buff, pattern, jmpt)))
            grep_output(buff, find, strlen(pattern));
    }
    return 0;
}
