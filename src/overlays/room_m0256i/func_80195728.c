/* room_m0256i func_80195728 -- blob offset 0x6740, 0x5A0 bytes.  inline GTE/asm
 * Room effect controller (cmd 0/1/2, same shape as room_m0023i func_8018FC14).
 * cmd 0: clears the counter, creates the mesh D_801960A8 (func_8006E498 /
 *   func_800C6D5C) and arms a 8-by-16 particle list with func_8019552C.
 * cmd 1: ramps the counter by 4 up to 128; on D_800E27EC == 1 spawns up to
 *   16 particles (x = 0, z = 512 + rand & 0x1FF, y = rand); done at 16.
 * cmd 2: palette block, func_800CE8F0 position, ring func_800D004C, then
 *   (from D_800E27EC >= 4) a scaled/rotated func_800C6xxx mesh pass, and
 *   the closing palette block. * Default profile era_o2_g0 (-O2 -G0, no knobs). LINK_EXACT.
 * D_8018F258 is the 8-byte template at blob 0x270, split as [0x270, rodata] (M0256_HEADER_DECISION.md).
 * Levers (gcc 2.7 alias model decides most of the schedule):
 *   - D_800F3368 is one struct (as in room_m0023i func_8018FC14); each a70
 *     table store is written right after the a76/a78 stores and before
 *     a6C/a6E so the D_800E2850[] load stays between them;
 *   - D_801960A0 is a GteShortVector (struct-fixed stores keep the in-struct
 *     D_800F32D0[2] pool load after them) while D_80196094 stays a scalar
 *     (alias-exempt, so its store sinks below the pool load);
 *   - r / r2 pointer locals for D_80196098 (retail $s0 / $a0 base);
 *   - the object pointer is pinned to $17 behind a zero-code asm barrier
 *     (without it the param pseudo wins and it lands in $s2: 10 words);
 *   - one zero-code asm memory barrier before the /40 call keeps the ang
 *     stores ahead of the mult (209 words without it);
 *   - the D_800E27EC - 4 value reuses the loop counter i ($s1);
 *   - case 2 ends with break (return 0 there costs 4 words of $v0 pressure
 *     and a stolen delay slot); a68 before a6A and a6C before a6E. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short x, y, z; } Particle;

extern GteShortVector D_8018F258;      /* 8-byte template (data, not code) */
extern unsigned char D_80195E8C[];
extern int func_8019552C();
extern int D_801960A8;
extern short D_80196094;
extern short D_80196098;
extern GteShortVector D_801960A0;   /* +0 x, +2 y, +4 z */
extern int D_800B0E64;
extern int D_800E27EC;
extern void **D_800F33E0;
extern void **D_800F32D0;
extern unsigned short D_800E11E8, D_800E11EA, D_800E11FA;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern void *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_8006E498();
extern void func_800C6D5C();
extern int func_800CE560();
extern Particle *func_800CE610();
extern int func_80071A54();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern int func_80077DC4();
extern int func_80077CF4();
extern void func_800CF3AC();
extern void func_800D004C();
extern int func_80077A64();
extern int func_80077AA4();
extern void func_800C6EC0();
extern void func_800C6ED8();
extern void func_80079754();
extern void func_80078CC4();
extern void func_800C6EF8();
extern void func_800C6FA0();
extern void func_800C71E4();
extern void func_800C6F4C();

int func_80195728(int a0, short *a1_)
{
    register short *a1 asm("$17") = a1_;
    GteShortVector eff;
    GteShortVector pos;
    GteShortVector ang;
    unsigned char buf[8];
    MATRIX m;
    VECTOR sc;
    int mode;
    int i;
    int s;
    int c;
    int u;
    int k;
    int w;
    int t;
    int n;
    short v;
    Particle *o;
    unsigned short *tbl;
    short *r;
    short *r2;
    register GteShortVector *e asm("$6");

    __asm__ volatile("" : "=r"(a1) : "0"(a1));
    mode = a0;
    eff = D_8018F258;
    e = &eff;

    switch (mode) {
    case 0:
        *a1 = 0;
        D_801960A8 = func_8006E498(D_800B0E64, 0xC5487704, e);
        func_800C6D5C(D_801960A8, 0, 0);
        return func_800CE560(D_800F33E0[2], 8, 16, func_8019552C);
    case 1:
        v = *a1;
        if (v < 128)
            *a1 = v + 4;
        if (D_800E27EC == 1) {
            for (i = 0; i < 16; i++) {
                o = func_800CE610(D_800F33E0[2]);
                if (o != 0) {
                    o->x = 0;
                    o->z = (func_80071A54() & 0x1FF) + 0x200;
                    o->y = func_80071A54();
                }
            }
        }
        if (D_800E27EC < 16)
            goto ret0;
        return 1;
    case 2:
        D_800F3368.a68 = 0x40;
        D_800F3368.a6A = 4;
        D_800F3368.a76 = 0x40;
        D_800F3368.a78 = 0x40;
        D_800F3368.a70 = D_800E2850[D_800E11FA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 1;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x18;
        func_800CE8F0(D_800F32D0[2], 2, e, &pos);
        n = D_800E27EC;
        tbl = D_800E2850;
        ang.x = 0;
        ang.y = 0;
        ang.w = 0;
        ang.z = -n << 6;
        __asm__ volatile("" ::: "memory");
        func_80077DC4((n << 10) / 40);
        func_800CF3AC(D_80195E8C, buf, D_800E27EC * 3);
        func_800D004C(&pos, 0xA0, 0xA0, 8, 0, 0x1000, 0x1000, buf, 0, 0x80, 1);
        D_80196094 = *a1;
        D_801960A0.x = pos.x;
        D_801960A0.y = pos.y;
        D_801960A0.z = pos.z;
        r = &D_80196098;
        func_800CE9D4(D_800F32D0[2], 2, r);
        *r += 0x400;
        if (D_800E27EC >= 4) {
            i = D_800E27EC - 4;
            t = (i << 10) / 12;
            s = func_80077DC4(t);
            c = func_80077CF4(t) / 32;
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            D_800F3368.a70 = tbl[D_800E11EA];
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 0;
            u = (tbl[D_800E11EA] | func_80077A64(0, 1, 0, 0)) & 0xFFFF;
            k = D_800F3368.a6C;
            w = D_800E1204[k];
            if (k == 4 && D_800F3428 != 0)
                w += 4;
            func_800C6EC0(u, func_80077AA4(0x50, w) & 0xFFFF);
            func_800C6ED8(1);
            func_80079754(&D_80196098, &m);
            m.t[0] = pos.x;
            m.t[1] = pos.y;
            m.t[2] = pos.z;
            sc.vx = s;
            sc.vy = s;
            sc.vz = s;
            func_80078CC4(&m, &sc);
            func_800C6EF8(D_801960A8);
            func_800C6FA0(D_801960A8, c & 0xFFFF);
            func_800C71E4(D_801960A8, &m);
            func_800C6F4C(D_801960A8);
        }
        D_800F3368.a68 = 0x10;
        D_800F3368.a6A = 1;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        r2 = &D_80196098;
        *r2 += 0x600;
        D_800F3368.a70 = D_800E2850[D_800E11E8];
        D_800F3368.a6C = 2;
        D_800F3368.a6E = 0;
        D_800F3368.a74 = 12;
        break;
    }
ret0:
    return 0;
}
