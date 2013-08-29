#include "types.h"
#include "x86.h"

void*
memset(void *dst, int c, u32 n)
{
    if ((int)dst%4 == 0 && n%4 == 0){
        c &= 0xFF;
        stosl(dst, (c<<24)|(c<<16)|(c<<8)|c, n/4);
    } else
        stosb(dst, c, n);
    return dst;
}

int
memcmp(const void *v1, const void *v2, u32 n)
{
    const u8 *s1, *s2;

    s1 = v1;
    s2 = v2;
    while(n-- > 0){
        if(*s1 != *s2)
            return *s1 - *s2;
        s1++, s2++;
    }

    return 0;
}

void*
memmove(void *dst, const void *src, u32 n)
{
    const char *s;
    char *d;

    s = src;
    d = dst;
    if(s < d && s + n > d){
        s += n;
        d += n;
        while(n-- > 0)
            *--d = *--s;
    } else
        while(n-- > 0)
            *d++ = *s++;

    return dst;
}

// memcpy exists to placate GCC.  Use memmove.
void*
memcpy(void *dst, const void *src, u32 n)
{
    return memmove(dst, src, n);
}

int
strncmp(const char *p, const char *q, u32 n)
{
    while(n > 0 && *p && *p == *q)
        n--, p++, q++;
    if(n == 0)
        return 0;
    return (u8)*p - (u8)*q;
}

int
strcmp(const char *p, const char *q)
{
    while(*p && *p == *q)
        p++, q++;

    return (u8)*p - (u8)*q;
}

char*
strncpy(char *s, const char *t, int n)
{
    char *os;

    os = s;
    while (n-- > 0 && (*s++ = *t++) != 0)
        ;

    return os;
}

char*
strcpy(char *s, const char *t)
{
    char *os;

    os = s;
    while ((*s++ = *t++) != 0)
        ;
    return os;
}

// Like strncpy but guaranteed to NUL-terminate.
char*
safestrcpy(char *s, const char *t, int n)
{
    char *os;

    os = s;
    if (n <= 0)
        return os;
    while (--n > 0 && (*s++ = *t++) != 0)
        ;
    *s = 0;
    return os;
}

int
strlen(const char *s)
{
    int n;

    for(n = 0; s[n]; n++)
        ;
    return n;
}

