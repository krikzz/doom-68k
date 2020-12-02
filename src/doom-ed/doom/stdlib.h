/* 
 * File:   stdlib.h
 * Author: igor
 *
 * Created on November 30, 2020, 3:47 PM
 */

#ifndef STDLIB_H
#define	STDLIB_H

void stdInit();

int std_open(const char *fname, int flags, int mode);
int std_read(int handler, void *dst, unsigned int size);
int std_close(int handler);
long std_lseek(int handler, long offset, int origin);
int std_access(const char *fname, int access_mode);
long std_filelength(int handler);
int std_write(int handler, const void *src, unsigned int size);
int std_mkdir(const char * dirname, int flags);
int std_fsize(int handler);
void* std_malloc(size_t size);
void* std_alloca(size_t size);
void* std_realloc(void *memory, size_t new_size);
void* std_memset(void *dst, u8 val, size_t size);
void * std_memcpy(void * dst, const void *src, size_t size);
int std_strcmp(const char *str1, const char *str2);
char* std_strcpy(char * dst, const char * src);
char* std_strncpy(char *dst, const char * src, size_t count);
int std_strcmpi(const char *str1, const char *str2);
size_t std_strlen(const char *str);
int std_strncasecmp(const char *str1, const char *str2, size_t max);
char* std_strupr(char *str);
int std_printf(const char *str, ...);
int std_sprintf(char *s, const char *format, ...);
int std_fprintf(FILE *file, const char *format, ...);
char* std_strcat(char* dst, const char *src);
void std_exit(int code);

int std_abs(int val);
int std_min(int val1, int val2);
int std_strncmp(const char *str1, const char *str2, size_t max);
void std_vsync();
int std_toupper(int c);
int std_atoi(const char *str);

#endif	/* STDLIB_H */

