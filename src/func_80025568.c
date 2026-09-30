extern unsigned int D_8009D1A0;
extern unsigned int *D_8009D254;
extern unsigned int D_8009D278;
extern void func_800218D8(void);
extern void func_800209F0(void);
extern void func_80021AF8(void);

void func_80025568(int a0) {
    if (D_8009D1A0 & 2) {
        return;
    }
    D_8009D278 = *D_8009D254;
    switch (a0) {
    case 0x197:
        func_800218D8();
        func_800209F0();
        break;
    case 0x198:
        func_80021AF8();
        break;
    }
}
