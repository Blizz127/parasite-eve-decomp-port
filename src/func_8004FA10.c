/* VRAM 0x8004FA10 / file 0x40210 / size 0xE8.
 * Battle-start setup: queries two records, drives the sound helpers, stores
 * the func_8005332C result to the gp slot 0x1B0 (0x8009CF20), records arg0
 * in gp 0x184 (0x8009CEF4) and registers func_80050AD8 as callback. era_o2_g8. */
extern int D_8009CF20;
extern int D_8009CEF4;

int func_80062A34();
int func_80063428();
int func_80052E30();
int func_80062A20();
int func_800556E8();
int func_80059EC8();
int func_80059F08();
int func_8005332C();
int func_800647D0();
int func_800638D8();
int func_80050AD8();

int func_8004FA10(int arg0) {
    int temp_s0;
    int *temp_v0;

    temp_s0 = func_80062A34(1, 7);
    func_80052E30(func_80063428(func_80062A34(2, 0x36)) == 1);
    if ((temp_s0 != 0) && (((int *)temp_s0)[0x12] == 0)
        && (func_80063428(func_80062A20(temp_s0, 0)) >= 0)) {
        func_80059EC8(1, func_800556E8(func_80063428(func_80062A34(2, 7))));
    }
    temp_v0 = (int *)func_8005332C(func_80059F08(1));
    D_8009CF20 = (int)temp_v0;
    func_800647D0(arg0, *(unsigned char *)((char *)temp_v0 + 0x14));
    D_8009CEF4 = arg0;
    return func_800638D8(arg0, func_80050AD8);
}
