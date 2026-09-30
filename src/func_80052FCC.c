typedef struct Rec {
    unsigned char pad0[5];
    unsigned char f5;
    unsigned char f6;
    unsigned char pad1[0x19];
} Rec;

extern Rec D_800C0EAC[];

void func_80052FCC(unsigned char *a0)
{
    Rec *q;
    int flag;

    if (a0 == 0) {
        return;
    }
    flag = (a0[6] == 9);
    for (q = D_800C0EAC; q < D_800C0EAC + 0x80; q++) {
        if (q->f6 != 9) {
            if (flag != 1) {
                goto do_clear;
            }
            continue;
        }
        if (flag == 0) {
            continue;
        }
    do_clear:
        q->f5 &= 0xEF;
    }
    a0[5] |= 0x10;
}
