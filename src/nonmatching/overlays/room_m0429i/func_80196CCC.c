/* room_m0429i — func_80196CCC, blob offset 0x7CE4, 0x620 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback, modes 0/1/2 on a record a1, read from the retail
 * disassembly. Mode 0 zeroes +0, sets +2 = 0x10, plays sound 0x5E1/0x5E2 and
 * arms func_80196748 (0x10, 0x32). Mode 1 counts +0 up; on odd ticks it spawns
 * one state-2 spark with three random coordinates, every 4th tick (while +2 >
 * 0) a state-0 spark with a +-16 velocity (and decrements +2), and on tick 1 a
 * burst of 26 state-1 sparks; done at tick 0x30. Mode 2 places the D_800F3368
 * draw block, reads a world position through func_800CE8F0 into D_8019B670,
 * draws three func_800CEE20 sprites and two func_800D0728 rings (size by
 * tick*128/48), and finishes by re-seeding the draw block.
 * inline GTE/asm: none (the Psy-Q SetRot/SetTrans macros of the twins are
 * not used by this callback). */
typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned char c[4]; } B4;
typedef struct { unsigned char c[8]; } B8;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern B8 func_8018F210;
extern B4 D_8018F230;
extern GteShortVector D_8019B670;
extern unsigned char D_8019B404[];
extern short D_800942EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern void *D_800F33E0;
extern void *D_800F32D0;
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80196748();
extern int func_800D3FD8();
extern int func_800D3F64();
extern int func_800CE560();
extern unsigned short *func_800CE610();
extern void func_800CFFAC();
extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077AA4();
extern void func_800CE8F0();
extern void func_800CF3AC();
extern void func_800CEE20();
extern void func_800D004C();
extern void func_800D0728();

int func_80196CCC(int mode, unsigned char *a1)
{
    struct {                    /* retail frame 0x30..0x60 */
        B8 vec;
        GteShortVector p;
        int col0, col1;
        B4 buf;
        int pad;
        GteShortVector v50;
        GteShortVector m58;
    } L;
    unsigned short *e;
    int i;
    int s1;
    int s4;
    int t;
    int w;
    int clut;

    L.vec = func_8018F210;
    L.buf = D_8018F230;
    L.col0 = 0;
    L.col1 = 0;
    L.pad = 0;
    switch (mode) {
    case 0:
        SH(a1, 0) = 0;
        SH(a1, 2) = 0x10;
        func_800D3F64(0x5E1, func_800D3FD8());
        func_800D3F64(0x5E2, 0x80);
        return func_800CE560(P(D_800F33E0, 8), 0x10, 0x32, func_80196748);
    case 1:
        H(a1, 0)++;
        if ((H(a1, 0) & 1) != 0) {
            e = func_800CE610(P(D_800F33E0, 8));
            if (e != 0) {
                e[0] = func_80071A54();
                e[1] = func_80071A54();
                e[2] = func_80071A54();
                e[5] = 0;
                e[4] = 2;
            }
        }
        if ((H(a1, 0) & 3) == 0 && SH(a1, 2) > 0) {
            e = func_800CE610(P(D_800F33E0, 8));
            if (e != 0) {
                func_800CFFAC(e);
                e[5] = 0;
                e[4] = 0;
                e[6] = (func_80071A54() & 0x1F) - 0x10;
                e[7] = (func_80071A54() & 0x1F) - 0x10;
            }
            H(a1, 2)--;
        }
        if (SH(a1, 0) == 1) {
            for (i = 0; i < 26; i++) {
                e = func_800CE610(P(D_800F33E0, 8));
                if (e != 0) {
                    func_800CFFAC(e);
                    e[5] = 0;
                    e[4] = 1;
                    e[6] = (func_80071A54() & 0x1FF) + 0x258;
                    e[7] = (func_80071A54() & 7) - 3;
                }
            }
        }
        if (SH(a1, 0) < 0x30)
            return 0;
        return 1;
    case 2:
        D_800F3368.a6A = 4;
        D_800F3368.a68 = 0x40;
        D_800F3368.a76 = 0x40;
        D_800F3368.a78 = 0x40;
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x18;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        L.p.w = 0;
        func_800CE8F0(P(D_800F32D0, 8), 0, &L.vec, &L.p);
        D_8019B670.x = L.p.x;
        D_8019B670.y = L.p.y;
        D_8019B670.z = L.p.z;
        s4 = ((H(a1, 0) & 1) != 0) ? 0x60 : 0x80;
        t = func_80077CF4((SH(a1, 0) << 10) / 48);
        s1 = t / 2 + 0x800;
        func_800CF3AC(D_8019B404, &L.col0, SH(a1, 0));
        func_800D004C(&L.p, 0x320, 0x320, 0xC, 0, s1, s1, &L.col0, 0, 100, 1);
        w = D_800E1204[D_800F3368.a6C];
        if (D_800F3368.a6C == 4 && D_800F3428 != 0)
            w += 4;
        clut = func_80077AA4(0x90, w);
        func_800CEE20(&L.p, 0, s1, s1, 8, clut & 0xFFFF, 1, s4, 0);
        L.v50.x = L.p.x;
        L.v50.y = D_800942EC;
        L.v50.z = L.p.z;
        L.v50.w = 0;
        L.m58.x = 0x400;
        L.m58.y = 0;
        L.m58.z = 0;
        L.m58.w = 1;
        w = D_800E1204[D_800F3368.a6C];
        if (D_800F3368.a6C == 4 && D_800F3428 != 0)
            w += 4;
        clut = func_80077AA4(0x10, w);
        func_800CEE20(&L.v50, &L.m58, s1, s1, 8, clut & 0xFFFF, 3, s4, 0);
        s4 = (SH(a1, 0) << 7) / 48;
        func_800D0728(&L.p, 0x44C, 0x1770, 0x10, 0, 0x1000, 0x1000, 0, &L.buf, s4, 1);
        func_800D0728(&L.p, 0x3E8, 0x4B0, 0x18, 0, 0x1000, 0x1000, &L.buf, 0, s4, 1);
        D_800F3368.a68 = 0x40;
        D_800F3368.a6A = 4;
        D_800F3368.a76 = 0x40;
        D_800F3368.a78 = 0x40;
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x18;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        return 0;
    }
    return 0;
}
