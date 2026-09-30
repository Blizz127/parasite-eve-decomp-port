/* ovl_03D2 (PE.IMG subsystem overlay, VRAM 0x8018EFF0)
 * func_80191B64 — blob offset 0x2B74, 0x264 bytes. Profile era_o2_g0 (default).
 * Movie frame poll: waits up to 2000 tries on func_8007C484 (StGetNext-style addr/header), fades
 * volume over the last 16 frames, flags a wrap in D_801D0DBD, re-clears the display RECT when the
 * frame size changes, and writes the (x1.5 when widescreen) frame size into the sprite.
 * Levers (docs/evidence/ovl6-lane-2026-09-27/REPORT.md): func_800719E4 (exit) noreturn;
 * RECT declared in the inner block (frame layout); u = frame + 16 then v = u - t; v = 14 - v;
 * per-branch r.h stores (cross-jumped tail); s->x0 = s->x1 = ... assignment order. */
typedef struct { short x, y, w, h; } RECT;
typedef struct {
    unsigned char pad0[8];
    unsigned int frame;    /* +0x08 */
    unsigned char padC[4];
    unsigned short w;      /* +0x10 */
    unsigned short h;      /* +0x12 */
} StHdr;
typedef struct {
    unsigned char pad0[0x1A];
    short x0;              /* +0x1A */
    short y0;              /* +0x1C */
    unsigned char pad1E[4];
    short x1;              /* +0x22 */
    short y1;              /* +0x24 */
    unsigned char pad26[8];
    short y2;              /* +0x2E */
} Spr;
typedef struct { unsigned char pad[8]; short end; } Mov;
extern Mov *D_801D11AC;
extern short D_801D11B0;
extern unsigned char D_800B0DBE;
extern signed char D_800B0DBB;
extern unsigned char D_801D0DBD;
extern short D_801D0DE0[2];
extern unsigned short D_800B0DD4;
extern int func_8007F72C();
extern int func_8007F7A8();
extern void func_800719E4() __attribute__((noreturn));
extern int func_8007C484();
extern void func_800870F0();
extern void func_80074F44();

int func_80191B64(Spr *s)
{
    int addr;
    StHdr *hdr;
    int n;
    int v;
    int t;
    unsigned int u;

    n = 2000;
    if (func_8007F72C() == 1) {
        if (func_8007F7A8() != D_800B0DD4) {
            func_800719E4(1);
        }
    }
    while (func_8007C484(&addr, &hdr) != 0) {
        if (--n == 0) {
            return 0;
        }
    }
    t = D_801D11AC->end;
    if (hdr->frame >= t - 16) {
        u = hdr->frame + 16;
        v = u - t;
        v = 14 - v;
        if (v < 0) {
            v = 0;
        }
        func_800870F0((D_800B0DBE * v) / 14);
    }
    if (hdr->frame < D_801D11B0 || hdr->frame >= D_801D11AC->end) {
        D_801D0DBD = 1;
    }
    D_801D11B0 = hdr->frame;
    if (D_801D0DE0[0] != hdr->w || D_801D0DE0[1] != hdr->h) {
        RECT r;

        if (D_800B0DBB != 0) {
            r.x = 0;
            r.y = 0;
            r.w = 0x1E0;
            r.h = 0x1E0;
        } else {
            r.x = 0;
            r.y = 0;
            r.w = 0x140;
            r.h = 0x1E0;
        }
        func_80074F44(&r, 0, 0, 0);
        D_801D0DE0[0] = hdr->w;
        D_801D0DE0[1] = hdr->h;
    }
    if (D_800B0DBB != 0) {
        s->x0 = s->x1 = (D_801D0DE0[0] * 3) / 2;
    } else {
        s->x0 = s->x1 = D_801D0DE0[0];
    }
    s->y0 = s->y1 = D_801D0DE0[1];
    s->y2 = D_801D0DE0[1];
    return addr;
}
