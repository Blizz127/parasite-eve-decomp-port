/*
 * func_800136C0 - field-VM handler: walk the camera toward a target point.
 *
 * VRAM 0x800136C0 / file 0x3EC0 / size 0x2C8 (178 words). asm/disc1/3B00.s.
 * era -O2 -G8 with MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0,D_8009D254.
 * Verified LINK_EXACT, 0 word mismatches, zero pad.
 *
 * Levers that were load-bearing here:
 *  - The camera x/y pair is ONE pair of C variables reused three times
 *    (current position -> delta>>16 -> the 0x6A/0x70 velocity pair).  Giving
 *    each role its own local drops those pseudos' reference count and cc1
 *    hands $s2/$s3 to the target pair instead (68 -> 46 words).
 *  - `if (o != D_8009D254) { plain } else { call }` -- the `==` spelling puts
 *    the call block in the fall-through and costs 84 words.
 *  - `t = ang;` must sit OUTSIDE the `if (rate != 0)` block so the copy lands
 *    in the branch delay slot.
 *  - PHYSICAL BLOCK ORDER WITH AN `slt` CONDITION IS INVERTED: cc1 lays the
 *    two arms of `if (<slt-derived>) A else B` out as B-then-A, so the arm
 *    that must come FIRST in the object is the one written SECOND.  Writing
 *    the return-1 arm first and falling through to the return-0 tail is what
 *    closes the last 17 words.
 */
extern unsigned char *D_8009D300;
extern int D_8009CE00;
extern unsigned char *D_8009D254;
extern unsigned char *D_8009D2F0;

extern int func_8003708C(int a0, int a1);
extern int func_80079FB4(int a0, int a1);
extern int func_80077CF4(int a0);
extern int func_80077DC4(int a0);

int func_800136C0(int **a0) {
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
    if ((*(unsigned short *)(st + 8) & 0x20) == 0) {
        tx = *a0[0];
        ty = *a0[1];
        if (cx == tx && cy == ty) {
            return 1;
        }
        rate = *a0[2];
        {
            unsigned char *s2 = D_8009D300;
            *(int *)(s2 + 0x14) = tx;
            *(int *)(s2 + 0x18) = ty;
            *(int *)(s2 + 0x1C) = rate;
            *(unsigned short *)(s2 + 8) = *(unsigned short *)(s2 + 8) | 0x20;
        }
    } else {
        tx = *(int *)(st + 0x14);
        ty = *(int *)(st + 0x18);
        rate = *(int *)(st + 0x1C);
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
            st = D_8009D300;
            *(int *)(o5 + 0x28) = tx;
            *(int *)(o5 + 0x30) = ty;
            *(int *)(o5 + 0x68) = 0;
            *(int *)(o5 + 0x70) = 0;
            *(unsigned short *)(st + 8) = *(unsigned short *)(st + 8) & 0xFFDF;
            return 1;
        }
        D_8009CE00 = D_8009CE00 - 0x14;
        *(int *)(D_8009D300 + 0x10) = 1;
    }
    return 0;
}
