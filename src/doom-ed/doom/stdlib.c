
#include "sys.h"
#include "bios.h"



u8 *str_append(u8 *dst, u8 *src);
u8 *str_append_num(u8 *dst, s32 num);
u8 *str_append_num_z(u8 *dst, s32 num, u8 zeros);

u32 file_size;
u32 file_offset;
u8 *sys_ram;
u32 mall_ptr;

void stdInit() {

    mall_ptr = 0;
    sys_ram = (u8 *) ADDR_SYS_RAM;
}

//****************************************************************************** file io

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

    return 1;
}

int std_read(int handler, void *dst, unsigned int size) {

    u8 resp;

    size = std_min(size, file_size - file_offset);

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

void* std_memset(void *dst, u8 val, size_t size) {

    //return memset(dst, val, size);
    if (size > 32) {
        bi_cmd_mem_set(val, (u32) dst, size); //use io on-board mcu for faster memset
        return dst;
    }


    u8 mask = size | (u32) dst;

    //if (((u16) dst & 1 == 0) && (size & 1 == 0)) {
    if ((mask & 1) == 0) {

        u16 *dst16 = dst;
        u16 val16 = (val << 8) | val;
        size /= 2;

        while (size--)* dst16++ = val16;

    } else {
        u8 *dst8 = dst;
        while (size--)*dst8++ = val;
    }

    return dst;
}

void* std_memcpy(void * dst, const void *src, size_t size) {

    //return memcpy(dst, src, size);
    u8 mask = size | (u32) src | (u32) dst;

    if ((mask & 1) == 0) {

        u16 *dst16 = dst;
        const u16 *src16 = src;
        size /= 2;
        while (size--)*dst16++ = *src16++;

    } else {

        u8 *dst8 = dst;
        const u8 *src8 = src;
        while (size--)*dst8++ = *src8++;
    }

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

int std_strncmp(const char *str1, const char *str2, size_t max) {

    while (max--) {

        if (*str1 == 0 && *str2 == 0)return 0;
        if (*str1 > *str2)return 1;
        if (*str1 < *str2)return -1;
    }

    return 0;
}

int std_toupper(int c) {

    if (c >= 'a' && c <= 'z')c &= ~0x20;
    return c;
}

int std_atoi(const char *str) {
    return 1;
}

int std_abs(int val) {
    if (val < 0)return -val;
    return val;
}

int std_min(int val1, int val2) {

    if (val2 < val1)return val2;
    return val1;
}

void std_vsync() {
    //Sleep(1);
}
