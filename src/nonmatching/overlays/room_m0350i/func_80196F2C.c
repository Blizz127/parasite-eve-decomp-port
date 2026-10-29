/* room_m0350i — func_80196F2C, blob offset 0x7F44, 0x2F0 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Per-frame callback of a thrown object (record a1: +0..+4 velocity, +8..+0xC
 * position, +0xE landed flag).  mode 1 integrates the position, flags a
 * fall past the floor (D_800942EC) or a hit (func_8019721C) and otherwise
 * logs the point in a 4-entry trail at D_8019A85C - 0x28; mode 2 draws the
 * effect (func_800CEE20 + func_800D004C) at the record position. */
typedef struct { short x, y, z, w; } GteShortVector;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern short D_800942EC;
extern unsigned short D_800E11FA;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;
extern short D_800F336A;
extern short D_800F336C;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;
extern unsigned char D_8019A3C0[];
extern unsigned char D_8019A61C[];
extern signed char D_8019A856;
extern signed char D_8019A857;
extern short D_8019A85C;
extern unsigned short D_8019A860;
extern unsigned short D_8019A862;
extern unsigned short D_8019A864;
extern unsigned short D_8019A866;
extern unsigned short D_8019A868;

extern int func_8019721C();
extern int func_80077AA4();
extern void func_800CEE20();
extern void func_800D004C();

int func_80196F2C(int mode, unsigned char *a1)
{
    GteShortVector v;
    int idx;
    unsigned short *e;
    int a2;
    int t;
    unsigned short a3;

    if (mode == 1) {
        if (a1[0xE] != 0)
            return 1;
        H(a1, 0x8) = H(a1, 0x8) + H(a1, 0x0);
        H(a1, 0xA) = H(a1, 0xA) + H(a1, 0x2);
        H(a1, 0xC) = H(a1, 0xC) + H(a1, 0x4);
        if (D_800942EC < SH(a1, 0xA)) {
            D_8019A856 = 1;
            D_8019A860 = H(a1, 0x8);
            D_8019A857 = 1;
            D_8019A862 = H(a1, 0xC);
            D_8019A864 = H(a1, 0x8);
            D_8019A866 = H(a1, 0xA) - 0xA0;
            D_8019A868 = H(a1, 0xC);
            func_8019721C(a1, 0xC8);
            return 1;
        }
        if (func_8019721C(a1, 0x50)) {
            D_8019A857 = 1;
            D_8019A864 = H(a1, 0x8);
            D_8019A866 = H(a1, 0xA);
            D_8019A868 = H(a1, 0xC);
            a1[0xE] = 1;
        } else if (D_8019A85C < 4) {
            idx = D_8019A85C;
            D_8019A85C = idx + 1;
            e = (unsigned short *)((char *)&D_8019A85C - 0x28 + idx * 8);
            e[0] = H(a1, 0x8);
            e[1] = H(a1, 0xA);
            e[2] = H(a1, 0xC);
        }
        return 0;
    }
    if (mode != 2)
        return 0;

    v.x = H(a1, 0x8);
    v.y = H(a1, 0xA);
    a3 = H(a1, 0xC);
    D_800F3368.a68 = 0x20;
    D_800F336A = 2;
    D_800F3376 = 0x20;
    D_800F3378 = 0x20;
    D_800F3376 = 0x20;
    D_800F3378 = 0x20;
    a2 = D_800E11FA << 1;
    v.z = a3;
    D_800F336C = 3;
    D_800F336E = 1;
    D_800F3372 = 0;
    D_800F3374 = 0;
    D_800F3370 = *(unsigned short *)((char *)D_800E2850 + a2);
    t = func_80077AA4(0, D_800E120A + 4, a2, a3);
    func_800CEE20(&v, 0, 0xA00, 0xA00, D_800F336A * 6 + 0x80,
                  (unsigned short)t, 1, 0x80, D_8019A61C);
    v.y = D_800942EC;
    func_800D004C(&v, 0x60, 0x60, 6, D_8019A3C0, 0x1000, 0x1000,
                  D_8019A61C + 4, D_8019A61C + 8, 0x80, 1);
    return 0;
}
