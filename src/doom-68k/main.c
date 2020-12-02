/* 
 * File:   main.c
 * Author: igor
 *
 * Created on November 22, 2020, 2:31 PM
 */

#include "sys.h"

void D_DoomInit(void);
void D_DoomLoop(void);
void sysStart(void (*callback)(void));
void sysInit(int argc, char** argv);

void gAppendString(u8 *str);
void gVsync();
void gConsPrint(u8 *str);
void gAppendHex4(u8 val);
void gAppendHex8(u8 val);
void gAppendHex16(u16 val);
void gAppendHex32(u32 val);
void gPrintHex(void *src, u16 len);
void gAppendNum(u32 num);

int main(int argc, char** argv) {

    extern int myargc;
    extern char** myargv;

    myargc = 0;
    myargv = argv;

    sysInit(argc, argv);

    //gAppendString("Doom init...");
    std_printf("Doom init...\n");
    /*
    sys_printf("multi print: %d, %s, %c, %x\n", 8086, "hello", 'X', 0xED64);
    sys_printf("multi print: %d, %s, %c, %x\n", 8086, "hello", 'X', 0xED64);
    sys_printf("multi print: %d, %s, %c, %x\n", 8086, "hello", 'X', 0xED64);
    sys_printf("multi print: %d, %s, %c, %x\n", 8086, "hello", 'X', 0xED64);
    sys_printf("multi print: %d, %s, %c, %x\n", 8086, "hello", 'X', 0xED64);
    sys_printf("multi print: %d, %s, %c, %x\n", 8086, "hello", 'X', 0xED64);*/

    *(u16 *) 0xA130F0 = 0xA000;
    D_DoomInit();
    sysStart(D_DoomLoop);

    while (1);

    return 0;
}


