/* room_m0005i (PE.IMG room m0005i chunk 2, VRAM 0x8018EFE8)
 * func_8018F330 — blob offset 0x348, 0x2E4 bytes. Profile era_o2_g0 (-O2 -G0, default).
 * Room particle callback (cmd 0 init, cmd 1 step counter ending after 0x20 frames,
 * cmd 2 draw through the D_800F3368 draw state).
 * Levers (lane r005):
 *   - D_800F3368 as one struct, with a70 written before a6C/a6E;
 *   - D_800B0E64 through a block-local pointer: plain for the first call, the second
 *     call's argument read through a volatile cast (forces retail's reload);
 *   - step as `short t = a1[5] + 1; a1[5] = t; if (t >= 0x20) return 1; break;` onto the
 *     shared return 0;
 *   - the q[1] store before q[3] = 0.
 * inline GTE/asm: Psy-Q inline_c.h-shaped COP2 macros (gte_SetRotMatrix/gte_SetTransMatrix) —
 * count separately from plain C. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { short x, y, z, flag, vx, vy, vz; } Particle;
typedef struct { short vx, vy, vz, pad; } GteShortVector;
extern GteShortVector D_8018EFF4;
extern void *D_800F33E0;
extern void *D_800F32D0;
extern int D_800E27EC;
extern int D_800B0E64;
extern unsigned char *D_800E2368;
extern unsigned short D_800E11E8;
extern unsigned short D_800E2850[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;
extern char D_80190B84[], D_80190B8C[];
extern int func_8018F018();
extern int func_800CE560();
extern int func_800D3FD8();
extern void func_8006DF50();
extern void func_800CE870();
extern void func_800CE8F0();
extern void *func_800CE610();
extern int func_80071A54();

#define P(o, x) (*(void **)((char *)(o) + (x)))

int func_8018F330(int cmd, short *a1)
{
    GteShortVector v = D_8018EFF4;
    GteShortVector *e = &v;
    GteShortVector buf;
    short *q;
    unsigned short idx;

    switch (cmd) {
    case 0:
        a1[4] = 0;
        a1[5] = 0;
        return func_800CE560(P(D_800F33E0, 8), 0x10, 0x18, func_8018F018);
    case 1:
        if (D_800E27EC == 7) {
            int *pb = &D_800B0E64;
            if (*pb != 0) {
                func_8006DF50(*pb, 0x5AB, func_800D3FD8(), 0x80, 0x7F);
                if (*pb != 0)
                    func_8006DF50(*(volatile int *)pb, 0x5AC, 0x80, 0x80, 0x7F);
            }
        }
        if (D_800E27EC < 0x19) {
            func_800CE870(P(D_800F32D0, 8), 0, &buf);
            q = func_800CE610(P(D_800F33E0, 8));
            if (q != 0) {
                int t;
                q[4] = func_80071A54();
                t = func_80071A54() & 0x1FF;
                q[1] = buf.vy + t - 0x100;
                q[3] = 0;
                q[5] = (func_80071A54() & 0xFF) + 0x100;
                q[6] = 0;
                q[7] = 0;
            }
        }
        if (a1[4] == 1) {
            short t = a1[5] + 1;
            a1[5] = t;
            if (t >= 0x20)
                return 1;
            break;
        } else if (*(short *)(D_800E2368 + 0x12) != 0) {
            a1[4] = 1;
        }
        return 0;
    case 2:
        func_800CE870(P(D_800F32D0, 8), 0, D_80190B84);
        func_800CE8F0(P(D_800F32D0, 8), 0x14, e, D_80190B8C);
        D_800F3368.a68 = 0x10;
        D_800F3368.a6A = 1;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a70 = D_800E2850[D_800E11E8];
        D_800F3368.a6C = 2;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 8;
        break;
    default:
        return 0;
    }
    return 0;
}
