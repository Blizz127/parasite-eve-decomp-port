extern unsigned int D_800A1860;
extern unsigned char D_800A0ED5;
void func_800428D4(void) {
    unsigned int i = D_800A1860 - 1;
    *(unsigned char *)((unsigned char *)&D_800A0ED5 + i * 0x418) = 0xD;
}
