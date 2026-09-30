typedef struct {
    unsigned char flag;
    unsigned char pad[0xB];
    unsigned int word;
    unsigned char pad2[0x28];
} Rec;
extern Rec D_800BCEA8[4];
void func_800374E8(void) {
    unsigned char i = 0;
    do {
        D_800BCEA8[i].flag = 0;
        D_800BCEA8[i].word &= 0xFDFFFFFF;
        i++;
    } while (i < 4);
}
