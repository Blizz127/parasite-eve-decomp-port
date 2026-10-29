/* NON_MATCHING: functionally equivalent to retail room_m0089i 0x4218..0x4630
 * (func_80193200).  Written from the retail asm, not byte-matched; native port
 * differential-tested by tools/analysis/e21_diff.py.
 * Particle callback of the m0089i battle effect (registered by
 * func_80193618 through func_800CE560): mode 1 moves the particle (kind 0:
 * spiral out from the origin a2 and fall, kind 1: drift), ending it at frame
 * 64 / 6 of the counter D_800E27EC; mode 2 loads the camera (D_800BCFA4)
 * into the GTE and draws it (kind 0: func_800CEE20 sprite, kind 1:
 * func_800D2370 flash at a2). */
typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern unsigned char D_8018F1D8[4];
extern unsigned char *D_800BCFA4;
extern int D_800E27EC;
extern int D_800F3428;
extern unsigned short D_800E1204[];
extern short D_800F336A;
extern unsigned short D_800F336C;
extern int func_80077DC4();
extern int func_80077CF4();
extern int func_80077AA4();
extern void func_800CEE20();
extern void func_800D2370();

int func_80193200(int mode, short *q, short *a2)
{
    struct { SVECTOR pos; SVECTOR ang; unsigned char col[4]; } f; /* retail sp+56/64/72 */
    unsigned char *cm;
    int i;
    int t;
    int s;
    int r;
    int c;
    int k;

    for (i = 0; i < 4; i++)
        f.col[i] = D_8018F1D8[i];
    if (mode == 1) {
        if (q[4] == 0) {
            q[0] = a2[0];
            q[1] = a2[1];
            q[2] = a2[2];
            s = (q[6] * D_800E27EC) / 64;
            q[0] = q[0] + (func_80077DC4(q[5]) * s) / 4096;
            q[2] = q[2] + (func_80077CF4(q[5]) * s) / 4096;
            q[1] = q[1] - (func_80077CF4(D_800E27EC << 5) * 120) / 4096;
            q[5] = q[5] + 12;
            return D_800E27EC < 64 ? 0 : 1;
        }
        if (q[4] == 1) {
            q[0] = q[0] + 32;
            return D_800E27EC < 6 ? 0 : 1;
        }
        return 0;
    }
    if (mode != 2)
        return 0;
    cm = D_800BCFA4;
    asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4\n\t" "lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"(cm) : "$12", "$13", "$14");
    if (q[4] == 0) {
        t = D_800E27EC;
        f.ang.vx = 0;
        f.ang.vy = 0;
        f.ang.pad = 0;
        r = 128 - t * 2;
        f.ang.vz = q[6] + q[5] + t * 12;
        s = func_80077CF4(t << 4) + 2048;
        c = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0)
            c += 4;
        k = func_80077AA4(32, c) & 0xFFFF;
        func_800CEE20(q, &f.ang, s, s, D_800F336A * (D_800E27EC / 16) + 36, k, 1, r, 0);
        return 0;
    }
    if (q[4] == 1) {
        f.pos.vx = a2[0];
        f.pos.vy = a2[1];
        f.pos.vz = a2[2];
        r = func_80077DC4((D_800E27EC << 10) / 6) / 32;
        f.col[0] = 0x80;
        f.col[1] = 0x80;
        f.col[2] = 0x80;
        f.col[3] = 0;
        c = D_800E1204[D_800F336C];
        s = (D_800E27EC & 1) * 16 + 128;
        if (D_800F336C == 4 && D_800F3428 != 0)
            c += 4;
        k = func_80077AA4(16, c) & 0xFFFF;
        func_800D2370(&f.pos, q, 960, 140, 0, s, 255, 15, k, f.col, 0, (short)r, 1);
        return 0;
    }
    return 0;
}
