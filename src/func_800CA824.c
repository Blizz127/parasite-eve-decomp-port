typedef struct { short vx; short vy; short vz; } SVec;

extern unsigned char *D_800E27A8;
extern unsigned short D_800E2360;
extern unsigned short D_800E2362;
extern unsigned short D_800E2364;
extern int func_80071A54(void);
extern void func_80078C34(void *a0, SVec *a1, unsigned char *a2);

void func_800CA824(int a0, int a1, unsigned char *a2) {
    SVec v;

    v.vx = -(func_80071A54() % 3 + 9);
    v.vy = -(func_80071A54() % 3 + 9);
    v.vz = func_80071A54() % 5 - 2;
    func_80078C34(*(void **)(D_800E27A8 + 0x238), &v, a2 + 0x10);
    *(unsigned short *)(a2 + 8) = D_800E2360;
    *(unsigned short *)(a2 + 0xA) = D_800E2362;
    {
        unsigned short t = D_800E2364;
        a2[2] = 0x14;
        a2[1] = 0;
        *(unsigned short *)(a2 + 0xC) = t;
    }
}
