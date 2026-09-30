extern unsigned char D_800A22E0[];
extern unsigned char D_800A2FD0[];
extern unsigned char *D_8009D158;
extern int D_8009D15C;
extern int D_8009D154;

void func_80062568(void)
{
    unsigned char *p;
    unsigned char *end;
    unsigned char *q;

    p = D_800A22E0;
    end = D_800A22E0 + 0xD80;
    while (p < end) {
        *(unsigned char **)p = p + 0x90;
        p = p + 0x90;
    }
    q = D_800A2FD0;
    *(unsigned char **)q = 0;
    D_8009D158 = q - 0xCF0;
    D_8009D15C = 0;
    D_8009D154 = 0;
}
