/* room_m0358i (PE.IMG room m0358i chunk 2, VRAM 0x8018EFE8)
 * func_80192814 (port of room_m0063i func_8018F320) — blob offset 0x382C, 0x9F4 bytes. Profile era_o2_g0_expand_div
 * (-O2 -G0, MASPSX_EXPAND_DIV=1; retail has break guards around the div, and
 * 417 words mismatch without it).
 * Thrown-object update: on the first frame (o+0x1A == 0) it seeds the launch
 * position/velocity from the owner (func_800DFF80/DFC80/DFE94, a Y-rotation
 * matrix built in the scratchpad from the D_800966EC sin/cos table); every frame
 * it rotates the velocity, integrates the arc, and on landing (or when the
 * horizontal distance is < 0x100) swaps the handler to func_80193208. Otherwise
 * it composes the draw matrix into link+0x1E8 and the position into link+0x28.
 * Same body family as room_m0318i func_8018F6A8 (vendor), offsets shifted.
 * Masked-body port of room_m0063i func_8018F320 (handler func_8018FD14 -> func_80193208).
 * Levers (lane s637):
 *   - one `sc` scratch pointer: cse folds it to absolute addresses before the
 *     first join (lui $at stores) and keeps $s1-relative forms after it;
 *   - the product of the arc step must stay in LO across the if/else join
 *     (retail's mflo $t0 after the join). Any mult whose result is not in LO
 *     needs an LO reload, and 2.7 reload then evicts product to the stack (+8
 *     frame). So each distance square goes through a temp and an empty register
 *     tie (`__asm__("" : "=r"(a) : "0"(t1))`): the temp stays in LO, and the
 *     tie's input reload is an LO->GR mflo (no LO spill). a/b pinned $3/$7;
 *   - the vendor arc-step shape (pinned `work` in $3, memory barriers, the
 *     `"r"(upd)` use) and one MB() before the second bias block, so sched1
 *     cannot hoist that block's mult above the product use (LO conflict);
 *   - block-local `bias2` (retail homes it in $a0); SH 0x82 = 0 after SH 0x80.
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros plus empty
 * register-tie and memory-barrier asm — count separately from plain C.
 * Partly vendor-derived: the arc-step block follows khasinski/parasite-eve-decomp@3067919e
 * src/overlays/room_m318/RoomLib_HandlerD.c (in this repo as room_m0318i func_8018F6A8)
 * — attribution khasinski (Chris Hasinski); imported under the owner's 2026-09-28
 * authorization (vendor/khasinski-parasite-eve-decomp/PROVENANCE.md).
 * The rest was hand-written in this repo (lane s637). */
#define MB() __asm__ volatile("" : : : "memory")
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)" : : "r"(r0) : "memory")
#define gte_rtv0() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A486012")
#define gte_stlvnl(r0) __asm__ volatile ( \
    "swc2 $25,0(%0)\n\tswc2 $26,4(%0)\n\tswc2 $27,8(%0)" : : "r"(r0) : "memory")
#define gte_ldclmv(r0) __asm__ volatile ( \
    "lhu $12,0(%0)\n\tlhu $13,6(%0)\n\tlhu $14,12(%0)\n\t" \
    "mtc2 $12,$9\n\tmtc2 $13,$10\n\tmtc2 $14,$11" : : "r"(r0) : "$12", "$13", "$14")
#define gte_rtir() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A49E012")
#define gte_stclmv(r0) __asm__ volatile ( \
    "mfc2 $12,$9\n\tmfc2 $13,$10\n\tmfc2 $14,$11\n\t" \
    "sh $12,0(%0)\n\tsh $13,6(%0)\n\tsh $14,12(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")
#define gte_MulMatrix0(r1, r2, r3) { \
    gte_SetRotMatrix(r1); \
    gte_ldclmv(r2); gte_rtir(); gte_stclmv(r3); \
    gte_ldclmv((char *)(r2) + 2); gte_rtir(); gte_stclmv((char *)(r3) + 2); \
    gte_ldclmv((char *)(r2) + 4); gte_rtir(); gte_stclmv((char *)(r3) + 4); }

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern int D_800966EC[];
extern void **D_8009D254;
extern void func_80193208();
extern int func_800DFF80(void *, void *);
extern int func_800DFC80(void *, void *);
extern void func_800DFE94(void *, void *, void *);
extern int func_800DFC44(int);
extern int func_800DFE20(void *, void *);
extern void func_80079754(void *, void *);

