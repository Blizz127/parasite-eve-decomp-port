/* room_m0152i (PE.IMG room m0152i chunk 2, VRAM 0x8018EFE8)
 * func_80192940 - blob offset 0x3958, 0x3c0 bytes. Profile era_o2_g0 (default).
 * Written from the retail asm (2026-10-09); masked-body twins: room_m0145i 0x3914, room_m0153i 0x4E28, room_m0399i 0x4918, room_m0401i 0x5A6C.
 * Was the m0152i story 0x1F0 (Day 4) CPU_BOUNDARY stop (@func_801926BC jalr).
 * inline GTE/asm: uses inline PSY-Q GTE (COP2) asm macros.
 *
 * Bounce/steer step for a sliding actor task. On first tick it clears the
 * actor's 0x40000 flag and seeds heading/speed; while the actor is active
 * (state >= 2) it rotates (0, 0, speed>>12) by the heading through the GTE,
 * adds it to the actor's x/z, and nclip-tests the new point against every
 * edge of the room's boundary polygon (D_8009D248, D_8009D1CC vertices).
 * Outside an edge it reflects the heading off that edge (ratan2, one
 * retry), or, the first time, turns toward the player (func_800DFF80).
 * Then y moves by the vertical speed, bouncing on the floor D_800942EC.
 * Matching notes (gcc 2.7.2):
 *  - the scratchpad is a struct, so its stores (MEM_IN_STRUCT_P) don't
 *    alias the D_8009D1CC load and sched hoists that load like retail;
 *  - opz is assigned inside the outer loop, after the mask's first use, so
 *    loop.c hoists 0xFFFF0000 ($s3) ahead of 0x1F800048 ($s5);
 *  - i++ before prev = cur keeps retail's load-delay nop in the edge loop;
 *  - the table half goes through an int local (retail lh, not lhu). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define UH(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

#define gte_SetRotMatrix(r0) __asm__ volatile(                       \
    "lw $12, 0(%0);" "lw $13, 4(%0);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8(%0);" "lw $13, 12(%0);" "lw $14, 16(%0);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4"                     \
    : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldv0(r0) __asm__ volatile("lwc2 $0, 0(%0);" "lwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtv0() __asm__ volatile("nop;" "nop;" ".word 0x4A486012")
#define gte_stlvnl(r0) __asm__ volatile(                                \
    "swc2 $25, 0(%0);" "swc2 $26, 4(%0);" "swc2 $27, 8(%0)" : : "r"(r0) : "memory")
#define gte_ldsxy3(r0, r1, r2) __asm__ volatile(                        \
    "mtc2 %0, $12;" "mtc2 %2, $14;" "mtc2 %1, $13"                       \
    : : "r"(r0), "r"(r1), "r"(r2))
#define gte_nclip() __asm__ volatile("nop;" "nop;" ".word 0x4B400006")
#define gte_stopz(r0) __asm__ volatile("swc2 $24, 0(%0)" : : "r"(r0) : "memory")

typedef struct { short pad; unsigned short x; int z; } PolyPt;
typedef struct {
    int vxy;          /* 0x00 */
    short vz, pad06;  /* 0x04 */
    int mac[3];       /* 0x08 */
    int pad14;
    int x;            /* 0x18 */
    int pad1C;
    int z;            /* 0x20 */
    int pad24;
    short m[3][3];    /* 0x28 */
    short pad3A;
    int t[3];         /* 0x3C */
    int opz;          /* 0x48 */
} Scratch;

extern int D_800966EC[];
extern short D_8009D1CC;
extern PolyPt *D_8009D248;
extern char *D_8009D254;
extern short D_800942EC;
extern int func_80079FB4(int, int);
extern int func_800DFF80(void *, void *);

void func_80192940(char *o)
{
    char *p = o + 0xC;
    Scratch *sc = (Scratch *)0x1F800000;
    char *a = P(o, 0x8);
    short tries;
    int i, n, pt, prev, cur, ang;
    short *t;
    int c;
    int *opz;

    if (B(o, 0x1A) == 0) {
        B(o, 0x1A) = 1;
        W(a, 0x8C) = 0;
        W(a, 0x6C) = 0;
        W(a, 0x98) &= ~0x40000;
        H(o, 0x30) = H(a, 0x3A);
        W(o, 0x28) = W(o, 0x24);
    }
    if (B(a, 0xE) < 2)
        return;
    if (W(a, 0x98) & 0x40000) {
        H(o, 0x30) = (H(o, 0x30) + 0x800) & 0xFFF;
        W(a, 0x98) &= ~0x40000;
    }
    if (H(o, 0x32) != 0)
        return;

    tries = 1;
    do {
        t = (short *)&D_800966EC[(UH(p, 0x24) + 0x800) & 0xFFF];
        c = t[1];
        sc->m[0][0] = c;
        sc->m[0][1] = 0;
        sc->m[0][2] = *(int *)t;
        sc->m[1][0] = 0;
        sc->m[1][1] = 0x1000;
        sc->m[1][2] = 0;
        sc->m[2][0] = -sc->m[0][2];
        sc->m[2][1] = 0;
        sc->m[2][2] = sc->m[0][0];
        sc->vxy = 0;
        sc->vz = W(p, 0x14) >> 12;
        gte_SetRotMatrix(sc->m);
        gte_ldv0(&sc->vxy);
        gte_rtv0();
        gte_stlvnl(sc->mac);
        n = D_8009D1CC;
        sc->x = W(a, 0x28) + (sc->mac[0] << 12);
        sc->z = W(a, 0x30) + (sc->mac[2] << 12);
        prev = D_8009D248[n - 1].x | (D_8009D248[n - 1].z & 0xFFFF0000);
        pt = ((unsigned int)sc->x >> 16) | (sc->z & 0xFFFF0000);
        opz = (int *)0x1F800048;
        for (i = 0; i < D_8009D1CC;) {
            cur = D_8009D248[i].x | (D_8009D248[i].z & 0xFFFF0000);
            gte_ldsxy3(pt, cur, prev);
            gte_nclip();
            gte_stopz(opz);
            if (sc->opz < 0)
                break;
            i++;
            prev = cur;
        }
        if (i >= D_8009D1CC) {
            H(p, 0x2A) = 1;
            break;
        }
        if (H(p, 0x2A) == 0) {
            H(p, 0x24) = (func_800DFF80(a + 0x28, D_8009D254 + 0x28) + 0x800) & 0xFFF;
            break;
        }
        ang = func_80079FB4((short)cur - (short)prev, (cur >> 16) - (prev >> 16)) & 0xFFF;
        if (ang >= 0x800)
            ang -= 0x800;
        tries--;
        H(p, 0x24) = (ang + (ang - UH(p, 0x24))) & 0xFFF;
    } while (tries != -1);
    if (tries < 0)
        H(p, 0x2A) = 0;

    W(a, 0x28) = sc->x;
    W(a, 0x30) = sc->z;
    W(a, 0x2C) += W(p, 0x1C);
    if ((W(a, 0x2C) >> 16) >= D_800942EC) {
        W(a, 0x2C) = D_800942EC << 16;
        W(p, 0x1C) = -W(p, 0x18);
        H(p, 0x26) = H(p, 0x28);
    } else {
        W(p, 0x1C) += W(p, 0x20);
    }
}
