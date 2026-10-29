/* room_m0350i (PE.IMG room m0350i, VRAM 0x8018EFE8)
 * func_80192E4C — blob offset 0x3E64, 0x7C0 bytes. Profile
 * room_m0350i_narrow_any_dest (-O2 -G0 + maspsx K3/K3b/K3c narrowing);
 * LINK_EXACT at the room VMA (2026-10-09, written from the retail asm).
 * Was the m0350i story 0x2D6/0x2DA (Day 6) CPU_BOUNDARY stop.
 * inline GTE/asm: uses inline PSY-Q GTE (COP2) asm macros. */

/* Effect callback (func_800CE560 pool task). mode 1 ends after 16 ticks;
 * mode 2 draws a sprite trail from the actor down to the floor
 * (func_800CEE20), a top sprite, two func_800D0E88 effects and a
 * func_800D0728 model, then runs a 4-edge nclip test of the player's
 * screen position against the projected quad and, if inside, flags the
 * actor (0x4000 in the player state, bit 31 in the owner) once.
 * Matching notes (gcc 2.7.2):
 *  - `scale` reads the rcossin word table as int >> 21 (maspsx K3c narrows
 *    `lw`/`sra $s2,$v0,21` to retail's `lh`/`sra 5` into another register).
 *  - `(short)scale` in the func_800D0E88 loop is the loop-invariant copy
 *    retail hoists into $s4; the literal 1 stays in the loop.
 *  - The second sprite stores sprite param f8 first so cse relates f2/f4/f6
 *    to the D_800F3368+8 base ($s0). The always-false check (f6 was just set
 *    to 1) ends cse1's block there; jump later deletes it, so cse2 rewrites
 *    the f2 read as -6($s0) without refolding it, and sched sees no alias
 *    with the outgoing-arg stores (retail lh after `sw 20($sp)`). */
#define gte_SetRotMatrix(r0) __asm__ volatile(                       \
    "lw $12, 0(%0);" "lw $13, 4(%0);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8(%0);" "lw $13, 12(%0);" "lw $14, 16(%0);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20(%0);" "lw $13, 24(%0);" "ctc2 $12, $5;"                 \
    "lw $14, 28(%0);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldv0(r0) __asm__ volatile("lwc2 $0, 0(%0);" "lwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtv0tr() __asm__ volatile("nop;" "nop;" ".word 0x4A480012")
#define gte_stsv(r0) __asm__ volatile(                                  \
    "mfc2 $12, $9;" "mfc2 $13, $10;" "mfc2 $14, $11;"                    \
    "sh $12, 0(%0);" "sh $13, 2(%0);" "sh $14, 4(%0)"                    \
    : : "r"(r0) : "$12", "$13", "$14", "memory")
#define gte_ldsxy3(r0, r1, r2) __asm__ volatile(                        \
    "mtc2 %0, $12;" "mtc2 %2, $14;" "mtc2 %1, $13"                       \
    : : "r"(r0), "r"(r1), "r"(r2))
#define gte_nclip() __asm__ volatile("nop;" "nop;" ".word 0x4B400006")
#define gte_stopz(r0) __asm__ volatile("swc2 $24, 0(%0)" : : "r"(r0) : "memory")

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { short f0; short f2; unsigned short f4; short f6; unsigned short f8; } SPRP;
typedef struct { SVECTOR *pos; short done; } Particle;

extern int D_800E27EC;
extern short D_800966EC[];
extern unsigned char *D_800F32D0;
extern unsigned char *D_8009D254;
extern unsigned short D_800E11EA, D_800E11FA, D_800E120A;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern short D_800942EC;
extern short D_800F336A;
extern SPRP D_800F3368;
extern unsigned short D_800F336C;
extern int D_800F3428;
extern CVECTOR D_8019A434[];
extern SVECTOR D_8019A43C, D_8019A440;
extern CVECTOR D_8019A3C8;
extern SVECTOR D_8019A444[];
extern unsigned char D_8019A79C;
extern void func_80079754(SVECTOR *, MATRIX *);
extern int func_80077AA4(int, int);
extern void func_800CEE20();
extern void func_800D0E88();
extern void func_800D0728();
extern int func_8005186C(int);
extern int func_80077DC4(int);
extern int func_80077CF4(int);

