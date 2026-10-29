/* room_m0113i (PE.IMG room m0113i chunk 2, VRAM 0x8018EFE8)
 * func_80190028 — blob offset 0x1040, 0xa54 bytes. Profile era_o2_g0_expand_div
 * (-O2 -G0, MASPSX_EXPAND_DIV=1).
 * Masked-body twin of room_m0104i func_8018F3A4; C re-targeted by symbol address
 * (tools/analysis/room_lane/port.py). */

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

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))



extern GteShortVector D_8018F008;
extern GteShortVector D_8018F010;
extern void func_8018FC8C();
extern unsigned char *D_800E2368;
extern void *D_800F32D0;
extern void *D_800F33E0;
extern void **D_8009D254;
extern short D_800942EC;
extern unsigned short D_800E11EC, D_800E11E8, D_800E120C;
extern unsigned short D_800E2850[];
extern int D_800F3428;
extern int D_8009CDDC, D_8009CDD8;
extern int D_8009D248;
extern unsigned short D_8009D1CC;
extern MATRIX *D_800BCFA4[];
typedef struct { char *ordering[2]; char *other_buffers[6]; char *packets[2]; } RenderBufferPrefix;
extern RenderBufferPrefix D_800B0E38;
extern int func_800D3FD8();
extern void func_800D3F64();
extern int func_800CE560();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern void *func_800CE610();
extern int func_80077DC4();
extern int func_80077CF4();
extern void func_800783E4();
extern int func_80071A54();
extern int func_8001CAB0();
extern int func_800C6B90();
extern int func_80077AA4();
extern void func_800CEE20();
extern void func_800CF6F8();
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

