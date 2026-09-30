extern unsigned char D_800F3450;
extern unsigned char D_800F3451;
extern unsigned char D_800F3452;
extern unsigned short D_800E2290;
extern unsigned short D_800E2292;
extern unsigned short D_800E2294;
extern int *func_800C2B10(int a0);
extern int func_80071A54(void);

void func_800CBFC4(int a0, int a1, unsigned char *a2) {
    D_800F3450 = *func_800C2B10(0);
    D_800F3451 = *func_800C2B10(1);
    D_800F3452 = *func_800C2B10(2);
    { register int r asm("$2"); register int b asm("$5"); r = func_80071A54() % 201; b = D_800E2290; b = b - 0x64; b = b + r; *(unsigned short *)(a2 + 6) = b; }
    *(unsigned short *)(a2 + 8) = D_800E2292;
    { register int r asm("$2"); register int b asm("$5"); r = func_80071A54() % 201; b = D_800E2294; *(unsigned short *)(a2 + 4) = 0x20C; a2[3] = 0x7F; b = b - 0x64; b = b + r; *(unsigned short *)(a2 + 0xA) = b; }
}
