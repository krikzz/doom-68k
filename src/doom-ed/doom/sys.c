

#include "sys.h"
#include "d_event.h"
#include "doomdef.h"
#include "bios.h"
#include "r_defs.h"
#include "stdlib.h"


void D_PostEvent(event_t* ev);

void sysDispCB();
void sysIdleCB();
u16 sysJoyRead();
void keyUpdate();


//Color gfx_buff[G_SCREEN_W * G_SCREEN_H];
void (*sys_cb)(void);

u16 joy_old;

//****************************************************************************** sega
#define VDP_REG_MOD1    0x8000
#define VDP_REG_MOD2    0x8100
#define VDP_REG_PLANA   0x8200 //plan-a addr. XXX0_0000_0000_0000 
#define VDP_REG_PLANW   0x8300 //windows addr
#define VDP_REG_PLANB   0x8400 //plan-b addr. XXX0_0000_0000_0000 
#define VDP_REG_PLANS   0x8500 //sprite table addr.  H32 mod=XXXX_XXX0_0000_0000, H40 mod=XXXX_XX00_0000_0000
#define VDP_REG_BGCLR   0x8700 //BG color. 00PP_CCCC P=pal num, C=color num
#define VDP_REG_HINT    0x8A00 //set hint line number
#define VDP_REG_MOD3    0x8B00
#define VDP_REG_MOD4    0x8C00
#define VDP_REG_HSCRL   0x8D00 //hscroll table addr. XXXX_XX00_0000_0000
#define VDP_REG_AINC    0x8F00 //auto increment
#define VDP_REG_PSIZ    0x9000 //plan size
#define VDP_REG_WIN_X   0x9100 //cell pos x
#define VDP_REG_WIN_Y   0x9200 //cell pos y
#define VDP_REG_DMALL   0x9300 //dma len lo
#define VDP_REG_DMALH   0x9400 //dma len hi
#define VDP_REG_DMASL   0x9500 //dma src lo. A8-A1
#define VDP_REG_DMASM   0x9600 //dma src mid. A16-A9
#define VDP_REG_DMASH   0x9700 //dma src hi. A22-A17

#define VDP_CPY_DMA_RUN(adr)    ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0xC0)
#define VDP_VRAM_DMA_RUN(adr)   ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x80)
#define VDP_CRAM_DMA_RUN(adr)   ((0xC000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x80)
#define VDP_VSRAM_DMA_RUN(adr)  ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x90)

#define GFX_DATA_PORT           0xC00000
#define GFX_CTRL_PORT           0xC00004
#define VDP_DATA16 *((volatile u16 *)GFX_DATA_PORT)
#define VDP_DATA32 *((volatile u32 *)GFX_DATA_PORT)
#define VDP_CTRL16 *((volatile u16 *)GFX_CTRL_PORT)
#define VDP_CTRL32 *((volatile u32 *)GFX_CTRL_PORT)

#define JOY_DATA_1      *((volatile u8 *) 0xa10003)
#define JOY_CONTROL_1   *((volatile u8 *) 0xa10009)

#define JOY_DATA_2      *((volatile u8 *) 0xa10005)
#define JOY_CONTROL_2   *((volatile u8 *) 0xa1000B)

#define VDP_VBLANK_FLAG (1 << 3)

#define GFX_WRITE_VRAM_ADDR(adr)    ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x00)
#define GFX_READ_VRAM_ADDR(adr)     ((0x0000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x00)
#define GFX_WRITE_CRAM_ADDR(adr)    ((0xC000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x00)
#define GFX_WRITE_VSRAM_ADDR(adr)   ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x10)

#define G_PLAN_W        64
#define G_PLAN_H        28

#define FONT_ADDR       1024
#define TILE_MEM_END    0xB000

#define WPLAN           (TILE_MEM_END + 0x0000)
#define HSCRL           (TILE_MEM_END + 0x0800)
#define SLIST           (TILE_MEM_END + 0x0C00)
#define APLAN           (TILE_MEM_END + 0x1000)
#define BPLAN           (TILE_MEM_END + 0x3000)

#define JOY_A   0x0040
#define JOY_B   0x0010
#define JOY_C   0x0020
#define JOY_STA 0x0080
#define JOY_U   0x0001
#define JOY_D   0x0002
#define JOY_L   0x0004
#define JOY_R   0x0008

#define PLAN_END        ((G_PLAN_W * G_PLAN_H * 2))

#define BITMAP_BASE     (FONT_ADDR + 4096)

void vdpVramWrite(u16 *src, u16 dst, u16 len);
void vdpVramRead(u16 src, u16 *dst, u16 len);
void vdpSetReg(u8 reg, u16 val);
void vdpVramWriteDMA(void *src, u16 dst, u16 len);

void gSetPal(u16 pal);
void gSetPlan(u16 plan);
void gCleanPlan();
void gSetColor(u16 color, u16 val);
void gResetPlan();

extern u16 font_base[];

