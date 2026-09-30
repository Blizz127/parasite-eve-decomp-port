/* ovl_03C5 (PE.IMG subsystem overlay) — port of ovl_03D2 func_80191DC8 (same body, remapped symbols)
 * func_801214D4 — blob offset 0x7D4, 0x1F0 bytes (VRAM 0x80120D00 overlay). Profile era_o2_g0_no_schedule_insns.
 * Double-buffered slice advance: copies the RECT at D_801228F4, toggles the buffer index
 * D_801228E0, steps x by w, and either submits the next slice (func_8010C01C) or flips the
 * entry table D_801228E2 and re-seeds x/y. Levers (docs/evidence/ovl6-lane-2026-09-27/REPORT.md):
 * -fno-schedule-insns; tab[2]/cur/ent/idx as one struct (gives the lw -8(&cur) fold); a volatile
 * first read of cur (retail loads it twice); x/w unsigned short temps loaded before the cur
 * store; D_801228E4 char-array read for .b (non-in-struct → stays after the x store);
 * v = w*h before the tab load and an asm barrier on the loaded tab pointer. */
typedef struct { short x, y, w, h; } RECT;
typedef struct { short a, b, c, d; } Ent;
typedef struct {
    void *tab[2];          /* 0x00 D_801228D8 */
    unsigned char cur;     /* 0x08 D_801228E0 */
    unsigned char pad9;
    Ent ent[2];            /* 0x0A D_801228E2 */
    unsigned char idx;     /* 0x1A D_801228F2 */
} Buf;
extern signed char D_800B0DBB;
extern short D_800B0CD0;
extern Buf D_801228D8;
extern char D_801228E4[];
extern short D_801228F4;
extern short D_801228F6;
extern short D_801228F8;
extern short D_801228FA;
extern unsigned char D_801228FC;
extern unsigned char D_801223F8;
extern int D_8009CDDC;
extern void func_8007C564();
extern void func_8010C01C();
extern void func_80121004();
extern void func_8007506C();

void func_801214D4(void)
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
    r = *(RECT *)&D_801228F4;
    cur = *(volatile unsigned char *)&D_801228D8.cur;
    n = D_801228D8.cur;
    x = D_801228F4; w = D_801228F8;
    D_801228D8.cur = nxt = n ^ 1;
    i = D_801228D8.idx;
    D_801228F4 = x += w;
    if ((short)x < D_801228D8.ent[i].a + D_801228D8.ent[i].c) {
        v = (short)w * D_801228FA;
        a = D_801228D8.tab[nxt];
        asm("" : "=r"(a) : "0"(a));
        func_8010C01C(a, v / 2);
    } else {
        D_801228FC = 1;
        D_801228D8.idx = j = i ^ 1;
        D_801228F4 = D_801228D8.ent[j].a;
        D_801228F6 = *(short *)(D_801228E4 + j * 8);
        if (D_801223F8 == 1) {
            f = &D_800B0DBB;
            if ((*f ^= 1) != 0) {
                D_801228F8 = 0x18;
            } else {
                D_801228F8 = 0x10;
            }
            func_80121004((signed char)(D_8009CDDC ^ 1), D_800B0DBB);
            D_801223F8 = 2;
        }
    }
    func_8007506C(&r, D_801228D8.tab[(signed char)cur]);
}
