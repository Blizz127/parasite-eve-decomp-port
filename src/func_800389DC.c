typedef struct Rd {
    int base;
    unsigned char pad4[0x90];
    unsigned char *buf;
} Rd;

typedef struct Sv {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
    unsigned char a[24];
    unsigned char c;
    unsigned char d[100];
    unsigned char e[100];
    unsigned char f[100];
} Sv;

extern Rd D_800B0DD8;
extern unsigned short D_80093176[];
extern Sv D_8009ECD8;
extern int func_8006E6A8(int, unsigned char *, int);
extern int func_8006E7E8(void);

int func_800389DC(int slot)
{
    int v;
    int i;
    int k;
    int off;

retry:
    while (func_8006E6A8(D_800B0DD8.base + D_80093176[0], D_800B0DD8.buf,
                         D_80093176[1] - D_80093176[0]) == -1) {
    }
    while ((v = func_8006E7E8()) != 0) {
        if (v == -1) {
            goto retry;
        }
    }
    off = (unsigned char)slot * 328;
    k = off + 1;
    D_8009ECD8.b1 = D_800B0DD8.buf[k++];
    D_8009ECD8.b2 = D_800B0DD8.buf[k++];
    D_8009ECD8.b3 = D_800B0DD8.buf[k++];
    for (i = 0; i < 24; i++) {
        D_8009ECD8.a[i] = D_800B0DD8.buf[k++];
    }
    D_8009ECD8.c = D_800B0DD8.buf[k++];
    for (i = 0; i < 100; i++) {
        D_8009ECD8.d[i] = D_800B0DD8.buf[k++];
    }
    for (i = 0; i < 100; i++) {
        D_8009ECD8.e[i] = D_800B0DD8.buf[k++];
    }
    for (i = 0; i < 100; i++) {
        D_8009ECD8.f[i] = D_800B0DD8.buf[k++];
    }
    return 0;
}
