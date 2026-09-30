extern unsigned int D_8009D02C;

unsigned int func_80033A20(void);
void func_800339A0(int value);

void func_8005C144(void) {
    D_8009D02C = func_80033A20() & 0xFF;
    func_800339A0(0);
}
