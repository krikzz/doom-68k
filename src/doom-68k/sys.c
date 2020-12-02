

#include "sys.h"
#include "d_event.h"
#include "doomdef.h"
#include "bios.h"
#include "r_defs.h"


void D_PostEvent(event_t* ev);

void sysDispCB();
void sysIdleCB();
u16 sysJoyRead();
void keyUpdate();
void gVsync();
void gConsPrint(u8 *str);
void gAppendHex4(u8 val);
void gAppendHex8(u8 val);
void gAppendHex16(u16 val);
void gAppendHex32(u32 val);
void gPrintHex(void *src, u16 len);
void gAppendNum(s32 num);

//Color gfx_buff[G_SCREEN_W * G_SCREEN_H];
void (*sys_cb)(void);

u8 *sys_ram;
u32 mall_ptr;
u16 joy_old;

//****************************************************************************** sega
#define GFX_DATA_PORT           0xC00000
#define GFX_CTRL_PORT           0xC00004
#define GFX_DATA_PORT16 *((volatile u16 *)GFX_DATA_PORT)
#define GFX_DATA_PORT32 *((volatile u32 *)GFX_DATA_PORT)
#define GFX_CTRL_PORT16 *((volatile u16 *)GFX_CTRL_PORT)
#define GFX_CTRL_PORT32 *((volatile u32 *)GFX_CTRL_PORT)

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

void vdpVramWrite(u16 *src, u16 dst, u16 len);
void vdpVramRead(u16 src, u16 *dst, u16 len);
void vdpSetReg(u8 reg, u16 val);

void gSetPal(u16 pal);
void gSetPlan(u16 plan);
void gCleanPlan();
void gAppendString(u8 *str);
void gSetColor(u16 color, u16 val);

extern u16 font_base[];

void sysInit(int argc, char** argv) {

    u16 i;

    sys_cb = 0;
    mall_ptr = 0;
    sys_ram = (u8 *) ADDR_SYS_RAM;

    vdpSetReg(15, 0x02); //autoinc
    vdpSetReg(16, 0x11); //plan size 64x64

    vdpSetReg(0, 0x04);
    vdpSetReg(1, 0x44);

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
    GFX_CTRL_PORT16 = 0x8000 | (reg << 8) | val;
}

void vdpVramWrite(u16 *src, u16 dst, u16 len) {

    len >>= 1;
    GFX_CTRL_PORT32 = GFX_WRITE_VRAM_ADDR(dst);
    while (len--)GFX_DATA_PORT16 = *src++;
}

void vdpVramRead(u16 src, u16 *dst, u16 len) {

    len >>= 1;

    *((volatile u32 *) GFX_CTRL_PORT) = GFX_READ_VRAM_ADDR(src);
    while (len--)*dst++ = GFX_DATA_PORT16;
}

//****************************************************************************** gfx base
u16 g_plan;
u16 g_pal;
u32 g_addr;

void gSetColor(u16 color, u16 val) {

    GFX_CTRL_PORT32 = GFX_WRITE_CRAM_ADDR(color << 1);
    GFX_DATA_PORT16 = val;
}

void gVsync() {

    u16 vdp_state = VDP_VBLANK_FLAG;

    while (vdp_state & VDP_VBLANK_FLAG) {
        vdp_state = GFX_CTRL_PORT16;
    }

    while (!(vdp_state & VDP_VBLANK_FLAG)) {
        vdp_state = GFX_CTRL_PORT16;
    }
}

void gSetPal(u16 pal) {
    g_pal = pal;
}

void gSetPlan(u16 plan) {
    g_plan = plan;
    GFX_CTRL_PORT32 = GFX_WRITE_VRAM_ADDR(g_plan + g_addr);
}

