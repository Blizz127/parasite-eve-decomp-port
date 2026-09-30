/* VRAM 0x8005C498 / file 0x4CC98 / size 0xFC.
 * Frame-start handler: if func_80042ED0 is clear, records arg0, runs the
 * eight startup calls, advances the gp small-data counter 0x2C0
 * (0x8009D030) or calls func_800425DC, clears the 0x2C4 (0x8009D034) flag,
 * and re-runs func_800514F8. else func_80042F44 and return 0. era_o2_g8. */
extern int D_8009D030;
extern int D_8009D034;
extern unsigned char D_8009D02C;
extern int D_8009D1E0[];

int func_80042ED0();
int func_80051504();
int func_8005E6F0();
int func_80046334();
int func_8005E30C();
int func_8004F464();
int func_80042B6C();
int func_80062FEC();
int func_8005E788();
int func_800425DC();
int func_800512AC();
int func_800514F8();
int func_800339A0();
int func_80042F44();

int func_8005C498(int arg0) {
    int temp_v1;

    if (func_80042ED0() == 0) {
        D_8009D1E0[0] = arg0;
        func_80051504();
        func_8005E6F0();
        func_80046334();
        func_8005E30C();
        func_8004F464();
        func_80042B6C();
        func_80062FEC();
        func_8005E788(1);
        temp_v1 = D_8009D030;
        if (temp_v1 >= 2) {
            func_800425DC();
        } else if (temp_v1 > 0) {
            D_8009D030 = temp_v1 + 1;
        }
        if (D_8009D034 != 0) {
            D_8009D034 = 0;
            func_800512AC(9, 0);
        }
        if (func_800514F8() != 0) {
            func_800339A0(D_8009D02C);
        }
        return func_800514F8();
    }
    func_80042F44();
    return 0;
}
