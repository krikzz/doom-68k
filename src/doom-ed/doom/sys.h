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

#define vs32    volatile long

#define Color   u32

#define G_SCREEN_W      320
#define G_SCREEN_H      200

#define ADDR_FBUFF      0x070000
#define ADDR_SYS_RAM    0x100000
#define SIZE_SYS_RAM    (0x800000 - ADDR_SYS_RAM)
#define SIZE_DOOM_RAM   0x600000
//#define SIZE_SYS_RAM    (0x400000 - ADDR_SYS_RAM)
//#define SIZE_DOOM_RAM   0x280000

#define REG_FBUFF       *(vu32 *)0xA13080
#define REG_SWAP        *(vu32 *)0xA13084       
#define REG_SWAP16W     *(vu16 *)0xA13084
#define REG_SWAP16R     *(vu16 *)0xA13086


#define REG_MUL_A       *(vs32 *)0xA13090
#define REG_MUL_B       *(vs32 *)0xA13094
#define REG_MUL_F       *(vs32 *)0xA13090//fixed mul result
#define REG_MUL_S       *(vs32 *)0xA13092//standard mul result

#define REG_DIV_A       *(vs32 *)0xA13098
#define REG_DIV_B       *(vs32 *)0xA1309C
#define REG_DIV_R       *(vs32 *)0xA13098

//row/columns render counters
#define REG_XSTEP       *(vs32 *)0xA130A0
#define REG_YSTEP       *(vs32 *)0xA130A4
#define REG_XFRAC       *(vs32 *)0xA130A8
#define REG_YFRAC       *(vs32 *)0xA130AC

#define REG_SPOT        *(vu8 *) 0xA130A0//row val
#define REG_COL_D       *(vu8 *) 0xA130B1//column dst
#define REG_COL_S       *(vu8 *) 0xA130B0//column src

#define REG_SHIFT_X     *(vu16 *)0xA130B6
#define REG_SHIFT_V     *(vs32 *)0xA130B8
#define REG_SHIFT_L     *(vs32 *)0xA130B8
#define REG_SHIFT_R     *(vs32 *)0xA130BC



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

#include "stdlib.h"

void sysInit(int argc, char** argv);
void sysStart(void (*callback)(void));
int sysGetTime();


void gVsync();
void gConsPrint(u8 *str);
void gAppendHex4(u8 val);
void gAppendHex8(u8 val);
void gAppendHex16(u16 val);
void gAppendHex32(u32 val);
void gPrintHex(void *src, u16 len);
void gAppendNum(s32 num);
void gAppendString(u8 *str);
void gAppendChar(char c);

#endif	/* SYS_H */

