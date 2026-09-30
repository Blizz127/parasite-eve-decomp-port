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
extern DISPENV D_801D1384[];
extern DRAWENV D_801D13AC[];
extern unsigned char D_800B0DBA;
extern unsigned int D_800B0CD8;
extern int D_801D0DE8;
extern int D_801D0DEC;
extern int D_801D0DF0;
extern int D_801D0DF4;
extern int D_801D0DF8;
extern int D_801D0DFC;
extern void func_80074DC0();
extern void func_80073A44();
extern void func_80074D28();
extern void func_8007512C();

void func_801922F4(void)
{
    RECT r;
    signed char i;
    unsigned char *p;

    p = &D_800B0DBA;
    if (*p != 0) {
        D_801D0DE8 = 0;
        D_801D0DEC = 0;
        D_801D0DFC = 0;
        D_801D0DF8 = 0;
        D_801D0DF0 = 0;
        D_801D0DF4 = 0;
        *p = 0;
        func_80074DC0(0);
        func_80073A44(0);
        func_80074D28(0);
        for (i = 0; i < 2; i++) {
            D_800BCE80[i] = D_801D1384[i];
            D_800BCDC8[i] = D_801D13AC[i];
        }
        r.x = 0x200;
        r.y = 0;
        r.w = 0xC0;
        r.h = 0x100;
        func_8007512C(&r, 0x140, 0);
        if (!(D_800B0CD8 & 0x8000000)) {
            r.x = 0x200;
            r.y = 0x100;
            r.w = 0x140;
            r.h = 0x40;
            func_8007512C(&r, 0, 0x1C0);
        }
    }
}
