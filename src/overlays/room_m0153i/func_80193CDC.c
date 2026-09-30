/* room_m0153i (PE.IMG room m0153i chunk 2, VRAM 0x8018EFE8)
 * func_80193CDC — blob offset 0x4cf4, 0x134 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0145i func_801927C8; C re-targeted by symbol address
 * (docs/evidence/room_m0153i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define UH(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern int func_8003010C();
extern void func_80030220();
extern void func_801941D0();
extern void func_80193E10();
void func_80193CDC(void *o)
{
    void *a = P(o, 0x1C);
    void *c = P(o, 0x8);

    W(c, 0x28) = W(P(a, 0x238), 0x94) << 16;
    W(c, 0x2C) = W(P(a, 0x238), 0x98) << 16;
    W(c, 0x30) = W(P(a, 0x238), 0x9C) << 16;
    *(SVECTOR *)((char *)c + 0x38) = *(SVECTOR *)((char *)a + 0x38);
    if (B(a, 0xE) == 7 && UH(a, 0x16) >= 3 && UH(a, 0x1A) < 3) {
        P(o, 0xC) = func_80193E10;
    }
    if (P(a, 0x0) != 0) {
        if (func_8003010C(a, 44) > 0) {
            return;
        }
        func_80030220(c, 45, 0);
        func_80030220(c, 44, 0);
        func_80030220(c, 95, 0);
    }
    func_801941D0(o);
}
