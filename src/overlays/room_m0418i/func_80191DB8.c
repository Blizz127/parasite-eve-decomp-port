/* room_m0418i — func_80191DB8, blob offset 0x2DD0, 0xC0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * 4-slot init: +0x28[i]=i<<11, zero vectors, +0x48[i]=rand()%10+10 (struct arrays) */

typedef struct { short x, y, z, pad; } SV;
typedef struct {
    unsigned char pad0[8];
    SV v[4];
    int a[4];
    short b[4];
    unsigned char pad40[8];
    short c[4];
    unsigned char pad50[8];
    short d58, d5A;
} W;
extern int func_80071A54();

void func_80191DB8(void *a0, void *a1, W *a2)
{
    unsigned int i;

    a2->d58 = 0;
    a2->d5A = 0;
    for (i = 0; i < 4; i++) {
        a2->a[i] = i << 11;
        a2->b[i] = 0;
        a2->v[i].x = 0;
        a2->v[i].y = 0;
        a2->v[i].z = 0;
        a2->c[i] = func_80071A54() % 10 + 10;
    }
}
