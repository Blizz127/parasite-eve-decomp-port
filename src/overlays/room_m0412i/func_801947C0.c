/* room_m0412i (PE.IMG room m0412i chunk 2, VRAM 0x8018EFE8)
 * func_801947C0 - blob offset 0x57D8, 0x914 bytes. Profile era_o2_g0 (default).
 * Body twin of room_m0141i func_80191330 / room_m0146i func_8019179C (retail
 * words identical apart from relocations: local callee func_80194680,
 * local data D_80195674..D_8019568C).
 * Set up alongside the m0153i copy (Day 4); not yet hit on a route.
 * See room_m0146i func_8019179C for the behaviour and matching notes. */
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
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { short w, dw; unsigned short kind; short f6E; unsigned short clut; short f72, f74, h, v; } FxParam;
typedef struct {
    int x, y, z;
    int floor;
    int speed;
    int f14, f18;
    int decel;
    int vy;
    int grav;
    int snd;
    short ang;
    short state;
    short done;
    short anim;
    short hit;
    short bounce;
    short damp;
} Obj;
extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern unsigned char *D_8009D254;
extern unsigned short D_8009D1CC;
extern unsigned char *D_8009D248;
extern short D_800942EC;
extern int D_800E27EC;
extern short D_800966EC[];
typedef struct { unsigned short a, b, c; } ClutIx;
extern ClutIx D_800E11E8;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern FxParam D_800F3368;
typedef struct { short a, b; } FxPad;
extern FxPad D_800F3372;
extern int D_800F3428;
extern short D_80195674[];
extern int D_80195680, D_80195688, D_8019568C;
extern int func_80194680();
extern int func_800CE560();
extern int func_8006DCE4();
extern int func_80077DC4(int);
extern int func_80077CF4(int);
extern int func_800DFF80();
extern int func_800DFE20();
extern int func_80079FB4(int, int);
extern unsigned char *func_800CE610();
extern void func_800866A4();
extern int func_8006F39C();
extern void func_8006F6D4();
extern int func_80077AA4(int, int);
extern void func_800CEE20();
extern void func_800D004C();

