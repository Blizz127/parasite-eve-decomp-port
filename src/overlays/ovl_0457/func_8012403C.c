/* ovl_0457 (PE.IMG credits overlay, VRAM 0x80120D00)
 * func_8012403C — blob offset 0x333C, 0x55C bytes. Local profile
 * ovl_0457_dispatch_8012359C_expand_div (-O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 +
 * MASPSX_DISPATCH_FOLD=jtbl_8012359C + MASPSX_EXPAND_DIV=1).
 * Credits page state machine: draws the current page sprite with a fade, then streams the next
 * page (load, decode into VRAM, 16-column LoadImage upload, hold, advance). Levers: the % 450
 * divisor goes through an asm barrier (retail divu by a register + `n - 93` from it); the VRAM
 * address arguments are built in pinned $a1/$a2/$a3 locals in retail's association order;
 * case 2 uses its own unpinned local. Evidence: docs/evidence/ovl8-lane-2026-09-28/REPORT.md */
typedef struct { short x, y, w, h; } RECT;
typedef struct {
    unsigned int tag;
    unsigned int code[1];
} DR_TPAGE;
typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short w, h;
} SPRT;
typedef struct {
    DR_TPAGE tp;
    SPRT s;
} Page;
typedef struct {
    unsigned short x, y;
    unsigned short pad[2];
    unsigned short c, d;
    unsigned short pad2[2];
} PageDef;

extern int D_80172CBC;
extern PageDef D_80125B84[];
extern Page *D_80172CB8;
extern unsigned short D_80172CC4;
extern int D_80172CD8;
extern int D_80172CDC;
extern unsigned int *D_80172CB4;
extern unsigned int D_80172CCC;
extern unsigned int D_80125BA4[];
extern unsigned short D_80093168;
extern unsigned short D_80093166;
extern int D_80011610;
extern unsigned short D_80172CC8;
extern int D_80172CC0;
extern unsigned int D_80172CD4;
extern unsigned short func_80077A64();
extern void func_80077C84();
extern void func_80077CB4();
extern void func_80077AC4();
extern unsigned int func_8010BF94();
extern void func_8010C86C();
extern int func_8010C89C();
extern void func_8010BFA0();
extern void func_8010C01C();
extern int func_8010C078();
extern void func_8007506C();

void func_8012403C(void)
{
    Page *p;
    RECT r;
    unsigned int t;
    register int d asm("$6");
    register int a5 asm("$5");
    register int k7 asm("$7");
    int e;
    unsigned int n;
    unsigned int v;
    int a;
    int b;

    p = D_80172CB8;
    func_80077C84(p, 0, 0, func_80077A64(2, 0, D_80125B84[D_80172CBC].x, D_80125B84[D_80172CBC].y));
    ((unsigned char *)&p->s.tag)[3] = 4;
    p->s.code = 0x64;
    func_80077CB4(p, &p->s);
    switch (D_80172CC4 & 3) {
    case 0:
        p->s.x0 = 0xA0;
        p->s.y0 = 0;
        break;
    case 1:
        p->s.x0 = 0;
        p->s.y0 = 0;
        break;
    case 2:
        p->s.x0 = 0xA0;
        p->s.y0 = 0x69;
        break;
    case 3:
        p->s.x0 = 0;
        p->s.y0 = 0x69;
        break;
    }
    p->s.u0 = 0;
    p->s.v0 = 0;
    p->s.r0 = D_80172CD8;
    p->s.g0 = D_80172CD8;
    p->s.b0 = D_80172CD8;
    p->s.w = 0xA0;
    p->s.h = 0x78;
    if (D_80172CDC == 0) {
        D_80172CD8 += 4;
        if (D_80172CD8 >= 0x80) {
            D_80172CD8 = 0x80;
            D_80172CDC = 1;
        }
    } else if (D_80172CDC == 2) {
        D_80172CD8 -= 4;
        if (D_80172CD8 <= 0) {
            D_80172CD8 = 0;
            D_80172CDC = 1;
        }
    }
    func_80077AC4(D_80172CB4 + 14, p);
    D_80172CB8++;
    switch (D_80172CCC) {
    case 0:
        func_8010C86C(func_8010BF94(D_80125BA4[D_80172CC4]) >> 2);
        a5 = 0x38800;
        k7 = 0x43000;
        d = D_80093168; d -= D_80093166; d <<= 11;
        a5 += d; a5 = D_80011610 + a5;
        func_8010C89C(D_80125BA4[D_80172CC4], a5, D_80011610 + (d += k7));
        D_80172CCC = 1;
        break;
    case 1:
        d = D_80093168; d -= D_80093166; d <<= 11;
        b = d + 0x43000;
        if (func_8010C89C(0, 0, D_80011610 + b) == 0) {
            D_80172CCC = 2;
            D_80172CC8 = 0;
        }
        break;
    case 2:
        e = D_80093168; e -= D_80093166; e <<= 11;
        D_80172CC0 = D_80011610 + e;
        a = e + 0x38800;
        func_8010BFA0(D_80011610 + a, 0);
        func_8010C01C(D_80172CC0, 0x2800);
        D_80172CCC = 3;
        break;
    case 3:
        if (func_8010C078(1) == 0) {
            D_80172CCC = 4;
            D_80172CC8 = D_80125B84[D_80172CBC].c;
        }
        break;
    case 4:
        if (D_80172CC8 < D_80125B84[D_80172CBC].c + 0xA0) {
            r.x = D_80172CC8;
            r.y = D_80125B84[D_80172CBC].d;
            r.w = 0x10;
            r.h = 0x80;
            func_8007506C(&r, D_80172CC0);
            D_80172CC0 += 0x1000;
            D_80172CC8 += 0x10;
        } else {
            D_80172CCC = 5;
        }
        break;
    case 5:
        v = D_80172CD4;
        n = 450;
        asm("" : "=r"(n) : "0"(n));
        t = v % n;
        if (t == n - 93) {
            D_80172CDC = 2;
        }
        if (t == 0) {
            D_80172CCC = 6;
            D_80172CDC = 0;
        }
        break;
    case 6:
        if (++D_80172CC4 < 30) {
            D_80172CCC = 0;
        } else {
            D_80172CCC = 7;
        }
        D_80172CDC = 0;
        D_80172CBC ^= 1;
        break;
    case 7:
        v = D_80172CD4;
        n = 450;
        asm("" : "=r"(n) : "0"(n));
        t = v % n;
        if (t == n - 93) {
            D_80172CDC = 2;
        }
        break;
    }
}
