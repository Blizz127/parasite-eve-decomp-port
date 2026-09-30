extern int D_8009D13C;
extern int D_8009D140;
extern int D_8009D144;

void func_8005F844(int enabled) {
    D_8009D13C = enabled ? 0x3A1C : 0x395D;
    D_8009D140 = enabled ? 0xCC : 0x84;
    D_8009D144 = 0xA4;
}
