/* room_m0413i (PE.IMG room m0413i chunk 2, VRAM 0x8018EFE8)
 * func_80192D74 — blob offset 0x3D8C, 0x360 bytes. Profile era_o2_g0_expand_div (-O2 -G0, MASPSX_EXPAND_DIV=1)
 * (load-bearing: the rand() % a2[1] div has break 7/6 guards).
 * Room effect controller: cmd 0 kicks the owner's state byte (1 -> 2), plays 0x5A8
 * and registers the func_80192500 particle pool (func_800CE560 0x14, 0x37);
 * cmd 1 spawns func_800CE610 puffs at the func_800CE8F0 anchors (D_800E27EC == 1
 * and odd frames < 0x2F), returns 2 once D_800E27EC >= 2; cmd 2 sets the
 * D_800F3368 draw state (0x10/1, palette D_800E2850[D_800E11EC], mode 4).
 * Masked-body twins (same address): room_m0416i, room_m0417i, room_m0437i.
 * Levers (lane r413):
 *   - the rand() result in its own local, and the whole ob[5] value computed
 *     into `int t` BEFORE the ob[8]/ob[9] zero stores (else sched puts the
 *     stores ahead of mfhi);
 *   - the func_800CE9D4 output vector is unsigned short (retail lhu at +0x2A);
 *   - `if (D_800E27EC >= 2) return 2; break;` (shared return 0) gives the
 *     bnez/move-0 + j/li-2 tail.
 * No asm ties or barriers. Hand-written in this repo (lane r413). */
typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned short x, y, z, w; } UV4;

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern GteShortVector D_8018F1D4;
extern GteShortVector D_8018F1DC;
extern void func_80192500();
extern unsigned char *D_800E2368;
extern void *D_800F32D0;
extern void *D_800F33E0;
extern int D_800E27EC;
extern unsigned short D_800E11EC;
extern unsigned short D_800E2850[];
extern int func_800D3FD8();
extern void func_800D3F64();
extern int func_800CE560();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern unsigned short *func_800CE610();
extern int func_80071A54();
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

int func_80192D74(int mode, int a1, int *a2)
{
    GteShortVector v0 = D_8018F1D4;
    GteShortVector v1 = D_8018F1DC;
    GteShortVector pos;
    UV4 rot;
    unsigned short *ob;
    void **pp, *r;
    unsigned char *c;

    switch (mode) {
    case 0:
        if (D_800E2368[0xD] != 0) {
            pp = P(D_800F32D0, 8);
            if (pp != 0) {
                r = *pp;
                if (r != 0) {
                    c = P(r, 0x18);
                    if (*c == 1) *c = 2;
                }
            }
        }
        func_800D3F64(0x5A8, func_800D3FD8());
        return func_800CE560(P(D_800F33E0, 8), 0x14, 0x37, func_80192500);
    case 1:
        func_800CE8F0(P(D_800F32D0, 8), 3, &v0, &pos);
        func_800CE9D4(P(D_800F32D0, 8), 0, &rot);
        if (D_800E27EC == 1) {
            ob = func_800CE610(P(D_800F33E0, 8));
            if (ob) {
                ob[0] = pos.x;
                ob[1] = pos.y;
                ob[2] = pos.z;
                ob[8] = 2;
                ob[9] = 0;
            }
        }
        func_800CE8F0(P(D_800F32D0, 8), 3, &v1, &pos);
        if ((D_800E27EC & 1) && D_800E27EC < 0x2F) {
            ob = func_800CE610(P(D_800F33E0, 8));
            if (ob) {
                ob[0] = pos.x;
                ob[1] = pos.y;
                ob[2] = pos.z;
                ob[6] = 0;
                ob[4] = 0;
                {
                    int r = func_80071A54();
                    int t = rot.y + r % a2[1] - a2[1] / 2;
                    ob[8] = 0;
                    ob[9] = 0;
                    ob[5] = t;
                }
            }
        }
        if (D_800E27EC >= 2) return 2;
        break;
    case 2:
        D_800F3368.a68 = 0x10;
        D_800F3368.a6A = 1;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a70 = D_800E2850[D_800E11EC];
        D_800F3368.a6C = 4;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 4;
        break;
    }
    return 0;
}
