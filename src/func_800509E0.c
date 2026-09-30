/* VRAM 0x800509E0 / file 0x411E0 / size 0xF8.
 * BGM/SE selector: reads the two gp small-data flags (0x1AC = 0x8009CF1C,
 * 0x1A8 = 0x8009CF18), picks a queue id, then drives the sound-command
 * helpers; also clears a pending slot-0x44 entry. era_o2_g8. */
extern int D_8009CF1C;
extern int D_8009CF18;
extern unsigned char D_8009D02C;
extern signed char D_800C0E20[];
extern signed char D_800C0E22[];

int func_80059F08();
int func_80052558();
int func_8005EB58();
int func_80063198();
int func_80062A34();
int func_800536B8();
int func_800631AC();
int func_80062CB8();
int func_8005F5B8();

int func_800509E0(void) {
    int var_s0;
    int temp_byte;
    int *temp_v1;

    if (D_8009CF1C != 0) {
        var_s0 = func_80059F08(0);
        goto block;
    }
    func_8005EB58(func_80052558() == 0);
    if (D_8009CF18 != 0) {
        temp_byte = D_800C0E20[0];
    } else {
        temp_byte = D_800C0E22[0];
    }
    var_s0 = temp_byte;
block:
    if (var_s0 >= 0) {
        func_80063198(func_80062A34(1, 6));
        return func_800536B8(var_s0);
    }
    func_800631AC(func_80062A34(1, 6));
    temp_v1 = (int *)func_80062A34(2, 6);
    if (temp_v1[0x11] >= 0) {
        temp_v1[0x11] = -1;
        temp_v1 = (int *)func_80062A34(2, 5);
        temp_v1[0x11] = 0;
        func_80062CB8(temp_v1);
    }
    return func_8005F5B8(0x39);
}
