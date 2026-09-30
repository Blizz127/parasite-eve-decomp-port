/* ovl_03C5 (PE.IMG movie-controller overlay, VRAM 0x80120D00)
 * func_80121A00 — blob offset 0xD00, 0x204 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03C5-func_80121A00/REPORT.md).
 * Movie teardown: clear the stream state, restore both DISPENV/DRAWENV pairs from
 * D_801227EC/D_80122814 and move the movie VRAM areas back (func_8007512C). */

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
extern unsigned char D_800B0DBA;
extern int D_80122420;
extern int D_80122424;
extern int D_80122428;
extern int D_8012242C;
extern int D_80122430;
extern int D_80122434;
extern DISPENV D_800BCE80[];
extern DISPENV D_801227EC[];
extern DRAWENV D_800BCDC8[];
extern DRAWENV D_80122814[];
extern unsigned int D_800B0CD8;
extern void func_80074DC0();
extern void func_80073A44();
extern void func_80074D28();
extern void func_8007512C();
void func_80121A00(void)
{
    unsigned char *p = &D_800B0DBA;
    signed char i;
    RECT r;

    if (*p != 0) {
        D_80122420 = 0;
        D_80122424 = 0;
        D_80122434 = 0;
        D_80122430 = 0;
        D_80122428 = 0;
        D_8012242C = 0;
        *p = 0;
        func_80074DC0(0);
        func_80073A44(0);
        func_80074D28(0);
        for (i = 0; i < 2; i++) {
            D_800BCE80[i] = D_801227EC[i];
            D_800BCDC8[i] = D_80122814[i];
        }
        r.x = 512;
        r.y = 0;
        r.w = 192;
        r.h = 256;
        func_8007512C(&r, 320, 0);
        if (!(D_800B0CD8 & 0x8000000)) {
            r.x = 512;
            r.y = 256;
            r.w = 320;
            r.h = 64;
            func_8007512C(&r, 0, 448);
        }
    }
}
