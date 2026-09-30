extern unsigned char D_8009CDB4;
extern unsigned char D_800A3180[0x150];
extern unsigned char *D_8009D20C;
extern unsigned char *func_80012700(int a0, int a1);

void func_80065400(void)
{
    unsigned char i;
    unsigned char *r;
    unsigned char *b;
    unsigned char *t;
    unsigned char *u;
    int key;

    i = 0;
    while (i < D_8009CDB4) {
        r = &D_800A3180[i * 12];
        if (r[4] != 0) {
            key = r[4];
            b = D_8009D20C;
            while (b != 0) {
                if (*(unsigned short *)(b + 0x24) == key) {
                    if (*(int *)(b + 0x19C) != 0) {
                        t = func_80012700(*(int *)(b + 0x19C), 0);
                        *(unsigned short *)(t + 8) = *(unsigned short *)(t + 8) | 4;
                        *(int *)(t + 0xC) = *(int *)(r + 8);
                        *(int *)(t + 0x14) = r[3];
                        u = *(unsigned char **)(b + 0xA8);
                        *(unsigned char **)(t + 0x24) = u;
                        if (u != 0) {
                            *(unsigned char **)(u + 0x28) = t;
                        }
                        *(unsigned char **)(b + 0xA8) = t;
                    }
                    b = 0;
                } else {
                    b = *(unsigned char **)(b + 4);
                }
            }
        } else {
            b = D_8009D20C;
            while (b != 0) {
                if (b[0xC] == *(unsigned short *)r && *(int *)(b + 0x19C) != 0 && b[0xD] == r[2]) {
                    t = func_80012700(*(int *)(b + 0x19C), 0);
                    *(unsigned short *)(t + 8) = *(unsigned short *)(t + 8) | 4;
                    *(int *)(t + 0xC) = *(int *)(r + 8);
                    *(int *)(t + 0x14) = r[3];
                    u = *(unsigned char **)(b + 0xA8);
                    *(unsigned char **)(t + 0x24) = u;
                    if (u != 0) {
                        *(unsigned char **)(u + 0x28) = t;
                    }
                    *(unsigned char **)(b + 0xA8) = t;
                }
                b = *(unsigned char **)(b + 4);
            }
        }
        i++;
    }
    D_8009CDB4 = 0;
}