int func_801947C0(int mode)
{
    VECTOR pos;
    SVECTOR v;
    SVECTOR w;
    MATRIX m;
    int res;
    Obj *p;
    unsigned char *a;
    unsigned char *mt;
    short cnt;
    int c8, n16, j, hd;
    unsigned short av, av2;
    int i, n, cur, prev, ang, h, palette, scale, alpha;
    unsigned int k;

    p = *(Obj **)(D_800E2368 + 0x14);
    switch (mode) {
    case 0:
        p->state = 0;
        p->ang = *(short *)(*(unsigned char **)(D_800F32D0 + 8) + 0x3A);
        return func_800CE560(*(void **)(D_800F33E0 + 8), 8, 4, func_80194680);
    case 1:
        a = *(unsigned char **)(D_800F32D0 + 8);
        switch (p->state) {
        case 0:
            c8 = a[0xE];
            n16 = *(unsigned short *)(a + 0x16);
            if (c8 == 8) {
                if (n16 < 8) p->anim = 26;
                else p->anim = 9;
            } else if (c8 == 9) {
                p->anim = 9;
                if (n16 >= 2) {
                    p->state = 1;
                    mt = *(unsigned char **)(a + 0x238);
                    p->snd = func_8006DCE4(0x585, *(int *)(*(unsigned char **)a + 8),
                                           *(short *)(mt + 0x134), *(short *)(mt + 0x138), *(short *)(mt + 0x13C));
                }
            }
            if (p->anim == 9) {
                v.vx = -16;
                v.vy = 0;
                v.vz = -64;
            } else {
                v.vx = 0;
                v.vy = 0;
                v.vz = 0;
            }
            gte_SetRotMatrix(&((MATRIX *)*(unsigned char **)(a + 0x238))[p->anim]);
            gte_ldv0(&v);
            gte_rtv0tr();
            gte_stsv(&v);
            for (i = 0; i < 3; i++) (&p->x)[i] = (&v.vx)[i] << 16;
            return 0;
        case 1:
            cnt = 1;
            v.vx = 0;
            v.vy = 0;
            v.vz = -p->speed >> 12;
            m.t[0] = 0;
            m.t[1] = 0;
            m.t[2] = 0;
            do {
                int c;
                i = func_80077DC4(p->ang);
                c = func_80077CF4(p->ang);
                m.m[0][2] = c;
                m.m[2][0] = -c;
                m.m[1][1] = 0x1000;
                m.m[0][0] = i;
                m.m[2][2] = i;
                m.t[0] = m.t[1] = m.t[2] = 0;
                m.m[0][1] = m.m[1][0] = m.m[1][2] = m.m[2][1] = 0;
                gte_SetRotMatrix(&m);
                gte_ldv0(&v);
                gte_rtv0tr();
                gte_stsv(&w);
                pos.vx = p->x + (w.vx << 12);
                pos.vz = p->z + (w.vz << 12);
                cur = ((unsigned int)pos.vx >> 16) | (pos.vz & 0xFFFF0000);
                n = D_8009D1CC;
                prev = *(unsigned short *)(D_8009D248 + n * 8 - 6) | (*(int *)(D_8009D248 + n * 8 - 4) & 0xFFFF0000);
                for (i = 0; i < D_8009D1CC; i++, prev = h) {
                    h = *(unsigned short *)(D_8009D248 + i * 8 + 2) | (*(int *)(D_8009D248 + i * 8 + 4) & 0xFFFF0000);
                    gte_ldsxy3(cur, h, prev);
                    gte_nclip();
                    gte_stopz(&res);
                    if (res < 0) break;
                }
                if (i >= D_8009D1CC) {
                    p->hit = 1;
                    break;
                }
                if (p->hit == 0) {
                    p->ang = (func_800DFF80(p, D_8009D254 + 40) + 0x800) & 0xFFF;
                    break;
                }
                i = func_80079FB4((short)h - (short)prev, (h >> 16) - (prev >> 16)) & 0xFFF;
                if (i >= 0x800) i -= 0x800;
                p->ang = (i + (i - p->ang)) & 0xFFF;
            } while (cnt--);
            p->x = pos.vx;
            p->z = pos.vz;
            if (cnt <= 0) p->speed = p->speed * p->damp / 16;
            if (p->grav != 0) {
                p->y -= p->vy;
                if ((p->y >> 16) >= p->floor - 32) {
                    p->y = (p->floor - 32) << 16;
                    p->vy = -p->vy * p->bounce / 16;
                    p->speed = p->speed * p->damp / 16;
                    if (p->vy < 0x300000) {
                        p->grav = 0;
                    } else {
                        unsigned char *e = func_800CE610(*(void **)(D_800F33E0 + 8));
                        if (e) {
                            ((short *)e)[0] = (p->x >> 16);
                            ((short *)e)[1] = D_800942EC;
                            ((short *)e)[2] = (p->z >> 16);
                        }
                    }
                } else {
                    p->vy -= p->grav;
                }
            } else {
                p->speed -= p->decel;
                if (p->speed <= 0) p->done = 1;
            }
            if (p->floor - 512 < (p->y >> 16)) {
                res = func_800DFE20(&pos, D_8009D254 + 40);
                if (res < 128) p->done = 1;
            }
            if (p->done == 0) return 0;
            if (p->snd != -1) func_800866A4(p->snd, 0);
            p->x = (p->x >> 16);
            p->y = (p->y >> 16) + 32;
            p->z = (p->z >> 16);
            hd = func_8006F39C(37, a);
            for (k = 0; k < 5; k++) func_8006F6D4(hd, 0, k + 1, (&p->x)[D_80195674[k]], 0, 0);
            return 1;
        default:
            return 1;
        }
        break;
    case 2:
    {
        SVECTOR sv;
        for (j = 0; j < 3; j++) (&sv.vx)[j] = (&p->x)[j] >> 16;
        if (D_800E27EC < 8) {
            alpha = 128;
            scale = D_800966EC[((D_800E27EC << 8) & 0xFFF) * 2] + 0x1000;
        } else {
            alpha = D_800966EC[((D_800E27EC << 10) & 0xFFF) * 2 + 1] / 64 + 96;
            scale = 0x1000;
        }
        D_800F3368.w = 32;
        D_800F3368.dw = 2;
        D_800F3368.h = 32;
        D_800F3368.v = 32;
        D_800F3368.h = 32;
        D_800F3368.v = 32;
        av = D_800E2850[D_800E11E8.c];
        D_800F3368.kind = 4;
        D_800F3368.f6E = 0;
        D_800F3372.a = 0;
        D_800F3372.b = 0;
        D_800F3368.clut = av;
        palette = D_800E1204[D_800F3368.kind];
        if (D_800F3368.kind == 4 && D_800F3428 != 0) palette += 4;
        func_800CEE20(&sv, 0, scale, scale, 66, func_80077AA4(48, palette) & 0xFFFF, 1, alpha, 0);
        if (p->state != 0) {
            sv.vy = p->floor;
            func_800D004C(&sv, 80, 80, 8, &D_80195680, 0x1000, 0x1000, &D_80195688, &D_8019568C, alpha, 1);
        }
        D_800F3368.w = 16;
        D_800F3368.dw = 1;
        D_800F3368.h = 16;
        D_800F3368.v = 16;
        av2 = D_800E2850[D_800E11E8.a];
        D_800F3368.kind = 2;
        D_800F3368.f6E = 0;
        D_800F3368.clut = av2;
    }
        break;
    }
    return 0;
}
