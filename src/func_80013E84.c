/*
 * func_80013E84 - field-VM handler: walk the camera toward a target OBJECT.
 *
 * VRAM 0x80013E84 / file 0x4684 / size 0x3A4 (233 words). asm/disc1/3B00.s.
 * NOTE for the integrator: the asm block carries `alabel func_80014000` at
 * 0x80014000 (mid-body, after the D_8009D2F0 reload).  Nothing in disc1
 * references that symbol -- it is an interior label only, and the `c` span
 * covers the whole 0x3A4.
 *
 * It is func_80013C34's object search head (D_8009D20C list walk, D_8009D254
 * fallback) bolted onto func_800136C0's movement tail, so the C is those two
 * matched leaves spliced together; it linked EXACT on the first attempt.
 * era -O2 -G8 with
 *   MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0,D_8009D254,D_8009D20C
 * Verified LINK_EXACT, 0 word mismatches, zero pad.
 */
extern unsigned char *D_8009D300;
extern int D_8009CE00;
extern unsigned char *D_8009D254;
extern unsigned char *D_8009D20C;
extern unsigned char *D_8009D2F0;

extern int func_8003708C(int a0, int a1);
extern int func_80079FB4(int a0, int a1);
extern int func_80077CF4(int a0);
extern int func_80077DC4(int a0);

int func_80013E84(int **a0) {
    unsigned char *st;
    unsigned char *obj;
    unsigned short fl;
    int id;
    int rate;
    int cx, cy;
    int tx, ty;
    int spd;
    int nspd;
    int ang, t, cd, d;
    int sy;

    st = D_8009D300;
    fl = *(unsigned short *)(st + 8);
    if ((fl & 0x20) == 0) {
        {
            register int idv asm("$2");
            idv = *a0[0];
            __asm__ __volatile__("" : "=r"(idv) : "0"(idv));
            if (idv == 0) {
                register unsigned char *o1 asm("$2");
                o1 = D_8009D254;
                __asm__ __volatile__("" : "=r"(o1) : "0"(o1));
                if (o1 == 0) {
                    return 1;
                }
                obj = o1;
                goto have;
            }
            id = idv;
        }
        for (obj = D_8009D20C; obj != 0; obj = *(unsigned char **)(obj + 4)) {
            if (obj[0xC] == id && obj[0xD] == *a0[1] &&
                (*(int *)(obj + 0x98) & 0x10) == 0) {
                break;
            }
        }
        if (obj == 0) {
            return 1;
        }
    have:
        {
            unsigned char *s2p = D_8009D300;
            *(unsigned short *)(s2p + 8) = *(unsigned short *)(s2p + 8) | 0x20;
            rate = *a0[2];
            *(unsigned char **)(s2p + 0x18) = obj;
            *(int *)(s2p + 0x14) = rate;
            *(int *)(s2p + 0x1C) = *(unsigned short *)(obj + 0x24);
        }
    } else {
        obj = *(unsigned char **)(st + 0x18);
        rate = *(int *)(st + 0x14);
        if ((*(int *)(obj + 0x98) & 0x10) != 0 ||
            *(unsigned short *)(obj + 0x24) != *(int *)(st + 0x1C)) {
            *(unsigned short *)(st + 8) = fl & 0xFFDF;
            return 1;
        }
    }
    {
        unsigned char *o = D_8009D2F0;
        tx = *(int *)(obj + 0x28);
        ty = *(int *)(obj + 0x30);
        cx = *(int *)(o + 0x28);
        cy = *(int *)(o + 0x30);
    }
    if (cx == tx && cy == ty) {
        return 1;
    }
    {
        unsigned char *o2 = D_8009D2F0;
        if (o2 != D_8009D254) {
            spd = *(int *)(o2 + 0x20);
        } else {
            spd = func_8003708C(0x50000, *(int *)(o2 + 0x20));
        }
    }
    spd = func_8003708C(spd, *(unsigned short *)(D_8009D2F0 + 0x26) << 4);
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
            unsigned char *st2 = D_8009D300;
            *(int *)(o5 + 0x28) = tx;
            *(int *)(o5 + 0x30) = ty;
            *(int *)(o5 + 0x68) = 0;
            *(int *)(o5 + 0x70) = 0;
            *(unsigned short *)(st2 + 8) = *(unsigned short *)(st2 + 8) & 0xFFDF;
            return 1;
        }
        D_8009CE00 = D_8009CE00 - 0x14;
        *(int *)(D_8009D300 + 0x10) = 1;
    }
    return 0;
}
