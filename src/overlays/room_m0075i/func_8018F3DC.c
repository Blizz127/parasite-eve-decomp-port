/* room_m0075i — func_8018F3DC, blob offset 0x3F4, 0x338 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * Beam init + 2x10 sprite arrays; per-iteration SPR *p = &arr[i] base; global order via climb. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct {
    unsigned char b0, b1, b2, b3;
    short h4, h6, h8, hA, hC, hE;
    unsigned char b10, b11, b12, b13;
    unsigned char pad[0x30];
} SPR;
extern unsigned char D_801940B8, D_801940B9, D_801940BA, D_801940BC, D_801940BD, D_801940BE, D_80194618, D_80194619, D_8019461A, D_8019461C, D_8019461D, D_8019461E, D_80194628, D_80194629, D_8019462A, D_8019462C, D_8019462D, D_8019462E, D_80194638, D_80194639, D_8019463A, D_8019463C, D_8019463D, D_8019463E, D_80194648, D_80194649, D_8019464A, D_8019464C, D_8019464D, D_8019464E;
extern short D_801940C0, D_801940C2, D_80194620, D_80194622, D_80194630, D_80194632, D_80194640, D_80194642, D_80194650, D_80194652;
extern SPR D_801940C8[10];
extern SPR D_80194370[10];
extern void func_800C2B40();
extern int func_8006DC18();

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

void func_8018F3DC(void *a0, int a1, unsigned char *o)
{
    void *x;
    unsigned int i;
    SPR *p;

    func_800C2B40(o);
    W(o, 0x6C) = func_8006DC18(0xC);
    x = P(a0, 8);
    P(o, 0) = x;
    *(MATRIX *)(o + 4) = *(MATRIX *)P(x, 0x238);
    *(MATRIX *)(o + 0x44) = *(MATRIX *)((char *)P(P(o, 0), 0x238) + 0x120);
    *(MATRIX *)(o + 0x24) = *(MATRIX *)((char *)P(P(o, 0), 0x238) + 0x1A0);
    H(o, 0x64) = 0x1E;
    H(o, 0x66) = 0;
    H(o, 0x68) = 0;
    D_8019462C = 0x2c;
    D_8019462D = 0x1;
    D_80194630 = -0x1e;
    D_80194632 = 0x0;
    D_80194628 = 0x80;
    D_80194629 = 0x80;
    D_8019462A = 0x80;
    D_8019462E = 0x0;
    D_8019463C = 0x2c;
    D_8019463D = 0x1;
    D_80194640 = 0x0;
    D_80194642 = 0x20;
    D_80194638 = 0x80;
    D_80194639 = 0x80;
    D_8019463A = 0x80;
    D_8019463E = 0x0;
    D_8019461C = 0x2c;
    D_8019461D = 0x1;
    D_80194620 = -0x1e;
    D_80194622 = 0x80;
    D_801940C0 = -0x1f;
    D_8019464C = 0x20;
    D_80194618 = 0x80;
    D_80194619 = 0x80;
    D_8019461A = 0x80;
    D_8019461E = 0x0;
    D_801940BC = 0x0;
    D_801940BD = 0x1;
    D_801940C2 = 0x80;
    D_801940B8 = 0x80;
    D_801940B9 = 0x80;
    D_801940BA = 0x80;
    D_801940BE = 0x0;
    D_8019464D = 0x1;
    D_80194650 = 0x64;
    D_80194652 = 0x80;
    D_80194648 = 0x80;
    D_80194649 = 0x80;
    D_8019464A = 0x80;
    D_8019464E = 0x0;
    for (i = 0; i < 10; i++) {
        p = &D_801940C8[i];
        p->b1 = 4;
        p->h4 = 0x80;
        p->h6 = 4;
        p->h8 = 0x1E;
        p->hA = 0;
        if (i == 0) {
            p->hC = -0x40;
        } else {
            p->hC = 0;
        }
        p->b10 = 0xFF;
        p->b11 = 0xFF;
        p->b12 = 0xFF;
        p = &D_80194370[i];
        p->b1 = 4;
        p->h4 = 0x80;
        p->h6 = 4;
        p->h8 = 0x1E;
        p->hA = 0;
        if (i == 0) {
            p->hC = 0x40;
        } else {
            p->hC = 0;
        }
        p->b10 = 0xFF;
        p->b11 = 0xFF;
        p->b12 = 0xFF;
    }
}
