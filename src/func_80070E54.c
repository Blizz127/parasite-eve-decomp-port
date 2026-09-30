extern int D_8009CDDC;
extern unsigned int D_800B0CD8[];
extern unsigned char D_800BCE80[][20];
extern unsigned char D_800BCDC8[][92];

int func_80074DC0(int mode);
void func_80042FE8(void);
int func_80073A44(int mode);
int func_8006EBE4(void);
int func_80074D28(int mask);
int func_80074A44(int mode);
void func_800755F0(void *env);
int func_8006EC08(void);
void func_80075424(void *env);
void func_800754E4(void *ot, void *env);

void func_80070E54(void) {
    func_80074DC0(0);
    func_80042FE8();
    if (D_800B0CD8[0] & 0x200) {
        func_80073A44(4);
        if ((short)func_8006EBE4() >= 3) {
            func_80074D28(1);
        }
    } else {
        func_80073A44(2);
    }
    func_80074A44(1);
    func_800755F0(D_800BCE80[D_8009CDDC]);
    if ((signed char)func_8006EC08() || (D_800B0CD8[0] & 0x200)) {
        func_80075424(D_800BCDC8[D_8009CDDC]);
    } else {
        func_800754E4((unsigned char *)D_800B0CD8[0x58 + D_8009CDDC] + 0x3FFC, D_800BCDC8[D_8009CDDC]);
    }
    D_8009CDDC = (D_8009CDDC == 0);
}
