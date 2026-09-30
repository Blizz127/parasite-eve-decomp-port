typedef struct {
    unsigned char f0;
    unsigned char pad1[0xF];
    short f10;
    unsigned char pad2[0x26];
} R;

extern R D_800BCEA8[];

void func_8003746C(short a0) {
    unsigned char i;

    i = 0;
    do {
        if (D_800BCEA8[i].f10 == a0 && D_800BCEA8[i].f0 != 0) {
            D_800BCEA8[i].f0 = 0;
            return;
        }
        i++;
    } while (i < 4);
}
