extern unsigned char *D_8009D2C8;
extern short D_800B002C[];
extern unsigned char D_800B8AC0[];
extern void func_80089F08();
extern void func_80089218();

void func_80089250(unsigned int a0)
{
    unsigned int i;
    unsigned int m;
    unsigned int b;

    m = (*(unsigned int *)(D_8009D2C8 + 4) & *(unsigned int *)(D_8009D2C8 + 0xC))
      | (*(unsigned int *)(D_8009D2C8 + 0x6C) & *(unsigned int *)(D_8009D2C8 + 0x74)) | a0;
    for (i = 0, b = 1; i < 0x18; i++) {
        if (m & (b << i)) {
            D_800B002C[i * 4] = 0x7FFF;
        } else {
            func_80089F08(i, &D_800B002C[i * 4]);
            if (D_800B002C[i * 4] == 0) {
                func_80089218(D_800B8AC0, i);
                func_80089218(D_800B8AC0 + 0x1AA0, i);
            }
        }
    }
}
