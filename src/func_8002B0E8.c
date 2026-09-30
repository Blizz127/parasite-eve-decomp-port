typedef struct {
    unsigned char pad0[0xF];
    unsigned char fF;
    unsigned char pad10[0x6];
    unsigned short f16;
    unsigned char pad18[0x2];
    unsigned short f1A;
    unsigned char pad1C[0x7C];
    unsigned int f98;
} Actor;

extern unsigned char D_8009CE74;
extern Actor *D_8009D254;
extern unsigned int D_8009D1A0;
extern void *D_8009D304;
extern unsigned short D_8009D21C;
extern unsigned char D_800A7FF0[];
extern short D_8009D2A4;
extern int D_8009D28C;
extern unsigned int D_800B0CD8[];

extern void func_800703F4(void);
extern void func_8004B70C(void *a0, int a1, void *a2);
extern void func_80067CBC(void);
extern void func_8001A680(Actor *a0, int a1);
extern int func_8006D60C(int a0);
extern void func_800295E4(void);

void func_8002B0E8(void) {
    Actor *a;
    int k;

    switch (D_8009CE74) {
    case 0:
        a = D_8009D254;
        if (a->f16 == 10 || (D_8009D1A0 & 0x800)) {
            a->f98 |= 0x100;
            func_800703F4();
            func_8004B70C(D_8009D304, D_8009D21C, D_800A7FF0);
            func_80067CBC();
            D_8009CE74++;
        }
        break;
    case 1:
        if (D_8009D2A4 == 1000) {
            D_8009CE74++;
            D_8009D254->f98 &= ~0x100;
        }
        break;
    case 2:
        a = D_8009D254;
        if (a->fF == a->f1A) {
            if (!(D_8009D1A0 & 0x1800)) {
                k = 0x15;
            } else {
                D_8009D1A0 &= ~0x1800;
                k = 0x18;
            }
            func_8001A680(a, k);
            D_8009CE74++;
        }
        break;
    case 3:
        if (func_8006D60C(0) != 1) {
            func_800295E4();
            D_8009D28C = 9;
            D_800B0CD8[0] &= ~0x8000;
        }
        break;
    }
}
