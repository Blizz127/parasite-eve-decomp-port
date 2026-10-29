/* room_m0269i — func_80191CFC, blob offset 0x2D14, 0x490 bytes. Profile era_o2_g0_expand_div
 * (MASPSX_EXPAND_DIV=1: the `% a2[1]` div keeps its break 7/break 6 guards).
 * 3-mode effect controller. Mode 0 arms a 0x14/0x37 callback on func_801913B4.
 * Mode 1 aims from the owner, spawns a random particle on odd ticks < 0x3C,
 * plays sounds 0x599/0x5B5 at tick 1 and 0x5B6/0x5B7 at tick 0x10, and spawns a
 * spread particle (a2[0], a2[1]) for ticks 0x10..0x3C; returns 2 once tick >= 2.
 * Mode 2 seeds the D_800F3368 block.
 * Levers: rand value and halfword read into block locals before the add (no
 * reassociation of -0x80/-0x200); the sound guard re-test reads through
 * `*(volatile int *)pb` (retail reloads after the test) while the outer test and
 * call arguments stay plain, so `li s1,0x7F` still fills the jal delay slot. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern SVECTOR D_8018F050;
extern unsigned char *D_800E2368;
extern void *D_800F32D0;
extern void *D_800F33E0;
extern int D_800E27EC;
extern int D_800B0E64;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern int func_801913B4();
extern int func_800D3FD8();
extern int func_800CE560();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern unsigned short *func_800CE610();
extern int func_80071A54();
extern void func_800CFB7C(unsigned short *, short, unsigned short *);
extern void func_8006DF50();

#define P(o, x) (*(void **)((char *)(o) + (x)))

int func_80191CFC(int mode, int a1, int *a2)
{
    SVECTOR sv;
    unsigned short p[4];
    unsigned short q[4];
    unsigned short r[4];
    unsigned short *e;
    void *obj;
    void *inner;
    unsigned char *st;

    sv = D_8018F050;
    switch (mode) {
    case 0:
        if (D_800E2368[13] != 0) {
            obj = P(D_800F32D0, 8);
            if (obj != 0) {
                inner = *(void **)obj;
                if (inner != 0) {
                    st = P(inner, 0x18);
                    if (*st == 1)
                        *st = 2;
                }
            }
        }
        return func_800CE560(P(D_800F33E0, 8), 0x14, 0x37, func_801913B4);
    case 1:
        func_800CE8F0(P(D_800F32D0, 8), 0, &sv, p);
        p[1] -= 0x50;
        func_800CE9D4(P(D_800F32D0, 8), 0, q);
        q[0] = q[2] = 0;
        if ((D_800E27EC & 1) && D_800E27EC < 0x3C) {
            e = func_800CE610(P(D_800F33E0, 8));
            if (e != 0) {
                e[0] = p[0];
                e[1] = p[1];
                e[2] = p[2];
                r[0] = q[0];
                r[1] = q[1];
                r[2] = q[2];
                {
                    int rr = func_80071A54();
                    int y = r[0];

                    r[0] = (y - 0x80) + (rr & 0xFF);
                }
                {
                    int rr = func_80071A54();
                    int y = r[1];

                    r[1] = (y - 0x200) + (rr & 0x3FF);
                }
                func_800CFB7C(r, (func_80071A54() & 0xF) | 0x10, e + 4);
                e[8] = 2;
                e[9] = 0;
            }
        }
        if (D_800E27EC == 1) {
            int *pb = &D_800B0E64;

            if (*pb != 0) {
                func_8006DF50(*pb, 0x599, func_800D3FD8(), 0x80, 0x7F);
                if (*(volatile int *)pb != 0)
                    func_8006DF50(*pb, 0x5B5, 0x80, 0x80, 0x7F);
            }
        }
        if (D_800E27EC == 0x10) {
            int *pb = &D_800B0E64;

            if (*pb != 0) {
                func_8006DF50(*pb, 0x5B6, func_800D3FD8(), 0x80, 0x7F);
                if (*(volatile int *)pb != 0)
                    func_8006DF50(*pb, 0x5B7, 0x80, 0x80, 0x7F);
            }
        }
        if ((unsigned int)(D_800E27EC - 0x10) < 0x2D) {
            e = func_800CE610(P(D_800F33E0, 8));
            if (e != 0) {
                e[0] = p[0];
                e[1] = p[1];
                e[2] = p[2];
                q[1] += func_80071A54() % a2[1] - a2[1] / 2;
                func_800CFB7C(q, a2[0] + (func_80071A54() & 0xF), e + 4);
                e[8] = 0;
                e[9] = 0;
            }
        }
        if (D_800E27EC >= 2)
            return 2;
        break;
    case 2:
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 4;
        break;
    }
    return 0;
}
