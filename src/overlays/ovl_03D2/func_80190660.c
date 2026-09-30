/* ovl_03D2 (PE.IMG subsystem overlay, VRAM 0x8018EFF0)
 * func_80190660 — blob offset 0x1670, 0x354 bytes. Profile era_o2_g0_no_cse_skip_blocks.
 * Title/logo fade: loads two TIM blocks, builds double-buffered DR_TPAGE+SPRT pairs, then
 * 480 frames of fade-in/hold/fade-out with per-frame LoadImage of the buffer's image rect.
 * Levers: -fno-cse-skip-blocks (else the &r argument becomes a loop-invariant pseudo hoisted
 * into $s3); fade value computed in its own variable and copied through an asm barrier into
 * `c` (so t dies before c is born and both share $s0); `tp[i & 1]` after the D_801D11C8 toggle;
 * chained `BC[0]->isrgb24 = BC[1]->isrgb24 = v` stores; sprite field store order as retail.
 * Evidence: docs/evidence/ovl8-lane-2026-09-28/REPORT.md */
typedef struct { short x, y, w, h; } RECT;
typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short w, h;
} SPRT;
typedef struct {
    unsigned int tag;
    unsigned int code[1];
} DR_TPAGE;
typedef struct {
    unsigned char draw[0x5C];
    RECT disp;
    RECT screen;
    unsigned char isinter;
    unsigned char isrgb24;
    unsigned char pad0, pad1;
    RECT img;
} Env;

extern int D_80193278;
extern unsigned char D_80193254[];
extern Env *D_801D11BC[2];
extern Env *D_801D11C4;
extern int D_801D11C8;
extern void func_8007506C();
extern void func_80074DC0();
extern void func_80077C84();
extern void func_80074D28();
extern void func_80075358();
extern void func_80073A44();
extern void func_80074A44();
extern void func_80075424();
extern void func_800755F0();

void func_80190660(void)
{
    SPRT sp[2][2];
    DR_TPAGE tp[2][2];
    RECT r;
    unsigned char *p;
    SPRT *s;
    DR_TPAGE *t;
    int i;
    int c;
    int v;
    unsigned int sz;

    p = D_80193254 + D_80193278;
    func_8007506C(p + 0xC, p + 0x14);
    sz = *(unsigned int *)(p + 8);
    p += 8;
    p += (sz >> 2) << 2;
    func_8007506C(p + 4, p + 0xC);
    func_80074DC0(0);
    for (i = 0; i < 2; i++) {
        s = sp[i & 1];
        t = tp[i & 1];
        func_80077C84(t, 0, 0, 0x18);
        s->x0 = 0x20;
        s->y0 = 0x58;
        s->w = 0x100;
        ((unsigned char *)&s->tag)[3] = 4;
        s->code = 0x64;
        s->v0 = 0;
        s->u0 = 0;
        s->clut = 0x7800;
        s->h = 0x40;
        s++;
        func_80077C84(t + 1, 0, 0, 0x19);
        s->x0 = 0x11C;
        s->w = 8;
        ((unsigned char *)&s->tag)[3] = 4;
        s->code = 0x64;
        s->y0 = 0x50;
        s->v0 = 0;
        s->u0 = 0;
        s->clut = 0x7800;
        s->h = 0x50;
    }
    D_801D11BC[0]->isrgb24 = D_801D11BC[1]->isrgb24 = 0;
    func_80074D28(1);
    for (i = 0; i < 0x1E0; i++) {
        s = sp[i & 1];
        D_801D11C8 = D_801D11C8 == 0;
        D_801D11C4 = D_801D11BC[D_801D11C8];
        t = tp[i & 1];
        if (i < 0x20) {
            v = i * 4;
        } else if (i < 0x1A8) {
            if (i >= 0x188) {
                v = (0x1A8 - i) * 4;
            } else {
                v = 0x80;
            }
        } else {
            v = 0;
        }
        c = v;
        asm("" : "=r"(c) : "0"(c));
        s->b0 = c;
        s->g0 = c;
        s->r0 = c;
        func_80075358(t);
        func_80075358(s);
        s++;
        s->b0 = c;
        s->g0 = c;
        s->r0 = c;
        func_80074DC0(0);
        if (D_801D11C4->img.w > 0) {
            r = D_801D11C4->img;
            r.x = (r.x * 3) >> 1;
            if (D_801D11C8 == 0) {
                r.y += 0xF0;
            }
            r.w = (r.w * 3) >> 1;
            func_8007506C(&r, (unsigned char *)D_801D11C4 + 0x8080);
        }
        func_80073A44(0);
        func_80074A44(1);
        func_80075424(D_801D11C4);
        func_800755F0(D_801D11C4->draw + 0x5C);
    }
    func_80074D28(0);
    D_801D11BC[0]->isrgb24 = D_801D11BC[1]->isrgb24 = 1;
}