void sysInit(int argc, char** argv) {

    u16 i;

    sys_cb = 0;
    stdInit();

    vdpSetReg(15, 0x02); //autoinc
    vdpSetReg(16, 0x11); //plan size 64x64

    vdpSetReg(0, 0x04);
    vdpSetReg(1, 0x54);

    JOY_CONTROL_1 = 0x40;
    JOY_DATA_1 = 0x00;
    JOY_CONTROL_2 = 0x40;
    JOY_DATA_2 = 0x00;

    vdpVramWrite(font_base, FONT_ADDR, 4096);

    u16 color = 0;

    for (i = 0; i < 16; i++) {
        gSetColor(i + 0x00, color);
        gSetColor(i + 0x10, color);
        gSetColor(i + 0x20, color);
        gSetColor(i + 0x30, color);
        color += 0x111;
    }

    //if(font_base[0] == 0xaaaa)gSetColor(0,0xf0f);

    gSetPal(0);
    gSetPlan(APLAN);
    gCleanPlan();

    bi_init();



}
//****************************************************************************** 

void vdpSetReg(u8 reg, u16 val) {
    VDP_CTRL16 = 0x8000 | (reg << 8) | val;
}

void vdpVramWrite(u16 *src, u16 dst, u16 len) {

    len >>= 1;
    VDP_CTRL32 = GFX_WRITE_VRAM_ADDR(dst);
    while (len--)VDP_DATA16 = *src++;
}

void vdpVramRead(u16 src, u16 *dst, u16 len) {

    len >>= 1;

    *((volatile u32 *) GFX_CTRL_PORT) = GFX_READ_VRAM_ADDR(src);
    while (len--)*dst++ = VDP_DATA16;
}

void vdpVramWriteDMA(void *src, u16 dst, u16 len) {

    VDP_CTRL16 = VDP_REG_DMALL | ((len >> 1) & 0xff);
    VDP_CTRL16 = VDP_REG_DMALH | ((len >> 9) & 0xff);

    VDP_CTRL16 = VDP_REG_DMASL | (((u32) src >> 1) & 0xff);
    VDP_CTRL16 = VDP_REG_DMASM | (((u32) src >> 9) & 0xff);
    VDP_CTRL16 = VDP_REG_DMASH | (((u32) src >> 17) & 0x7f);

    VDP_CTRL32 = VDP_VRAM_DMA_RUN(dst);
}

//****************************************************************************** gfx base
u16 g_plan;
u16 g_pal;
u32 g_addr;

void gSetColor(u16 color, u16 val) {

    VDP_CTRL32 = GFX_WRITE_CRAM_ADDR(color << 1);
    VDP_DATA16 = val;
}

void gVsync() {

    u16 vdp_state = VDP_VBLANK_FLAG;

    while (vdp_state & VDP_VBLANK_FLAG) {
        vdp_state = VDP_CTRL16;
    }

    while (!(vdp_state & VDP_VBLANK_FLAG)) {
        vdp_state = VDP_CTRL16;
    }
}

void gSetPal(u16 pal) {
    g_pal = pal;
}

void gSetPlan(u16 plan) {
    g_plan = plan;
    VDP_CTRL32 = GFX_WRITE_VRAM_ADDR(g_plan + g_addr);
}

void gResetPlan() {

    g_addr = 0;
    VDP_CTRL32 = GFX_WRITE_VRAM_ADDR(g_plan);
}

void gCleanPlan() {

    u16 len = (G_PLAN_W * G_PLAN_H) / 2;
    g_addr = 0;

    VDP_CTRL32 = GFX_WRITE_VRAM_ADDR(g_plan);
    while (len--) {
        VDP_DATA32 = 0;
    }

    VDP_CTRL32 = GFX_WRITE_VRAM_ADDR(g_plan);
}

void gAppendString(u8 *str) {

    //GFX_CTRL_PORT32 = GFX_WRITE_VRAM_ADDR(APLAN);
    u16 max = 40;

    while (*str != 0 && max--) {

        if (g_addr >= PLAN_END) {
            gCleanPlan();
        }

        VDP_DATA16 = *str++ | g_pal;
        g_addr++;
    }
}

void gAppendChar(char c) {

    if (g_addr >= PLAN_END) {
        gCleanPlan();
    }

    VDP_DATA16 = c;
    g_addr++;
}

void gConsPrint(u8 *str) {

    g_addr = g_addr / (G_PLAN_W * 2) * (G_PLAN_W * 2);
    g_addr += G_PLAN_W * 2;

    if (g_addr >= PLAN_END) {
        gCleanPlan();
    }

    VDP_CTRL32 = GFX_WRITE_VRAM_ADDR(g_plan + g_addr);
    gAppendString(str);
}

void gAppendHex4(u8 val) {

    val += (val < 10 ? '0' : '7');
    VDP_DATA16 = val | g_pal;
}

void gAppendHex8(u8 val) {

    gAppendHex4(val >> 4);
    gAppendHex4(val & 15);
}

void gAppendHex16(u16 val) {

    gAppendHex8(val >> 8);
    gAppendHex8(val);
}

void gAppendHex32(u32 val) {

    gAppendHex16(val >> 16);
    gAppendHex16(val);

}