void gCleanPlan() {

    u16 len = (G_PLAN_W * G_PLAN_H) / 2;
    g_addr = 0;

    //u16 i;
    //for(i = 0;i < 200;i++)gVsync();

    GFX_CTRL_PORT32 = GFX_WRITE_VRAM_ADDR(g_plan);
    while (len--) {
        GFX_DATA_PORT32 = 0;
    }

    GFX_CTRL_PORT32 = GFX_WRITE_VRAM_ADDR(g_plan);

}

void gAppendString(u8 *str) {

    //GFX_CTRL_PORT32 = GFX_WRITE_VRAM_ADDR(APLAN);
    u16 max = 40;

    while (*str != 0 && max--) {

        if (g_addr >= PLAN_END) {
            gCleanPlan();
        }

        GFX_DATA_PORT16 = *str++ | g_pal;
        g_addr++;
    }
}

void gAppendChar(char c) {

    if (g_addr >= PLAN_END) {
        gCleanPlan();
    }

    GFX_DATA_PORT16 = c;
    g_addr++;
}

void gConsPrint(u8 *str) {

    g_addr = g_addr / (G_PLAN_W * 2) * (G_PLAN_W * 2);
    g_addr += G_PLAN_W * 2;

    if (g_addr >= PLAN_END) {
        gCleanPlan();
    }

    GFX_CTRL_PORT32 = GFX_WRITE_VRAM_ADDR(g_plan + g_addr);
    gAppendString(str);
}

