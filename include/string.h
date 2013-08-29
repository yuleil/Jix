#ifndef STRING_H
#define STRING_H


void *memset(void *dst, int c, u32 n);
int memcmp(const void *v1, const void *v2, u32 n);
void *memmove(void *dst, const void *src, u32 n);
void *memcpy(void *dst, const void *src, u32 n);
int strcmp(const char *p, const char *q);
int strncmp(const char *p, const char *q, u32 n);
char *strncpy(char *s, const char *t, int n);
char *strcpy(char *s, const char *t);
char *safestrcpy(char *s, const char *t, int n);
int strlen(const char *s);



#endif // STRING_H
