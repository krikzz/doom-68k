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


void gResetPlan();

/*
int FixedMul2(int a, int b) {


    REG_MUL_A = a;
    REG_MUL_B = b;
    return REG_MUL_R;
    //return ((long long) a * (long long) b) >> FRACBITS;
}*/

#define MININT2		((int)0x80000000)	
#define MINLONG2	((long)0x80000000)
#define MAXINT2		((int)0x7fffffff)

/*
s32 FixedDiv2(s32 a, s32 b) {

    if ((std_abs(a) >> 14) >= std_abs(b)) {
        return (a^b) < 0 ? MININT2 : MAXINT2;
    }

    REG_DIV_A = a;
    REG_DIV_B = b;
    return REG_DIV_R;
    //return ((long long) a << 16) / b;
    //return FixedDiv2(a, b);
}*/

int fraktal1(int fx, int fy) {

    return ((fy >> (16 - 6))&(63 * 64)) + ((fx >> 16)&63);
}

int fraktal2(int fx, int fy) {

    REG_XFRAC = fx;
    REG_YFRAC = fy;

    return REG_SPOT;
}

int main(int argc, char** argv) {

    extern int myargc;
    extern char** myargv;

    myargc = 0;
    myargv = argv;

    sysInit(argc, argv);

    //gAppendString("Doom init...");
    std_printf("Doom init...\n");


    /*
    REG_MUL_A = 0x12345;
    REG_MUL_B = 0xabcde;


    std_printf("mula: %x\n", REG_MUL_S);
    
    std_printf("mula: %x\n", 0x12345 * 0xabcde);
    while(1);*/

    //REG_MUL_A = 0x1234ABCD;
    //REG_MUL_B = 0xABCD1234;

    //u32 rz
    /*
        u16 i;
        s32 val1 = 0x0fffffff;
        s32 val2 = 1;

        for (i = 0; i < 16; i++) {


            s32 f1 = fraktal1(val1, val2);
            s32 f2 = fraktal2(val1, val2);

            std_printf("mul: %x, %x, %x  \n", f1, f2, val2);

            //val1 -= 0x11111111;
            val2 += 0x11111111;
        }

        //while (1);*/
    /*
    REG_FBUFF = 0x200000;
    u8 *p = (u8 *) 0x200000;
    u16 i;
    u32 *md = (u32 *) ADDR_FBUFF;

    for (i = 0; i < 16; i++) {
        p[i] = i | 0xA0;
    }

    while (1) {
        gCleanPlan2();
        std_printf("vals: %x\n", md[0]);
        std_printf("vals: %x\n", md[1]);
    }

    while (1);*/

    /*
    sys_printf("multi print: %d, %s, %c, %x\n", 8086, "hello", 'X', 0xED64);
    sys_printf("multi print: %d, %s, %c, %x\n", 8086, "hello", 'X', 0xED64);
    sys_printf("multi print: %d, %s, %c, %x\n", 8086, "hello", 'X', 0xED64);
    sys_printf("multi print: %d, %s, %c, %x\n", 8086, "hello", 'X', 0xED64);
    sys_printf("multi print: %d, %s, %c, %x\n", 8086, "hello", 'X', 0xED64);
    sys_printf("multi print: %d, %s, %c, %x\n", 8086, "hello", 'X', 0xED64);*/

    *(u16 *) 0xA13084 = 0xffff;
    D_DoomInit();
    sysStart(D_DoomLoop);

    while (1);

    return 0;
}


