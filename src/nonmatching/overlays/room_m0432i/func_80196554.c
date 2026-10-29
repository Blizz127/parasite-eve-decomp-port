/* room_m0432i — func_80196554, blob offset 0x756C, 0xAD8 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-spawner / callback (a0 = command 0/1/2, a1 = effect record), read from
 * the retail disassembly. Registered with func_800CE560 by itself (cmd 0) and
 * the callback func_801962FC is attached to a second record.
 *   cmd 0: clears +0x18/+0x1A/+0x1C, loads the D_800B0E64 resource 0xC54C4704
 *          into D_80199500, registers the record with func_800CE8F0 /
 *          func_800CE9D4, shifts +0/+2 by (-0x220, -0x80) and registers
 *          func_801962FC with func_800CE560 (its result is the return value).
 *   cmd 1: state (+0x18) 0 waits 16 frames (+0x1A) then goes to state 1; state 1
 *          spawns one debris particle per frame via func_800CE610 for 16 frames
 *          then goes to state 2; state 2 returns 2 (retail leaves v0 = 2).
 *   cmd 2: state 0 / state 1 set the GTE rotation/translation from D_800BCFA4[0],
 *          drive the D_80199500 3D effect (func_800C6xxx) with a scale matrix and
 *          draw sprites through func_800CEE20/func_800D004C/func_800D3BC8
 *          (state 1 only). Every cmd 2 path ends D_800F3372 = 0, D_800F3374 = 4
 *          (0x28 stays only inside the state-1 draw calls) and returns 0.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned char c[4]; } B4;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern B4 D_8018F218;
extern GteShortVector D_8018F1F4;
extern GteShortVector D_8018F1E0;
extern int D_800B0E64;
extern int D_80199500;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern MATRIX *D_800BCFA4[];
extern unsigned short D_800E11EA;
extern unsigned short D_800E11FA;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800E27EC;
extern int D_800F3428;
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern unsigned char func_801962FC[];
extern int func_8006E498();
extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077A64();
extern int func_80077AA4();
extern void func_80079754();
extern void func_80078CC4();
extern void func_800C6D5C();
extern void func_800C6EC0();
extern void func_800C6ED8();
extern void func_800C6EF8();
extern void func_800C6F4C();
extern void func_800C6FA0();
extern void func_800C71E4();
extern int func_800CE560();
extern unsigned char *func_800CE610();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern void func_800CEE20();
extern void func_800CFB7C(void *, short, void *);
extern void func_800D004C();
extern void func_800D3BC8();

int func_80196554(int mode, unsigned char *a1)
{
    B4 b30;
    GteShortVector v;
    GteShortVector d;
    GteShortVector v48;
    GteShortVector b50;
    GteShortVector b58;
    MATRIX M;
    int sc[3];
    unsigned char *p;
    int n, s4, s6, s0, a, clut;

    b30 = D_8018F218;
    b50 = D_8018F1F4;
    b58 = D_8018F1E0;

    if (mode == 1) {
        func_800CFB7C(a1, 0x140, &v);
        v.x += H(a1, 8);
        v.y += H(a1, 0xA);
        v.z += H(a1, 0xC);
        switch (SH(a1, 0x18)) {
        case 0:
            H(a1, 0x1A)++;
            if (SH(a1, 0x1A) < 0x10)
                return 0;
            SH(a1, 0x18) = 1;
            SH(a1, 0x1A) = 0;
            return 0;
        case 1:
            H(a1, 0x1A)++;
            func_800CFB7C(a1, 0xA0, &d);
            H(a1, 8) += d.x;
            H(a1, 0xA) += d.y;
            H(a1, 0xC) += d.z;
            p = func_800CE610(*(int *)(D_800F33E0 + 8));
            if (p) {
                H(p, 0) = v.x;
                H(p, 2) = v.y;
                H(p, 4) = v.z;
                H(p, 6) = (func_80071A54() & 0xFF) + 0x180;
                H(p, 8) = H(a1, 0);
                H(p, 0xA) = H(a1, 2);
                H(p, 0xC) = H(a1, 4);
                H(p, 0xE) = func_80071A54();
                H(p, 0x10) = 0;
                H(p, 0x12) = 0;
            }
            if (SH(a1, 0x1A) < 0x10)
                return 0;
            SH(a1, 0x18) = 2;
            SH(a1, 0x1A) = 0;
            return 0;
        case 2:
            return 2;
        }
        return 0;
    }
    if (mode == 0) {
        H(a1, 0x18) = 0;
        H(a1, 0x1A) = 0;
        H(a1, 0x1C) = 0;
        D_80199500 = func_8006E498(D_800B0E64, 0xC54C4704);
        func_800C6D5C(D_80199500, 0, 0);
        func_800CE8F0(*(int *)(D_800F32D0 + 8), 0, &b58, a1 + 8);
        H(a1, 0x10) = H(a1, 8);
        H(a1, 0x12) = H(a1, 0xA);
        H(a1, 0x14) = H(a1, 0xC);
        /* retail loads the arena pointer after the +0x10..0x14 stores (a1 may alias it) */
        func_800CE9D4(*(volatile int *)(D_800F32D0 + 8), 0, a1);
        H(a1, 0) -= 0x220;
        H(a1, 2) -= 0x80;
        return func_800CE560(*(int *)(D_800F33E0 + 8), 0x14, 0x18, func_801962FC);
    }
    if (mode != 2)
        return 0;

    gte_SetRotMatrix(D_800BCFA4[0]);
    gte_SetTransMatrix(D_800BCFA4[0]);
    func_800CFB7C(a1, 0x140, &v);
    v.x += H(a1, 8);
    v.y += H(a1, 0xA);
    v.z += H(a1, 0xC);

    switch (SH(a1, 0x18)) {
    case 0:
        n = func_80077CF4(SH(a1, 0x1A) << 6);
        s4 = n / 32;
        break;
    case 1:
        s6 = SH(a1, 0x1A) << 6;
        n = func_80077DC4(s6);
        s4 = (n / 2 + 0x800) / 32;
        break;
    default:
        goto done;
    }
    if (D_800E27EC & 1)
        s4 = (s4 * 3) / 4;

    D_800F3368.a6C = 3;
    D_800F3368.a6E = 1;
    D_800F3368.a70 = D_800E2850[D_800E11FA];
    s0 = func_80077A64(0, 1, 0, 0);
    s0 = (D_800E2850[D_800E11FA] | s0) & 0xFFFF;
    a = D_800E1204[D_800F3368.a6C];
    if (D_800F3368.a6C == 4 && D_800F3428 != 0)
        a += 7;
    else
        a += 3;
    clut = func_80077AA4(0, a);
    func_800C6EC0(s0, clut & 0xFFFF);
    func_800C6ED8(1);
    v48.x = H(a1, 0);
    v48.y = H(a1, 2);
    v48.z = H(a1, 4);
    v48.x += 0x800;
    func_80079754(&v48, &M);
    M.t[0] = SH(a1, 8);
    M.t[1] = SH(a1, 0xA);
    M.t[2] = SH(a1, 0xC);
    sc[0] = 0x2AA;
    sc[1] = 0x2AA;
    sc[2] = 0x2AA;
    func_80078CC4(&M, sc);
    func_800C6EF8(D_80199500);
    func_800C6FA0(D_80199500, s4 & 0xFFFF);
    func_800C71E4(D_80199500, &M);
    func_800C6F4C(D_80199500);

    if (SH(a1, 0x18) == 0)
        goto done;

    /* state 1: three layered sprites around the stack vector v */
    D_800F3368.a68 = 0x40;
    D_800F3368.a6A = 4;
    D_800F3368.a76 = 0x40;
    D_800F3368.a78 = 0x40;
    D_800F3368.a6C = 3;
    D_800F3368.a6E = 1;
    D_800F3368.a72 = 0;
    D_800F3368.a74 = 0x28;
    D_800F3368.a70 = D_800E2850[D_800E11FA];
    b50.z = SH(a1, 0x1A) << 5;
    clut = func_80077AA4(0, D_800E120A + 3);
    func_800CEE20(&v, &b50, 0x1000, 0x1000, 4, clut & 0xFFFF, 1, s4 / 2, 0);
    s0 = func_80077CF4(s6) + 0x1000;
    func_800D004C(&v, 0x44C, 0x5DC, 0x20, 0, s0, s0, &b30, 0, s4, 1);

    D_800F3368.a68 = 0x20;
    D_800F3368.a6A = 2;
    D_800F3368.a76 = 0x20;
    D_800F3368.a78 = 0x20;
    a = D_800E1204[D_800F3368.a6C];
    if (D_800F3368.a6C == 4 && D_800F3428 != 0)
        a += 8;
    else
        a += 4;
    clut = func_80077AA4(0, a);
    func_800D3BC8(&v, 0x1000, 0x1000, D_800F3368.a6A + 0x64, clut & 0xFFFF, 1, s4, 0, 0x600);

    a = D_800E2850[D_800E11EA];
    D_800F3368.a6C = 3;
    D_800F3368.a6E = 0;
    D_800F3368.a68 = 0x20;
    D_800F3368.a6A = 2;
    D_800F3368.a76 = 0x20;
    D_800F3368.a78 = 0x20;
    D_800F3368.a70 = a;
    clut = func_80077AA4(0x40, D_800E120A);
    func_800D3BC8(&v, 0x1800, 0x1800, 2, clut & 0xFFFF, 1, s4, 0, 0xA00);

    D_800F3368.a68 = 0x40;
    D_800F3368.a6A = 4;
    D_800F3368.a76 = 0x40;
    D_800F3368.a78 = 0x40;
    a = D_800E1204[D_800F3368.a6C];
    if (D_800F3368.a6C == 4 && D_800F3428 != 0)
        a += 4;
    clut = func_80077AA4(0x50, a);
    func_800D3BC8(&v, 0x1800, 0x1800, 4, clut & 0xFFFF, 1, s4, 0, 0xE00);

done:
    D_800F3368.a72 = 0;
    D_800F3368.a74 = 4;
    return 0;
}
