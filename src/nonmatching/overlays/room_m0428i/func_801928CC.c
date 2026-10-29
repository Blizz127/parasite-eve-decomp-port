/* room_m0428i — func_801928CC, blob offset 0x38E4, 0x2C0 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Particle callback. cmd 1: integrates the position shorts (+4 += +6, +8 += +0xC,
 * +0xA += +0xE), grows the velocities (+0xC += 0x30, +0xE += 0x20), and while the
 * +0x12 flag is set tests the distance (func_8005186C) from the *D_8009D254
 * actor (+0x1FC/+0x204) to the point record *a1 against (short)+8 / 4; inside it
 * sets bit 0x4000 in the actor flags and bit 31 of **(D_800F32D0+8), clears the
 * flag. Then +0x10 -= 10 and, below 10, the D_801941C8 counter is decremented
 * and 1 is returned (callback done). cmd 2: builds a yaw matrix from sin/cos of
 * (short)+4 (func_80077DC4/CF4, func_80071A44 memset, func_80078CC4 scale by the
 * {x,z,x} vector from +8/+0xA), places it at *a1 and draws through
 * func_800C6EF8/6FA0/71E4/6F4C on D_801941C4 with size min((short)+0x10, 0x80).
 * Retail reads sp+0x4C after func_80071A44 (memset of sp+0x40 .. 0x10 bytes). */
#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

extern unsigned char *D_8009D254;
extern unsigned char *D_800F32D0;
extern int D_801941C4;
extern unsigned short D_801941C8;

extern int func_8005186C();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80071A44();
extern int func_80078CC4();
extern int func_800C6EF8();
extern int func_800C6F4C();
extern int func_800C6FA0();
extern int func_800C71E4();

int func_801928CC(int mode, unsigned char *a1)
{
    if (mode == 1) {
        unsigned char *rec;
        unsigned char *p;
        int dx, dz, d, lim;
        short s;

        H(a1, 4) = H(a1, 4) + H(a1, 6);
        H(a1, 8) = H(a1, 8) + H(a1, 0xC);
        H(a1, 0xA) = H(a1, 0xA) + H(a1, 0xE);
        H(a1, 0xC) = H(a1, 0xC) + 0x30;
        H(a1, 0xE) = H(a1, 0xE) + 0x20;
        if (SH(a1, 0x12) != 0) {
            p = *(unsigned char **)a1;
            rec = D_8009D254;
            dx = W(rec, 0x1FC) - W(p, 0);
            dz = W(rec, 0x204) - W(p, 8);
            d = func_8005186C(dx * dx + dz * dz);
            lim = SH(a1, 8);
            if (lim < 0)
                lim += 3;
            if (d < (lim >> 2)) {
                p = *(unsigned char **)D_8009D254;
                W(p, 0x4C) = W(p, 0x4C) | 0x4000;
                p = *(unsigned char **)(*(unsigned char **)(D_800F32D0 + 8));
                W(p, 0) = W(p, 0) | 0x80000000;
                SH(a1, 0x12) = 0;
            }
        }
        s = (short)(H(a1, 0x10) - 10);
        H(a1, 0x10) = s;
        if (s < 10) {
            D_801941C8 = D_801941C8 - 1;
            return 1;
        }
        return 0;
    }
    if (mode == 2) {
        short m[16];
        int sv[4];
        int sw[4];
        short c, sn;
        unsigned char *p;
        int sz;

        c = func_80077DC4(SH(a1, 4));
        sn = func_80077CF4(SH(a1, 4));
        m[2] = sn;
        m[6] = -sn;
        m[0] = c;
        m[8] = c;
        m[0xB] = 0;
        m[0xA] = 0;
        m[9] = 0;
        m[7] = 0;
        m[5] = 0;
        m[3] = 0;
        m[1] = 0;
        m[4] = 0x1000;
        func_80071A44(sw, 0, 0x10);
        sw[0] = SH(a1, 8);
        sw[1] = SH(a1, 0xA);
        sw[2] = SH(a1, 8);
        sv[0] = sw[0];
        sv[1] = sw[1];
        sv[2] = sw[2];
        sv[3] = sw[3];
        func_80078CC4(m, sv);
        p = *(unsigned char **)a1;
        W(m, 0x14) = W(p, 0);
        p = *(unsigned char **)a1;
        W(m, 0x18) = W(p, 4);
        p = *(unsigned char **)a1;
        W(m, 0x1C) = W(p, 8);
        func_800C6EF8(D_801941C4);
        sz = H(a1, 0x10);
        if (SH(a1, 0x10) >= 0x81)
            sz = 0x80;
        func_800C6FA0(D_801941C4, sz);
        func_800C71E4(D_801941C4, m);
        func_800C6F4C(D_801941C4);
        return 0;
    }
    return 0;
}
