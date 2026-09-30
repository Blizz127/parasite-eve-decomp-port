/* ovl_03D2 (PE.IMG subsystem overlay, VRAM 0x8018EFF0)
 * func_80191DC8 — blob offset 0x2DD8, 0x1F0 bytes. Profile era_o2_g0_no_schedule_insns.
 * Double-buffered slice advance: copies the RECT at D_801D148C, toggles the buffer index
 * D_801D1478, steps x by w, and either submits the next slice (func_8010C01C) or flips the
 * entry table D_801D147A and re-seeds x/y. Levers (docs/evidence/ovl6-lane-2026-09-27/REPORT.md):
 * -fno-schedule-insns; tab[2]/cur/ent/idx as one struct (gives the lw -8(&cur) fold); a volatile
 * first read of cur (retail loads it twice); x/w unsigned short temps loaded before the cur
 * store; D_801D147C char-array read for .b (non-in-struct → stays after the x store);
 * v = w*h before the tab load and an asm barrier on the loaded tab pointer. */
typedef struct { short x, y, w, h; } RECT;
typedef struct { short a, b, c, d; } Ent;
typedef struct {
    void *tab[2];          /* 0x00 D_801D1470 */
    unsigned char cur;     /* 0x08 D_801D1478 */
    unsigned char pad9;
    Ent ent[2];            /* 0x0A D_801D147A */
    unsigned char idx;     /* 0x1A D_801D148A */
} Buf;
extern signed char D_800B0DBB;
extern short D_800B0CD0;
extern Buf D_801D1470;
extern char D_801D147C[];
extern short D_801D148C;
extern short D_801D148E;
extern short D_801D1490;
extern short D_801D1492;
extern unsigned char D_801D1494;
extern unsigned char D_801D0DC0;
extern int D_8009CDDC;
extern void func_8007C564();
extern void func_8010C01C();
extern void func_801918F8();
extern void func_8007506C();

void func_80191DC8(void)
{
    RECT r;
    unsigned char cur;
    unsigned char nxt;
    unsigned char n;
    int i, j;
    unsigned short x, w;
    signed char *f;
    int v;
    void *a;

    if (D_800B0DBB != 0 && D_800B0CD0 != 0) {
        func_8007C564();
        D_800B0CD0 = 0;
    }
    r = *(RECT *)&D_801D148C;
    cur = *(volatile unsigned char *)&D_801D1470.cur;
    n = D_801D1470.cur;
    x = D_801D148C; w = D_801D1490;
    D_801D1470.cur = nxt = n ^ 1;
    i = D_801D1470.idx;
    D_801D148C = x += w;
    if ((short)x < D_801D1470.ent[i].a + D_801D1470.ent[i].c) {
        v = (short)w * D_801D1492;
        a = D_801D1470.tab[nxt];
        asm("" : "=r"(a) : "0"(a));
        func_8010C01C(a, v / 2);
    } else {
        D_801D1494 = 1;
        D_801D1470.idx = j = i ^ 1;
        D_801D148C = D_801D1470.ent[j].a;
        D_801D148E = *(short *)(D_801D147C + j * 8);
        if (D_801D0DC0 == 1) {
            f = &D_800B0DBB;
            if ((*f ^= 1) != 0) {
                D_801D1490 = 0x18;
            } else {
                D_801D1490 = 0x10;
            }
            func_801918F8((signed char)(D_8009CDDC ^ 1), D_800B0DBB);
            D_801D0DC0 = 2;
        }
    }
    func_8007506C(&r, D_801D1470.tab[(signed char)cur]);
}