void gAppendNum(s32 num) {

    u16 i;
    u8 buff[11];
    u8 *str = (u8 *) & buff[10];

    if (num < 0) {
        num = -num;
        gAppendChar('-');
    }

    *str = 0;
    if (num == 0)*--str = '0';
    for (i = 0; num != 0; i++) {

        *--str = num % 10 + '0';
        num /= 10;
    }

    gAppendString(str);

}

u16 sysJoyRead() {

    u8 joy;

    JOY_DATA_1 = 0x40;
    asm("nop");
    asm("nop");
    asm("nop");
    joy = (JOY_DATA_1 & 0x3F);
    JOY_DATA_1 = 0x00;
    asm("nop");
    asm("nop");
    asm("nop");
    joy |= (JOY_DATA_1 & 0x30) << 2;

    joy ^= 0xff;

    return joy & 0xff;
}

//******************************************************************************

void sysStart(void (*callback)(void)) {

    u16 i;
    u16 ctr = BITMAP_BASE / 32; //zero tile reserved for transparent


    //while (1);
    gSetColor(0, 0xf00);

    //clean zero tile
    VDP_CTRL32 = GFX_WRITE_VRAM_ADDR(0);
    for (i = 0; i < 64 / 4; i++) {
        VDP_DATA16 = 0x8888;
    }

    //clean plan
    VDP_CTRL32 = GFX_WRITE_VRAM_ADDR(APLAN);
    for (i = 0; i < 64 * 32; i++) {
        VDP_DATA16 = 0;
    }

    //screen tilemap
    VDP_CTRL32 = GFX_WRITE_VRAM_ADDR(APLAN + (G_PLAN_W * 6 + 4) * 2);
    for (i = 0; i < G_PLAN_W * 16; i++) {

        if (i % 64 < 32) {
            VDP_DATA16 = ctr++;
        } else {
            VDP_DATA16 = 0xffff;
        }
    }

    //setup palette
    u16 color = 0xfff;
    for (i = 0; i < 16; i++) {
        gSetColor(i, color);
        if (i % 3 == 0)color -= 0x002;
        if (i % 3 == 1)color -= 0x020;
        if (i % 3 == 2)color -= 0x200;
    }

    gSetColor(16, 0x000);
    gSetColor(31, 0xfff);

    sys_cb = callback;


    /*
        while (1) {

            extern u8 sshot[320 * 200];
            REG_FBUFF = (u32) sshot;
            u16 *frame_buff = (u16 *) ADDR_FBUFF;
            vdpVramWriteDMA(frame_buff, 32 - 2, 320 * 200 / 2);
        }*/


    while (1) {

        //u16 time = bi_get_ticks();
        keyUpdate();
        u16 game = bi_get_ticks();
        sysIdleCB();
        game = bi_get_ticks() - game;

        u16 vdma = bi_get_ticks();
        sysDispCB();
        vdma = bi_get_ticks() - vdma;

        gResetPlan();
        std_printf("game: %d    \n", game);
        std_printf("vdma: %d    \n", vdma);
        //time = bi_get_ticks() - time;
    }

}

void sysDispCB() {

    extern byte * screens[5];

    REG_FBUFF = (u32) & screens[0][320 * 20 + 32];
    //gVsync();
    vdpVramWriteDMA(
            (u16 *) ADDR_FBUFF, //bitmap transformation address
            BITMAP_BASE - 2, //vdp ram dst
            256 * 128 / 2 + 2//lenght   
            );

    //std_memset(& screens[0][320 * 20 + 32], 0xff, 128 * 320);
    //bi_cmd_mem_set(0xff, (u32)&screens[0][320 * 20 + 32],  128 * 320);

}

void sysIdleCB() {

    if (sys_cb == 0)return;
    sys_cb();

}

//****************************************************************************** controls
#define JOY_U   0x0001
#define JOY_D   0x0002
#define JOY_L   0x0004
#define JOY_R   0x0008
#define JOY_B   0x0010
#define JOY_C   0x0020
#define JOY_A   0x0040
#define JOY_STA 0x0080

void keyUpdate() {

    u16 i;
    static u8 key_tbl[] = {
        //JOY_U, JOY_D, JOY_L, JOY_R, JOY_B, JOY_C, JOY_A, JOY_STA
        KEY_UPARROW, KEY_DOWNARROW, KEY_LEFTARROW, KEY_RIGHTARROW, 'x', KEY_ESCAPE, 'z', KEY_ENTER
    };

    u16 joy = sysJoyRead();
    if (joy == joy_old)return;

    u16 cur = joy;
    u16 old = joy_old;

    joy_old = joy;

    event_t e;

    for (i = 0; i < 8; i++) {

        if ((cur & 1) != (old & 1)) {

            e.type = (cur & 1) ? ev_keydown : ev_keyup;
            e.data1 = key_tbl[i];
            D_PostEvent(&e);
        }

        old >>= 1;
        cur >>= 1;
    }


}



//****************************************************************************** var

int sysGetTime() {

    static int time;
    static u16 ntime;
    static u16 otime;
    u16 delta;

    ntime = bi_get_ticks();
    delta = (ntime - otime);
    otime = ntime;
    time += delta;

    //time += 28 * 4;
    return time / 32; // / 32;
}