int func_80192E4C(int event, Particle *p)
{
    SVECTOR pos;
    SVECTOR d;
    SVECTOR rot;
    SVECTOR rot2;
    MATRIX m;
    SVECTOR pts[4];
    MATRIX m2;
    int opz;
    int t, n1, i, scale, w, ta, tb2, dist, c, s, k, cur, prev, me, y;
    char *tb;
    unsigned char *o;

    if (event == 1) {
        if (D_800E27EC >= 16) return 1;
    } else if (event == 2) {
        t = D_800E27EC;
        n1 = t - 1;
        i = 8;
        if (n1 < 9) i = n1;
        rot.vx = -D_800966EC[((i * 128) & 0xFFF) * 2] * 608 / 4096 - 0x500;
        tb = (char *)D_800966EC;
        rot.vy = *(short *)(*(unsigned char **)(D_800F32D0 + 8) + 0x3A);
        rot.vz = 0x400;
        func_80079754(&rot, &m);
        m.t[0] = 0; m.t[1] = 0; m.t[2] = 0;
        *(int *)&d = 0;
        d.vz = 0x70;
        gte_SetRotMatrix(&m);
        gte_ldv0(&d);
        gte_rtv0tr();
        gte_stsv(&d);
        rot.vx += 0x400;
        rot.pad = 1;
        pos = *p->pos;
        if (n1 >= 9) scale = *(int *)(tb + (((t - 9) << 9) & 0x3E00)) >> 21;
        else scale = 0x80;
        ta = D_800E2850[D_800E11EA];
        D_800F3368.f4 = 3;
        D_800F3368.f6 = 0;
        D_800F3368.f8 = ta;
        for (i = n1; pos.vy < D_800942EC; i++) {
            y = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428) y += 4;
            func_800CEE20(&pos, &rot, 0x800, 0x1800, D_800F336A * (i & 7) + 0x40,
                          func_80077AA4(0, y) & 0xFFFF, 1, scale, &D_8019A434[D_800E27EC & 1]);
            pos.vx += d.vx;
            pos.vy += d.vy;
            pos.vz += d.vz;
        }
        {
        y = D_800E120A + 4;
        rot2.vx = 0x400;
        rot2.vy = 0;
        rot2.vz = D_800E27EC << 7;
        rot2.pad = 1;
        tb2 = D_800E2850[D_800E11FA];
        D_800F3368.f8 = tb2;
        D_800F3368.f4 = 3;
        D_800F3368.f6 = 1;
        if (D_800F3368.f6 == 0 && D_800F3368.f4 == 3) return 0;
        func_800CEE20(&pos, &rot2, 0x2000, 0x2000, D_800F3368.f2 * 17 + 0x84,
                      func_80077AA4(0, y) & 0xFFFF, 1, scale, &D_8019A434[D_800E27EC & 1]);
        }
        dist = func_8005186C((pos.vx - p->pos->vx) * (pos.vx - p->pos->vx) +
                             (pos.vz - p->pos->vz) * (pos.vz - p->pos->vz));
        rot.vz = 0;
        rot.vx = 0;
        rot.vy -= 20;
        for (i = 0; i < 2; i++) {
            func_800D0E88(&pos, &rot, dist, 0x200, &D_8019A43C, &D_8019A3C8, &D_8019A3C8, (short)scale, 1);
            rot.vy += 40;
        }
        rot2.vx = 0;
        rot2.vy = 0;
        rot2.pad = 0;
        rot2.vz = D_800E27EC << 7;
        w = D_800966EC[((n1 * 64) & 0xFFF) * 2];
        func_800D0728(p->pos, 0x100, 0x1C0, 0x10, &rot2, w + 0x1000, w + 0x1000,
                      &D_8019A3C8, &D_8019A440,
                      ((int *)D_800966EC)[((D_800E27EC - 1) << 6) & 0xFC0] >> 21, 1);
        if (p->done) return 0;
        if (D_8009D254[0xE] < 4) return 0;
        D_8019A444[2].vz = D_8019A444[3].vz = -dist;
        c = func_80077DC4(*(short *)(*(unsigned char **)(D_800F32D0 + 8) + 0x3A));
        s = func_80077CF4(*(short *)(*(unsigned char **)(D_800F32D0 + 8) + 0x3A));
        m2.m[0][2] = s;
        m2.m[2][0] = -s;
        m2.m[1][1] = 0x1000;
        m2.m[0][0] = c;
        m2.m[2][2] = c;
        m2.t[0] = m2.t[1] = m2.t[2] = 0;
        m2.m[0][1] = m2.m[1][0] = m2.m[1][2] = m2.m[2][1] = 0;
        m2.t[0] = pos.vx; m2.t[1] = pos.vy; m2.t[2] = pos.vz;
        gte_SetRotMatrix(&m2);
        for (k = 0; k < 4; k++) {
            gte_ldv0(&D_8019A444[k]);
            gte_rtv0tr();
            gte_stsv(&pts[k]);
        }
        prev = (pts[3].vz << 16) | (unsigned short)pts[3].vx;
        me = (*(int *)(D_8009D254 + 0x204) << 16) | (*(int *)(D_8009D254 + 0x1FC) & 0xFFFF);
        for (k = 0; k < 4; k++, prev = cur) {
            cur = (pts[k].vz << 16) | (unsigned short)pts[k].vx;
            gte_ldsxy3(me, cur, prev);
            gte_nclip();
            gte_stopz(&opz);
            if (opz < 0) break;
        }
        if (k < 4) return 0;
        o = D_800F32D0;
        *(int *)(*(unsigned char **)D_8009D254 + 0x4C) |= 0x4000;
        o = **(unsigned char ***)(o + 8);
        if (o) *(unsigned int *)o |= 0x80000000;
        p->done = 1;
        D_8019A79C = 1;
    }
    return 0;
}