int func_80190028(int mode, unsigned char *a1, unsigned short *p)
{
    GteShortVector eff = D_8018F008;
    GteShortVector eff2 = D_8018F010;
    short v38[4];
    unsigned int buf[2];
    int s4, j, r2, a3v;
    unsigned char *q, *c;
    void *o, **pp, *r;
    unsigned short *ob;
    char *prim;
    MATRIX **pm;
    register GteShortVector *e asm("$6");

    e = &eff;
    switch (mode) {
    case 0:
        SH(a1, 0x18) = 0;
        SH(a1, 0x1A) = 0;
        SH(a1, 0x1E) = 0;
        SH(a1, 0x20) = 0;
        SH(a1, 0x22) = 0;
        H(a1, 0x10) = p[0];
        H(a1, 0x12) = p[1];
        H(a1, 0x14) = p[2];
        H(a1, 0x12) = D_800942EC;

        func_800D3F64(0x57B, func_800D3FD8());
        if (D_800E2368[0xD] != 0) {
            pp = P(D_800F32D0, 8);
            if (pp != 0) {
                r = *pp;
                if (r != 0) {
                    c = P(r, 0x18);
                    if (*c == 1) *c = 2;
                }
            }
        }
        return func_800CE560(P(D_800F33E0, 8), 0x14, 0x18, func_8018FC8C);
    case 1:
        func_800CE8F0(P(D_800F32D0, 8), 0x13, e, a1);
        func_800CE9D4(P(D_800F32D0, 8), 0, v38);
        s4 = -v38[1] + 0x400;
        switch (SH(a1, 0x18)) {
        case 0:
            H(a1, 0x1A)++;
            if (SH(a1, 0x1A) == 1) {
                for (j = 0; j < 2; j++) {
                    ob = func_800CE610(P(D_800F33E0, 8));
                    if (ob) {
                        ob[0] = H(a1, 0);
                        ob[1] = H(a1, 2);
                        ob[2] = H(a1, 4);
                        ob[4] = func_80077DC4(s4) / 256;
                        ob[6] = func_80077CF4(s4) / 256;
                        ob[5] = 0;
                        ob[8] = j;
                        ob[9] = 0;
                    }
                }
            }
            a3v = 0x1000 - func_80077DC4((SH(a1, 0x1A) << 10) / 6);
            buf[1] = a3v;
            func_800783E4(a1, a1 + 0x10, 0x1000 - a3v, a3v, a1 + 8);
            SH(a1, 0x1C) = 0x80 - (SH(a1, 0x1A) << 5) / 6;
            if (SH(a1, 0x1A) < 6) return 0;
            SH(a1, 0x18) = 1;
            SH(a1, 0x1A) = 0;
            for (j = 0; j < 8; j++, s4 += 0x200) {
                ob = func_800CE610(P(D_800F33E0, 8));
                if (ob) {
                    ob[0] = H(a1, 0x10);
                    ob[1] = H(a1, 0x12);
                    ob[2] = H(a1, 0x14);
                    r2 = (func_80071A54() & 7) + 0x14;
                    ob[4] = func_80077DC4(s4) * r2 / 0x1000;
                    ob[6] = func_80077CF4(s4) * r2 / 0x1000;
                    ob[5] = 0;
                    ob[8] = 0;
                    ob[9] = 0;
                }
            }
            if (func_8001CAB0(SH(a1, 0x10) << 16, SH(a1, 0x14) << 16, D_8009D248, D_8009D1CC) == 0) return 1;
            break;
        case 1:
            H(a1, 0x1A)++;
            SH(a1, 0x1C) = func_80077DC4((SH(a1, 0x1A) << 10) / 6) / 128 + 0x40;
            H(a1, 8) = H(a1, 0x10);
            H(a1, 0xA) = H(a1, 0x12);
            H(a1, 0xC) = H(a1, 0x14);
            SH(a1, 0x1E) = func_80077CF4((SH(a1, 0x1A) << 10) / 6) + 0x800;
            SH(a1, 0x20) = 0x80;
            if (SH(a1, 0x1A) < 6) return 0;
            SH(a1, 0x18) = 2;
            SH(a1, 0x1A) = 0;
            SH(a1, 0x1C) = 0;
            for (j = 0; j < 12; j++) {
                ob = func_800CE610(P(D_800F33E0, 8));
                if (ob) {
                    int t = func_80071A54();
                    int b = t & 0xFFF;
                    buf[1] = b;
                    func_800783E4(a1, a1 + 0x10, 0x1000 - b, b, ob);
                    ob[5] = 0;
                    ob[7] = (func_80071A54() & 3) + 1;
                    ob[8] = 1;
                    ob[9] = 0;
                }
            }
            return 0;
        case 2:
            H(a1, 0x1A)++;
            if (SH(a1, 0x1E) > 0x1000) SH(a1, 0x1E) -= 0x400;
            SH(a1, 0x20) = func_80077CF4((SH(a1, 0x1A) << 14) / W(p, 8)) / 128 + 0x80;
            if (SH(a1, 0x22) != 0) SH(a1, 0x22) = SH(a1, 0x22) - 1;
            if (func_800C6B90(a1 + 0x10, 0x104) != 0 && SH(a1, 0x22) == 0) {
                if (D_800E2368[0xD] != 0) {
                    o = D_800F32D0;
                    if ((**(int **)P(o, 8) & 0x3F000000) == 0x01000000) {
                        W(*D_8009D254, 0x4C) |= 0x4000;
                        **(int **)P(o, 8) = (**(int **)P(o, 8) & 0xC0FFFFFF) | 0x19000000;
                        **(int **)P(o, 8) |= 0x80000000;
                    }
                }
                SH(a1, 0x22) = W(p, 0xC);
            }
            if (SH(a1, 0x1A) < W(p, 8)) return 0;
            SH(a1, 0x18) = 3;
            SH(a1, 0x1A) = 0;
            break;
        case 3:
            H(a1, 0x1A)++;
            SH(a1, 0x1E) = func_80077DC4(SH(a1, 0x1A) << 5);
            SH(a1, 0x20) = func_80077DC4(SH(a1, 0x1A) << 5) / 32;
            if (SH(a1, 0x1A) >= 0x20) return 1;
            break;
        }
        break;
    case 2:
        pm = D_800BCFA4;
        gte_SetRotMatrix(pm[0]);
        gte_SetTransMatrix(pm[0]);
        if (SH(a1, 0x1C) != 0) {
            prim = D_800B0E38.packets[D_8009CDDC] + D_8009CDD8;
            D_8009CDD8 += 0x10;
            __asm__ volatile("lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)\n\tlwc2 $2,0(%1)\n\tlwc2 $3,4(%1)\n\tlwc2 $4,0(%0)\n\tlwc2 $5,4(%0)\n\tnop\n\tnop\n\t.word 0x4A280030" : : "r"(a1), "r"(a1 + 8) : "memory");
            B(prim, 3) = 3;
            B(prim, 7) = 0x40;
            B(prim, 4) = H(a1, 0x1C);
            B(prim, 5) = H(a1, 0x1C);
            B(prim, 6) = H(a1, 0x1C);
            __asm__ volatile("mfc2 $12,$19\n\tnop\n\tsra $12,$12,2\n\tsw $12,0(%0)" : : "r"(&buf[0]) : "$12", "memory");
            if (buf[0] - 1 < 0xFFF) {
                __asm__ volatile("swc2 $12,0(%0)\n\tswc2 $13,0(%1)\n\tswc2 $14,0(%2)" : : "r"(prim + 8), "r"(prim + 0xC), "r"(&buf[1]) : "memory");
                func_800CF6F8(D_800B0E38.ordering[D_8009CDDC] + buf[0] * 4, prim, 1);
            }
        }
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11EC];
        D_800F3368.a6C = 4;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 5;
        D_800F3368.a74 = 0;
        if (SH(a1, 0x20) != 0) {
            int w = D_800E120C;
            if (D_800F3428 != 0) w += 4;
            func_800CEE20(a1 + 0x10, &eff2, SH(a1, 0x1E), SH(a1, 0x1E), 0, func_80077AA4(0, w) & 0xFFFF, 1, SH(a1, 0x20) / 2, 0);
        }
        D_800F3368.a68 = 0x10;
        D_800F3368.a6A = 1;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a70 = D_800E2850[D_800E11E8];
        D_800F3368.a6C = 2;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 8;
        break;
    }
    return 0;
}