void gAppendHex4(u8 val) {

    val += (val < 10 ? '0' : '7');
    GFX_DATA_PORT16 = val | g_pal;
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

u8 *str_append(u8 *dst, u8 *src) {

    while (*dst != 0)dst++;
    while (*src != 0)*dst++ = *src++;
    *dst = 0;
    return dst;
}

u8 *str_append_num(u8 *dst, s32 num) {

    u16 i;
    u8 buff[11];
    u8 *str = (u8 *) & buff[10];

    if (num < 0) {
        num = -num;
        str_append(dst, "-");
    }

    *str = 0;
    if (num == 0)*--str = '0';
    for (i = 0; num != 0; i++) {
        *--str = num % 10 + '0';
        num /= 10;
    }

    return str_append(dst, str);
}

u8 *str_append_num_z(u8 *dst, s32 num, u8 zeros) {

    u16 str_len = 0;
    u8 buff[16];

    if (num < 0) {
        dst = str_append(dst, "-");
        num = -num;
    }
    buff[0] = 0;
    str_append_num(buff, num);


    while (buff[str_len] != 0)str_len++;


    while (str_len++ < zeros) {
        dst = str_append(dst, "0");
    }

    return str_append(dst, buff);
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
u16 *pixel_pos_lookup; //[G_SCREEN_W * G_SCREEN_H / 8];
u8 *pixel_val_lookup;

void sysStart(void (*callback)(void)) {

    u16 i;
    u16 ctr = 1; //zero tile reserved for transparent

    //while (1);
    gSetColor(0, 0xf00);
    pixel_pos_lookup = std_malloc(G_SCREEN_W * G_SCREEN_H / 8 * sizeof (pixel_pos_lookup[0]));
    pixel_val_lookup = std_malloc(0x10000 * sizeof (pixel_val_lookup[0]));

    for (i = 0;; i++) {
        pixel_val_lookup[i] = ((i & 0x0f00) >> 4) | (i & 0x000f);
        if (i == 65535)break;
    }

    for (i = 0; i < G_SCREEN_W * G_SCREEN_H; i += 8) {

        u16 ty = i % 64 / 8;
        u16 col = i / 64 % (G_SCREEN_W / 8);
        u16 row = i / (8 * G_SCREEN_W);
        pixel_pos_lookup[i / 8] = (ty * G_SCREEN_W) + (col * 8) + row * (G_SCREEN_W * 8);
    }
    gSetColor(0, 0x0f0);

    //clean zero tile
    GFX_CTRL_PORT32 = GFX_WRITE_VRAM_ADDR(0);
    for (i = 0; i < 64 / 4; i++) {
        GFX_DATA_PORT16 = 0xffff;
    }

    //clean plan
    GFX_CTRL_PORT32 = GFX_WRITE_VRAM_ADDR(APLAN);
    for (i = 0; i < 64 * 32; i++) {
        GFX_DATA_PORT16 = 0;
    }

    //screen tilemap
    GFX_CTRL_PORT32 = GFX_WRITE_VRAM_ADDR(APLAN + G_PLAN_W * 2);
    for (i = 0; i < G_PLAN_W * (G_SCREEN_H / 8); i++) {

        if (i % 64 < 40) {
            GFX_DATA_PORT16 = ctr++;
        } else {
            GFX_DATA_PORT16 = 0xffff;
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

    sys_cb = callback;

    while (1) {

        //u16 time = bi_get_ticks();
        keyUpdate();
        sysIdleCB();
        sysDispCB();
        //time = bi_get_ticks() - time;
    }

}

void sysDispCB() {

    extern byte * screens[5];
    u16 i;
    //u8 *p_ptr;
    u8 *frame_buff = (u8 *) screens[0];
    u16 *pp_lup = pixel_pos_lookup;


    GFX_CTRL_PORT32 = GFX_WRITE_VRAM_ADDR(32);

    //i = (G_SCREEN_W * G_SCREEN_H) / 8;
    for (i = 0; i < (G_SCREEN_W * G_SCREEN_H) / 8; i++) {


        u32 p_val;
        u8 *p_val_ptr = (u8 *) & p_val;
        u16 *p_ptr = (u16 *) & frame_buff[*pp_lup++];

        *p_val_ptr++ = pixel_val_lookup[*p_ptr++];
        *p_val_ptr++ = pixel_val_lookup[*p_ptr++];
        *p_val_ptr++ = pixel_val_lookup[*p_ptr++];
        *p_val_ptr++ = pixel_val_lookup[*p_ptr++];

        GFX_DATA_PORT32 = p_val;
    }

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

//****************************************************************************** file io
u32 file_size;
u32 file_offset;

int std_open_rel(const char *fname) {

    u8 resp;
    u8 buff[512];
    u8 *path = buff;
    u16 i, s;

    resp = bi_cmd_file_open("MEGA/sys/registery.dat", FA_READ);
    if (resp)return -1;

    resp = bi_cmd_file_read(buff, sizeof (buff));
    if (resp)return -1;

    bi_cmd_file_close();

    for (i = 0, s = 0; buff[i] != 0; i++) {
        if (buff[i] == '/')s = i;
    }

    buff[s] = 0;

    while (*path != 0) {
        path++;
    }

    *path++ = '/';

    while (*fname != 0) {
        *path++ = *fname++;
    }
    *path++ = 0;

    resp = bi_cmd_file_open(buff, FA_READ);
    if (resp)return -1;

    //std_printf("....path: %s \n", buff);

    return 1;
}

int std_open(const char *fname, int flags, int mode) {

    int resp;
    fname++;

    resp = std_open_rel(fname);

    if (resp < 0) {
        resp = bi_cmd_file_open((u8*) fname, FA_READ);
        if (resp)return -1;
    }

    file_size = bi_cmd_file_available();
    file_offset = 0;
    //while(1);
    //return open(fname, flags, mode);
    return 1;
}

int std_read(int handler, void *dst, unsigned int size) {

    u8 resp;

    size = min(size, file_size - file_offset);

    //resp = bi_cmd_file_read(dst, size);
    resp = bi_cmd_file_read_mem((u32) dst, size);
    if (resp)return -1;

    //return read(handler, dst, size);
    return size;
}

int std_close(int handler) {

    u8 resp;

    resp = bi_cmd_file_close();
    if (resp)return -1;

    //return close(handler);
    return 0;
}

long std_lseek(int handler, long offset, int origin) {

    u8 resp;

    if (origin != 0) {
        std_printf("seek: %d, %d\n", offset, origin);
    }

    resp = bi_cmd_file_set_ptr(offset);
    if (resp)return -1;
    file_offset = offset;

    //return lseek(handler, offset, origin);
    return 0;
}

int std_access(const char *fname, int access_mode) {

    int resp;
    fname++;

    //sys_printf(".access %s\n", fname);

    resp = std_open(fname, 0, 0);
    if (resp < 0)return resp;
    //resp = bi_cmd_file_open((u8*) fname, FA_READ);
    //if (resp)return -1;

    bi_cmd_file_close();
    //while(1);
    //return access(fname, access_mode);
    return 0;
}

long std_filelength(int handler) {

    //sys_printf("......flen\n");
    //return filelength(handler);
    return file_size;
}

int std_write(int handler, const void *src, unsigned int size) {

    //return write(handler, src, size);
    return -1;
}

int std_mkdir(const char * dirname, int flags) {


    //return mkdir(dirname, flags);
    return -1;
}

int std_fsize(int handler) {

    //printf(".......fola: %d\n", filelength(handler));

    //sys_printf("......fsize\n");
    //return filelength(handler);
    return file_size;
}
//****************************************************************************** memory

void* std_malloc(size_t size) {

    //return malloc(size);
    //return alloca(size);
    //sys_printf("malloc: %x \n", size);

    void *ptr = (void *) &sys_ram[mall_ptr];
    if ((size & 1) != 0)size++;
    mall_ptr += size;

    if (mall_ptr > SIZE_SYS_RAM) {
        std_printf("Out of memory");
        std_exit(-1);
    }
    //printf(".............malloc %d, %d \n", (u32) size, mall_ptr / 1024);
    return ptr;

}

void* std_alloca(size_t size) {

    //dirty hack
    //sys_printf(".............alloca: %d\n", size);
    return std_malloc(size);
}

void* std_realloc(void *memory, size_t new_size) {

    //printf(".............reloc %d \n", new_size);

    void *ptr = std_malloc(new_size);
    std_memcpy(ptr, memory, new_size);
    return ptr;

    //return realloc(memory, new_size);
}

void* std_memset(void *dst, int val, size_t size) {

    //return memset(dst, val, size);
    u8 *dst8 = dst;

    while (size--)*dst8++ = val;

    return dst;
}

void* std_memcpy(void * dst, const void *src, size_t size) {

    //return memcpy(dst, src, size);
    u8 *dst8 = dst;
    const u8 *src8 = src;

    while (size--)*dst8++ = *src8++;

    return dst;
}

//****************************************************************************** string

char* std_strcpy(char * dst, const char * src) {

    //return strcpy(dst, src);
    u8 *dst8 = dst;
    const u8 *src8 = src;
    while (*src8)*dst8++ = *src8++;
    *dst8 = 0;

    return dst;
}

char* std_strncpy(char *dst, const char * src, size_t count) {

    //strncpy(dst, src, count);
    //return strncpy(dst, src, count);   

    u8 *dst8 = dst;
    const u8 *src8 = src;

    while (count--) {

        if (*src8 == 0) {
            *dst8++ = 0;
        } else {
            *dst8++ = *src8++;
        }
    }

    return dst;
}

int std_strcmp(const char *str1, const char *str2) {

    //return strcmp(str1, str2);
    while (1) {

        if (*str1 == 0 && *str2 == 0)return 0;
        if (*str1 > *str2)return 1;
        if (*str1 < *str2)return -1;
    }

}

int std_strcmpi(const char *str1, const char *str2) {

    //return strcmpi(str1, str2);
    u8 val1, val2;

    while (1) {

        val1 = *str1++;
        val2 = *str2++;
        if (val1 >= 'A' && val1 <= 'Z')val1 |= 0x20;
        if (val2 >= 'A' && val2 <= 'Z')val2 |= 0x20;

        if (val1 == 0 && val2 == 0)return 0;
        if (val1 > val2)return 1;
        if (val1 < val2)return -1;
    }
}

size_t std_strlen(const char *str) {

    //return strlen(str);
    size_t len = 0;

    while (*str++ != 0)len++;

    return len;
}

int std_strncasecmp(const char *str1, const char *str2, size_t max) {

    //return _strnicmp(str1, str2, max);

    u8 val1, val2;

    while (max--) {

        val1 = *str1++;
        val2 = *str2++;
        if (val1 >= 'A' && val1 <= 'Z')val1 |= 0x20;
        if (val2 >= 'A' && val2 <= 'Z')val2 |= 0x20;

        if (val1 == 0 && val2 == 0)return 0;
        if (val1 > val2)return 1;
        if (val1 < val2)return -1;
    }

    return 0;
}

char* std_strupr(char *str) {

    //return strupr(str);

    while (*str != 0) {

        if (*str >= 'a' && *str <= 'z')*str &= ~0x20;
        str++;
    }

    return str;
}

char* std_strcat(char* dst, const char *src) {

    //return strcat(dst, src);
    //not tested
    char *dst_new = dst + std_strlen(dst);
    std_strcpy(dst_new, src);

    return dst;
}

int std_printf(const char *str, ...) {

    void **args = (void *) &str;

    char *s = *args++;

    while (*s != 0) {

        if (*s == '%') {

            switch (s[1]) {
                case 'd':
                case 'u':
                case 'i':
                    gAppendNum((s32) * args);
                    break;
                case 's':
                    gAppendString((u8 *) * args);
                    break;
                case 'c':
                    gAppendChar((u32) * args);
                    break;
                case 'x':
                case 'X':
                    gAppendHex32((u32) * args);
                    break;
                default:
                    gConsPrint("unknown pf: ");
                    gAppendChar(s[1]);
                    break;
            }

            args++;
            s++;
        } else if (*s == '\n') {
            gConsPrint("");
        } else {
            gAppendChar(*s);
        }

        s++;
    }

    return 1;
}

int std_sprintf(char *dst, const char *str, ...) {

    void **args = (void *) &str;
    char *ptr;
    char *d;

    d = (u8*) dst;

    dst[0] = 0;
    char *s = *args++;
    char *sxx = s;


    while (*s != 0) {

        if (*s == '%') {

            switch (s[1]) {
                case '.':
                    // sys_sprintf(buffer, "STCFN%.3d", j++);
                    dst = str_append_num_z(dst, (s32) * args, s[2] - '0');
                    s++;
                    s++;
                    break;
                case 's':
                    ptr = (char *) *args;
                    while (*ptr != 0)*dst++ = *ptr++;
                    *dst = 0;
                    break;
                case 'i':
                case 'd':
                    dst = str_append_num(dst, (s32) * args);
                    break;
                default:
                    std_printf("unknown sp: %c, %s\n", s[1], sxx);
                    break;
            }

            *dst = 0;
            s++;
            args++;

        } else {
            *dst++ = *s;
            *dst = 0;
        }

        s++;
    }

    return 1;
}

int std_fprintf(FILE *file, const char *format, ...) {

    return 0;
}

//****************************************************************************** var

void std_exit(int code) {

    gConsPrint("exit: ");
    if (code < 0) {
        gAppendString("-");
        code = -code;
    }
    gAppendNum(code);
    while (1);
    //exit(code);
}

int sys_get_time() {

    static int time;
    time += 28 * 4;
    return time;
}

void sys_vsync() {
    //Sleep(1);
}

int std_strncmp(const char *str1, const char *str2, size_t max) {

    while (max--) {

        if (*str1 == 0 && *str2 == 0)return 0;
        if (*str1 > *str2)return 1;
        if (*str1 < *str2)return -1;
    }

    return 0;
}

int sys_toupper(int c) {

    if (c >= 'a' && c <= 'z')c &= ~0x20;
    return c;
}

int std_atoi(const char *str) {
    return 1;
}

int abs(int val) {
    if (val < 0)return -val;
    return val;
}

int min(int val1, int val2) {

    if (val2 < val1)return val2;
    return val1;
}
