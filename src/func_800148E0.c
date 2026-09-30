/*
 * func_800148E0 - field-VM handler: walk the camera by a relative offset.
 *
 * VRAM 0x800148E0 / file 0x50E0 / size 0x2C0 (176 words). asm/disc1/50E0.s.
 * Sibling of func_800136C0 (identical apart from the target-latch block: the
 * operands are relative deltas, the latch fields are 0x14/0x18/0x1C plus a
 * 0x20 clear, and there is no "already there" early return).
 * era -O2 -G8 with MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0,D_8009D254.
 * Verified LINK_EXACT, 0 word mismatches, zero pad.
 *
 * Extra lever beyond func_800136C0's: the VM-state pointer reloaded in the
 * arrived branch must be a FRESH local (`st2`), not a re-assignment of the
 * one the latch block used -- reusing it lengthens that pseudo's live range
 * and swaps $a0/$a1 for the whole tail (13 words).
 */
extern unsigned char *D_8009D300;
extern int D_8009CE00;
extern unsigned char *D_8009D254;
extern unsigned char *D_8009D2F0;

extern int func_8003708C(int a0, int a1);
extern int func_80079FB4(int a0, int a1);
extern int func_80077CF4(int a0);
extern int func_80077DC4(int a0);

int func_800148E0(int **a0) {
    unsigned char *o;
    unsigned char *st;
    int spd;
    int nspd;
    int cx, cy;
    int tx, ty;
    int rate;
    int ang;
    int cd, d, t;
    int sy;

    o = D_8009D2F0;
    cx = *(int *)(o + 0x28);
    cy = *(int *)(o + 0x30);
    if (o != D_8009D254) {
        spd = *(int *)(o + 0x20);
    } else {
        spd = func_8003708C(0x50000, *(int *)(o + 0x20));
    }
    spd = func_8003708C(spd, *(unsigned short *)(D_8009D2F0 + 0x26) << 4);
    st = D_8009D300;
    {
        unsigned short fl = *(unsigned short *)(st + 8);
        if ((fl & 0x20) == 0) {
            rate = *a0[2];
            tx = cx + *a0[0];
            ty = cy + *a0[1];
            *(unsigned short *)(st + 8) = fl | 0x20;
            *(int *)(st + 0x20) = 0;
            *(int *)(st + 0x14) = rate;
            *(int *)(st + 0x1C) = ty;
            *(int *)(st + 0x18) = tx;
        } else {
            rate = *(int *)(st + 0x14);
            tx = *(int *)(st + 0x18);
            ty = *(int *)(st + 0x1C);
        }
    }
    ang = (0x1400 - func_80079FB4(cy - ty, cx - tx)) & 0xFFF;
    t = ang;
    if (rate != 0) {
        cd = *(short *)(D_8009D2F0 + 0x3A);
        if (cd < t) {
            d = t - cd;
            if (d < 0x800) {
                if (rate < d) {
                    ang = cd + rate;
                }
            } else if (rate < d) {
                ang = cd - rate;
                if (ang < 0) {
                    int x = cd + 0x1000;
                    if (x - t < rate) {
                        ang = t;
                    }
                }
            }
        } else {
            d = cd - t;
            if (d < 0x800) {
                if (rate < d) {
                    ang = cd - rate;
                }
            } else if (rate < d) {
                ang = cd + rate;
                if (ang > 0x1000) {
                    int x = t + 0x1000;
                    if (x - cd < rate) {
                        ang = t;
                    }
                }
            }
        }
        ang = ang & 0xFFF;
        *(short *)(D_8009D2F0 + 0x3A) = ang;
    }
    {
        int c1 = func_80077CF4(ang);
        nspd = -spd;
        *(int *)(D_8009D2F0 + 0x68) = func_8003708C(nspd, c1 << 4);
    }
    sy = func_8003708C(nspd, func_80077DC4(ang) << 4);
    cx = (tx - cx) >> 16;
    cy = (ty - cy) >> 16;
    {
        int dd = cx * cx + cy * cy;
        unsigned char *o5 = D_8009D2F0;
        cx = *(short *)(o5 + 0x6A);
        cy = sy >> 16;
        *(int *)(o5 + 0x70) = sy;
        if (cx * cx + cy * cy >= dd) {
            {
            unsigned char *st2 = D_8009D300;
            *(int *)(o5 + 0x28) = tx;
            *(int *)(o5 + 0x30) = ty;
            *(int *)(o5 + 0x68) = 0;
            *(int *)(o5 + 0x70) = 0;
            *(unsigned short *)(st2 + 8) = *(unsigned short *)(st2 + 8) & 0xFFDF;
            return 1;
            }
        }
        D_8009CE00 = D_8009CE00 - 0x14;
        *(int *)(D_8009D300 + 0x10) = 1;
    }
    return 0;
}
