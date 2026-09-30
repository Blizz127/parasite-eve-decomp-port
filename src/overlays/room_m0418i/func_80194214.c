/* room_m0418i — func_80194214, blob offset 0x522C, 0x1B8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * History shift (2x SV8 unaligned + VECTOR copies), timeline thresholds on a1+2, floor/collision test via func_8001CAB0, proximity via func_800C6B90; t/x/z locals keep the single loads. */

typedef struct { short x, y, z, w; } SV8;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short x, y, z; } SV;
extern int D_8009D248;
extern unsigned short D_8009D1CC;
extern char *func_800C2B50();
extern int func_8001CAB0();
extern int func_800C6B90();

void func_80194214(int a0, unsigned char *a1, char *a2)
{
    SV s;
    char *r;
    unsigned int i;
    int t;
    int x, z;

    r = func_800C2B50();
    for (i = 2; i != 0; i--) {
        ((SV8 *)(a2 + 0x30))[i] = ((SV8 *)(a2 + 0x30))[i - 1];
        ((VECTOR *)a2)[i] = ((VECTOR *)a2)[i - 1];
        ((SV8 *)(a2 + 0x48))[i] = ((SV8 *)(a2 + 0x48))[i - 1];
    }
    if (*(short *)(a1 + 2) >= 0x33) {
        t = *(unsigned short *)(a2 + 0x8A) + *(unsigned short *)(a2 + 0x8C);
        *(short *)(a2 + 0x8A) = t;
        *(short *)(a2 + 0x34) = *(short *)(a2 + 0x34);
        *(int *)(a2 + 0) -= (short)t;
    }
    if ((unsigned short)(*(short *)(a1 + 2) - 0x22) < 9) {
        *(short *)(a2 + 0x48) += 0x12C;
    }
    if (*(short *)(a1 + 2) >= 0x4C) {
        *(short *)(a2 + 0x4A) += 0x96;
    }
    if (*(short *)(a1 + 2) == 0x78) {
        a1[1] = 2;
    }
    x = *(int *)(a2 + 0);
    s.x = x;
    s.y = *(int *)(a2 + 4);
    z = *(int *)(a2 + 8);
    s.z = z;
    if (func_8001CAB0(x << 16, z << 16, D_8009D248, D_8009D1CC) == 0) {
        a1[1] = 2;
    }
    if (func_800C6B90(&s, 0x320) != 0) {
        *(short *)(r + 0x2C) = 1;
    }
}
