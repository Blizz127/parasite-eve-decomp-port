typedef struct { short x, y, w, h; } RECT;
typedef struct {
    RECT clip;
    short ofs[2];
    RECT tw;
    unsigned short tpage;
    unsigned char dtd;
    unsigned char dfe;
    unsigned char isbg;
    unsigned char r0, g0, b0;
    unsigned long dr_env[16];
} DRAWENV;
typedef struct {
    RECT disp;
    RECT screen;
    unsigned char isinter;
    unsigned char isrgb24;
    unsigned char pad0, pad1;
} DISPENV;

extern DISPENV D_800BCE80[];
extern DRAWENV D_800BCDC8[];
extern DISPENV D_801227EC[];
extern DRAWENV D_80122814[];
extern unsigned char D_800B0DBA;
extern unsigned int D_800B0CD8;
extern int D_80122420;
extern int D_80122424;
extern int D_80122428;
extern int D_8012242C;
extern int D_80122430;
extern int D_80122434;
extern void func_80074DC0();
extern void func_8007512C();
extern void func_80073A44();
extern void func_80074D28();
extern unsigned char D_800B0DBE;
extern short D_800B0DBC;
extern short D_801227E8;

int func_801216C4(signed char n, int *buf)
{
    RECT r;
    signed char i;
    int p;
    int k;
    int *q;
    int a, b, c, d, e, f;

    if (n != 1 && n != 2) {
        return 0;
    }
    for (i = 0; i < n; i++) {
        if (buf[i] == 0) {
            return 0;
        }
    }
    if (D_800B0DBA != 0) {
        return 0;
    }
    switch (n) {
    case 1:
        p = buf[0];
        k = 0xFA00;
        a = p + k;
        k = 0x1F400;
        b = p + k;
        k = 0x3F400;
        c = p + k;
        k = 0x50400;
        d = p + k;
        k = 0x53100;
        D_80122428 = d;
        D_80122420 = p;
        D_80122424 = a;
        D_80122434 = b;
        D_80122430 = c;
        D_8012242C = p + k;
        break;
    case 2:
        p = buf[0];
        k = 0xFA00;
        e = p + k;
        k = 0x1F400;
        f = p + k;
        D_80122420 = p;
        D_80122424 = e;
        D_80122434 = f;
        q = &buf[1];
        p = *q;
        k = 0x11000;
        D_80122428 = p + k;
        k = 0x13D00;
        D_80122430 = p;
        D_8012242C = p + k;
        break;
    }
    D_800B0DBA = 1;
    D_800B0DBE = 0x98;
    D_800B0DBC = 0;
    D_801227E8 = -1;
    for (i = 0; i < 2; i++) {
        D_801227EC[i] = D_800BCE80[i];
        D_80122814[i] = D_800BCDC8[i];
    }
    r.x = 0x140;
    r.y = 0;
    r.w = 0xC0;
    r.h = 0x100;
    func_8007512C(&r, 0x200, 0);
    if (!(D_800B0CD8 & 0x8000000)) {
        r.x = 0;
        r.y = 0x1C0;
        r.w = 0x140;
        r.h = 0x40;
        func_8007512C(&r, 0x200, 0x100);
    }
    return 1;
}
