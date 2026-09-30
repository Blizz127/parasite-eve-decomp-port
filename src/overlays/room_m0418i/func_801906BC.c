/* room_m0418i — func_801906BC, blob offset 0x16D4, 0x258 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Spawn/init near the owner: jittered position rotated about the anchor (RotMatrix+ApplyMatrixSV), colour fields; struct-array views of a2 (MEM_IN_STRUCT) let sched hoist the D_8009D254 load; D_8009D254 ints read as >>16. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { short x, y, z, w; } SV8;
extern SV8 D_8018EFFC;
extern char *D_8009D254;
extern int D_800B0E64;
extern char *func_800C2B50();
extern int *func_800C2B10();
extern int func_80071A54();
extern void func_800794C4(), func_80078C34(), func_8006DF50();

typedef struct { short h[0x58]; } HS;
typedef struct { unsigned char b[0xB0]; } BS;
#define B(off) ((BS *)a2)->b[off]
#define H(off) ((HS *)a2)->h[(off) / 2]

void func_801906BC(int a0, int a1, unsigned char *a2)
{
    SV8 d;
    SV8 o;
    SV8 rot;
    MATRIX m;
    char *r;
    char *p;
    register int n asm("$2");
    int t;
    int t2;
    volatile int *e;

    r = func_800C2B50();
    rot = D_8018EFFC;
    B(0xAA) = 3;
    H(0x10) = *(int *)(r + 0x18);
    H(0x12) = *(int *)(r + 0x1C);
    H(0x14) = *(int *)(r + 0x20);
    H(0x8) = *(int *)(r + 0x18);
    H(0xA) = *(int *)(r + 0x1C);
    H(0xC) = *(int *)(r + 0x20);
    p = D_8009D254;
    H(0) = *(int *)(p + 0x28) >> 16;
    H(2) = *(int *)(p + 0x2C) >> 16;
    H(4) = *(int *)(p + 0x30) >> 16;
    n = func_80071A54() % 300;
    t = *(unsigned short *)(a2 + 0);
    t -= 150;
    t += n;
    H(0) = t;
    n = func_80071A54() % 300;
    t2 = *(unsigned short *)(a2 + 4);
    t2 -= 150;
    t2 += n;
    H(4) = t2;
    rot.y = *func_800C2B10(2);
    func_800794C4(&rot, &m);
    d.x = H(0) - H(0x10);
    d.y = H(2) - H(0x12);
    d.z = H(4) - H(0x14);
    func_80078C34(&m, &d, &o);
    H(0) = H(0x10) + o.x;
    H(2) = H(0x12) + o.y;
    H(4) = H(0x14) + o.z;
    B(0x9A) = 0xF0;
    B(0x98) = 0x80;
    B(0x9E) = 0xF0;
    B(0x99) = 0x80;
    B(0x9C) = 0x20;
    B(0x9D) = 0x20;
    ((int *)a2)[0xA0 / 4] = 0xFF;
    B(0xA9) = 0;
    *(int *)(a2 + 0xA4) = 0;
    B(0xA8) = *func_800C2B10(1);
    e = &D_800B0E64;
    if (*e != 0) {
        func_8006DF50(*e, 0x606, 0, 0x80, 0x7F);
    }
}
