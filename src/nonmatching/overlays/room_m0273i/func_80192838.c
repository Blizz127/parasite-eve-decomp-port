/* NON_MATCHING: functionally equivalent to retail room_m0273i 0x3850..0x3C18
 * (func_80192838).  Written from the retail asm, not byte-matched; native port
 * differential-tested by tools/analysis/e21_diff.py.
 * Slime (m0273i) grab-attack driver: mirrors the hold pose into the player
 * (D_8009D254 +0x1E8.., position +0x28/+0x30), starts the grab once
 * (func_80192C00) while the player lives, then steps the owner's animation
 * events: 9 the grab (loop, release flag, drop velocity at 0x22, re-grab at
 * 0x32, hit at 3, done at the last frame), 8 / 0x10 the wind-up and return;
 * any other animation hands back to func_80192D8C. */
extern int *D_8009D254;
extern short D_800942EC;
extern void func_80192C00(unsigned char *actor, unsigned char *st);
extern void func_80192D8C(unsigned char *e);
extern void func_80020CE4(void);

void func_80192838(unsigned char *e)
{
    unsigned char *a = *(unsigned char **)(e + 8);
    unsigned char *s3 = e + 0xC;
    int *P = D_8009D254;
    int k, f, pf, v, *o;

    for (k = 0; k < 8; k++) P[0x1E8 / 4 + k] = *(int *)(e + 0x1C + k * 4);
    P = D_8009D254;
    f = *(short *)(a + 0x16);
    pf = *(short *)(a + 0x1A);
    P[0x28 / 4] = *(int *)(e + 0x40);
    P[0x30 / 4] = *(int *)(e + 0x44);
    if (e[0x4A] == 0) {
        v = P[0];
        if (v == 0 || *(short *)(v + 0xC) <= 0) {
            func_80192C00(a, s3);
            *(short *)(e + 0x14) = 0;
            e[0x4A] = 1;
        }
    }
    switch (a[0xE]) {
    case 9:
        if (pf < 0x18 && f >= 0x18) {
            if (*(short *)(s3 + 8) > 0) {
                *(int *)(a + 0x14) = 0;
                *(short *)(s3 + 8) = *(short *)(s3 + 8) - 1;
            }
        } else if (pf <= 0 && f > 0) {
            o = *(int **)a;
            ((unsigned char *)o[6])[1] = s3[0x40];
        } else if (pf < 0x22 && f >= 0x22) {
            *(int *)(s3 + 0x30) = ((D_800942EC << 16) - *(int *)(a + 0x2C)) / 16;
            *(short *)(s3 + 0x3C) = 0x10;
        } else if (pf < 0x32 && f >= 0x32) {
            if (s3[0x3E] == 0) {
                func_80020CE4();
                func_80192C00(a, s3);
            }
        } else if (pf < 3 && f >= 3) {
            v = D_8009D254[0];
            if (v && *(short *)(v + 0xC) > 0) {
                *(int *)(v + 0x4C) |= 0x4000;
                o = *(int **)a;
                if (o)
                    *o |= 0x80000000;
            }
        } else if (f >= a[0xF] - 1) {
            o = *(int **)a;
            if (o)
                *(unsigned char *)o[6] = 4;
            func_80192D8C(e);
        }
        if (*(short *)(s3 + 0x3C) != 0) {
            *(int *)(a + 0x2C) = *(int *)(a + 0x2C) + *(int *)(s3 + 0x30);
            *(short *)(s3 + 0x3C) = *(short *)(s3 + 0x3C) - 1;
        }
        return;
    case 8:
        if (f <= 0) {
            o = *(int **)a;
            if (o)
                *(unsigned char *)o[6] = 2;
        }
        return;
    case 0x10:
        if (f >= 0x21 && pf < 0x21) {
            *(int *)(a + 0x14) = 0x3D0000;
            return;
        }
        if (f >= a[0xF] - 1)
            **(int **)(s3 + 4) = 2;
        return;
    default:
        func_80192D8C(e);
        return;
    }
}
