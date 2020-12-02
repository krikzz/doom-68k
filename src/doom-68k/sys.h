/* 
 * File:   sys.h
 * Author: igor
 *
 * Created on November 22, 2020, 5:39 PM
 */

#ifndef SYS_H
#define	SYS_H


#define s8      char
#define s16     short
#define s32     long

#define u8      unsigned char
#define u16     unsigned short
#define u32     unsigned long
#define u64     unsigned long long

#define vu8     volatile unsigned char
#define vu16    volatile unsigned short
#define vu32    volatile unsigned long
#define vu64    volatile unsigned long long

#define Color   u32


#define G_SCREEN_W      320
#define G_SCREEN_H      200

#define ADDR_SYS_RAM    0x100000
#define SIZE_SYS_RAM    (0x400000 - ADDR_SYS_RAM)
#define SIZE_DOOM_RAM   0x280000


#define NO_SHADING

#define __BIG_ENDIAN__

#ifndef _FILE_DEFINED

struct _iobuf {
    char *_ptr;
    int _cnt;
    char *_base;
    int _flag;
    int _file;
    int _charbuf;
    int _bufsiz;
    char *_tmpfname;
};
typedef struct _iobuf FILE;

#define _FILE_DEFINED
#endif



#ifndef stderr
#define stderr 0
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef size_t
#define size_t u32
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef O_RDONLY
#define O_RDONLY 0x0000
#define O_BINARY 0x8000
#define O_WRONLY 0x0001
#define O_TRUNC  0x0200
#define O_CREAT  0x0100
#endif



void sysInit(int argc, char** argv);
void sysStart(void (*callback)(void));


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
void* std_memset(void *dst, int val, size_t size);
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
int sys_get_time();

int abs(int val);
int min(int val1, int val2);
int std_strncmp(const char *str1, const char *str2, size_t max);
void sys_vsync();
int sys_toupper(int c);
int std_atoi(const char *str);
//void* memcpy(void * dst, const void *src, size_t size);

#endif	/* SYS_H */