/* Y-rotation matrix at scratch offset m from angle a (sin/cos table) */
#define ROTY(m, a) { \
    int *r_ = (int *)((char *)D_800966EC + (((a) & 0xFFF) << 2)); \
    int c_ = SH(r_, 2); \
    SH(sc, (m) + 2) = 0; \
    SH(sc, (m) + 0) = c_; \
    SH(sc, (m) + 4) = *r_; \
    SH(sc, (m) + 6) = 0; \
    SH(sc, (m) + 8) = 0x1000; \
    SH(sc, (m) + 0xA) = 0; \
    SH(sc, (m) + 0xC) = -SH(sc, (m) + 4); \
    SH(sc, (m) + 0xE) = 0; \
    SH(sc, (m) + 0x10) = SH(sc, (m) + 0); }

void func_80192814(unsigned char *o)
{
    unsigned char *st = o + 0xC;
    unsigned char *link = P(o, 8);
    unsigned char *sub = P(link, 0x18C);
    unsigned char *sc = (unsigned char *)0x1F800000;
    int bias;

    if (B(o, 0x1A) == 0) {
        int v;
        B(o, 0x1A) = 1;
        W(P(o, 8), 0x98) |= 0x10000;
        H(P(o, 8), 0x250) |= 0x400;
        W(o, 4) = 0;
        **(int **)sub |= 0x40000000;
        SH(sc, 0x70) = 0x80;
        SH(sc, 0x72) = 0;
        SH(sc, 0x74) = -0x15E;
        ROTY(0, H(sub, 0x3A));
        gte_SetRotMatrix(sc);
        gte_ldv0(sc + 0x70);
        gte_rtv0();
        gte_stlvnl(sc + 0x40);
        W(o, 0x5C) = W(sub, 0x28) + (W(sc, 0x40) << 16);
        W(o, 0x60) = W(sub, 0x2C) + (W(sc, 0x44) << 16);
        W(o, 0x64) = W(sub, 0x30) + (W(sc, 0x48) << 16);
        if (P(o, 0x7C) != 0) {
            W(o, 0x4C) = W(P(o, 0x7C), 0x28);
            W(o, 0x50) = W(P(o, 0x7C), 0x2C);
            W(o, 0x54) = W(P(o, 0x7C), 0x30);
        }
        v = func_800DFF80(o + 0x5C, o + 0x4C);
        ROTY(0, v);
        gte_SetRotMatrix(sc);
        gte_ldv0(o + 0x6C);
        gte_rtv0();
        gte_stlvnl(sc + 0x40);
        W(o, 0x4C) += W(sc, 0x40) << 16;
        W(o, 0x50) += W(sc, 0x44) << 16;
        W(o, 0x54) += W(sc, 0x48) << 16;
        v = func_800DFC80(o + 0x5C, o + 0x4C);
        v += (W(o, 0x84) * 0x70) >> 21;
        if (v < 0x148) v = 0x148;
        {
            int q = (0x5160000 / v) << 8;
            W(o, 0x8C) = q;
            W(o, 0x90) = q >> 3;
        }
        func_800DFE94(o + 0x5C, o + 0x4C, o + 0x74);
        func_80079754(o + 0x74, o + 0x1C);
    }

    bias = 0x8000;
    SH(sc, 0x90) = 0;
    ROTY(0, ((W(st, 0x84) + bias) >> 16) - 0x400);
    SH(sc, 0x70) = 0;
    SH(sc, 0x72) = 0;
    SH(sc, 0x74) = W(st, 0x74) >> 12;
    gte_SetRotMatrix(sc);
    gte_ldv0(sc + 0x70);
    gte_rtv0();
    gte_stlvnl(sc + 0x60);
    W(st, 0x30) += W(sc, 0x60) << 12;
    W(st, 0x38) += W(sc, 0x68) << 12;
    {
        register int work asm("$3");
        int product;
        if (W(sc, 0x68) >= 0) {
            int pos;
            pos = W(st, 0x74);
            pos += W(st, 0x78);
            work = (pos + bias) >> 16;
            product = work * (W(st, 0x80) >> 12);
            MB();
            W(st, 0x74) = pos;
            MB();
            work = W(st, 0x84);
        } else {
            int upd;
            work = W(st, 0x74);
            work = (work + bias) >> 16;
            product = work * (W(st, 0x80) >> 12);
            MB();
            upd = W(st, 0x74);
            upd -= W(st, 0x78);
            __asm__ volatile("" : : "r"(upd));
            work = W(st, 0x84);
            W(st, 0x74) = upd;
        }
        W(st, 0x84) = work + ((product >> 8) << 12);
    }

    MB();
    {
    int bias2 = 0x8000;
    SH(sc, 0x80) = ((((W(st, 0x30) + bias2) >> 16) * W(st, 0x7C)) + 0x800) >> 12;
    SH(sc, 0x82) = 0;
    SH(sc, 0x84) = (W(st, 0x38) + bias2) >> 16;
    }
    if (W(sc, 0x68) < 0) {
        int t1, t2;
        register int a asm("$3");
        register int b asm("$7");
        t1 = SH(st, 0x32) * SH(st, 0x32);
        __asm__("" : "=r"(a) : "0"(t1));
        t2 = SH(st, 0x3A) * SH(st, 0x3A);
        __asm__("" : "=r"(b) : "0"(t2));
        if (func_800DFC44(a + b) < 0x100)
            SH(sc, 0x90) = 1;
    }
    if (H(sc, 0x90) != 0) {
        W(sub, 0x14) = 0x380000;
        W(P(o, 8), 0x98) &= ~0x10000;
        H(P(o, 8), 0x250) &= ~0x400;
        W(st, 0) = (int)func_80193208;
        return;
    }

    SH(sc, 0x70) = 0;
    SH(sc, 0x72) = W(o, 4) * 0x280 + 0x640;
    SH(sc, 0x74) = -0x400;
    func_80079754(sc + 0x70, sc);
    ROTY(0x20, H(sc, 0x72));
    SH(sc, 0x70) = 0x100;
    SH(sc, 0x72) = 0;
    SH(sc, 0x74) = 0;
    gte_SetRotMatrix(sc + 0x20);
    gte_ldv0(sc + 0x70);
    gte_rtv0();
    gte_stlvnl(sc + 0x40);
    SH(sc, 0x8A) = -0xC8;
    SH(sc, 0x88) = H(sc, 0x80) + W(sc, 0x40);
    SH(sc, 0x8C) = H(sc, 0x84) + W(sc, 0x48);
    gte_SetRotMatrix(st + 0x10);
    gte_ldv0(sc + 0x80);
    gte_rtv0();
    gte_stlvnl(sc + 0x40);
    gte_ldv0(sc + 0x88);
    gte_rtv0();
    gte_stlvnl(sc + 0x50);
    gte_MulMatrix0(st + 0x10, sc, sc + 0x20);
    W(link, 0x28) = W(st, 0x50) + (W(sc, 0x40) << 16);
    W(link, 0x2C) = W(st, 0x54) + (W(sc, 0x44) << 16);
    W(link, 0x30) = W(st, 0x58) + (W(sc, 0x48) << 16);
    W(link, 0x1FC) = SH(st, 0x52) + W(sc, 0x50);
    W(link, 0x200) = SH(st, 0x56) + W(sc, 0x54);
    W(link, 0x204) = SH(st, 0x5A) + W(sc, 0x58);
    H(link, 0x1E8) = H(sc, 0x20);
    H(link, 0x1EA) = H(sc, 0x22);
    H(link, 0x1EC) = H(sc, 0x24);
    H(link, 0x1EE) = H(sc, 0x26);
    H(link, 0x1F0) = H(sc, 0x28);
    H(link, 0x1F2) = H(sc, 0x2A);
    H(link, 0x1F4) = H(sc, 0x2C);
    H(link, 0x1F6) = H(sc, 0x2E);
    H(link, 0x1F8) = H(sc, 0x30);
    if (SH(st, 0x88) == 0) {
        if (func_800DFE20(link + 0x28, (char *)D_8009D254 + 0x28) < 200) {
            int *p;
            void **dp = D_8009D254;
            SH(st, 0x88) = 1;
            W(*dp, 0x4C) |= 0x4000;
            p = *(int **)sub;
            if (p != 0) *p |= 0x80000000;
        }
    }
    if (H(sub, 0x1A) >= 0x2E) W(sub, 0x14) = 0x180000;
}
