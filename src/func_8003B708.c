extern unsigned char D_800B0CEC;
extern int *D_800B0D70;
extern int D_80091A38;
extern int D_80091A3C;
extern int D_80091A40;
extern int D_80091A44;
extern int D_80091A48;
extern int D_80091A4C;
extern int D_80091A50;
extern int D_80091A54;
extern unsigned char D_800BEA40;
extern void func_80039B74();
extern void func_8003A088();
extern void func_8006698C();
extern void func_8003B97C();
extern void func_8003BCE0();

void func_8003B708(unsigned char *a0, int a1) {
    int sv[8];
    int t;
    int arg;

    if (a0 != &D_800B0CEC) {
        t = *(int *)(a0 + 0x34);
        sv[1] = *(int *)(a0 + 0x38);
        sv[2] = *(int *)(a0 + 0x3C);
        sv[3] = *(int *)(a0 + 0x40);
        sv[4] = *(int *)(a0 + 0x44);
        sv[5] = *(int *)(a0 + 0x48);
        sv[6] = *(int *)(a0 + 0x4C);
        sv[7] = *(int *)(a0 + 0x50);
        arg = *(int *)(a0 + 0xB0);
        sv[0] = t;
        *(int *)(a0 + 0x34) = D_80091A38;
        *(int *)(a0 + 0x38) = D_80091A3C;
        *(int *)(a0 + 0x3C) = D_80091A40;
        *(int *)(a0 + 0x40) = D_80091A44;
        *(int *)(a0 + 0x44) = D_80091A48;
        *(int *)(a0 + 0x48) = D_80091A4C;
        *(int *)(a0 + 0x4C) = D_80091A50;
        *(int *)(a0 + 0x50) = D_80091A54;
        func_80039B74(a0, arg, 0, 1);
        func_8003A088(a0);
        func_8006698C(a0);
        func_8003B97C(a0, &D_800BEA40);
        func_8003BCE0(a0, 1, (short)a1);
        *(int *)(a0 + 0x34) = sv[0];
        *(int *)(a0 + 0x38) = sv[1];
        *(int *)(a0 + 0x3C) = sv[2];
        *(int *)(a0 + 0x40) = sv[3];
        *(int *)(a0 + 0x44) = sv[4];
        *(int *)(a0 + 0x48) = sv[5];
        *(int *)(a0 + 0x4C) = sv[6];
        *(int *)(a0 + 0x50) = sv[7];
        func_80039B74(a0, *(int *)(a0 - 4), *(short *)(a0 - 0x19E), 1);
        func_8003A088(a0);
    } else {
        int *p = D_800B0D70;
        *(volatile int *)&p[0] = D_80091A38;
        *(volatile int *)&p[1] = D_80091A3C;
        *(volatile int *)&p[2] = D_80091A40;
        *(volatile int *)&p[3] = D_80091A44;
        *(volatile int *)&p[4] = D_80091A48;
        *(volatile int *)&p[5] = D_80091A4C;
        *(volatile int *)&p[6] = D_80091A50;
        p[7] = D_80091A54;
        func_8006698C(a0);
        func_8003B97C(a0, &D_800BEA40);
        func_8003BCE0(a0, 1, (short)a1);
    }
}
