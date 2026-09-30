/* room_m0430i — func_801906EC, blob offset 0x1704, 0xF4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * effect slot setup in D_80190860[24B] by mode 1-4; levers: rand() before the +0xC store, scalar *(short *)p store orders before the D_800942EC load */

typedef struct { short x0, x2, x4, x6, x8, xA, xC, xE; int x10; int x14; } Slot;
extern Slot D_80190860[];
extern unsigned short D_800942EC;
extern int func_80071A54();

Slot *func_801906EC(unsigned int a0, int a1, int a2, int a3)
{
    Slot *p = &D_80190860[a1];
    int r;

    p->x14 = 1;
    switch (a0) {
    case 1:
        p->x8 = a2;
        r = func_80071A54();
        p->xC = a3;
        p->x14 = 0;
        p->xA = D_800942EC - 600 - (r & 0x1FF);
        break;
    case 2:
        *(short *)p = a2;
        p->x4 = a3;
        p->x2 = D_800942EC;
        break;
    case 3:
        p->x8 = a2;
        r = func_80071A54();
        p->xC = a3;
        p->xA = D_800942EC - 600 - (r & 0x1FF);
        break;
    case 4:
        p->x10 = a2;
        break;
    }
    return p;
}
