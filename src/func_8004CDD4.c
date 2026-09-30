extern unsigned char D_800A1A20[];

void func_8005E8A4(int a0, int a1);
void func_8005F594(int value);

void func_8004CDD4(void *state) {
    register unsigned char *record asm("$4");

    func_8005E8A4(0, 0xA);
    record = D_800A1A20;
    if (*(int *)((unsigned char *)state + 0x24) == 0x3D) {
        record += 0x40;
    }
    func_8005F594((int)record);
}
