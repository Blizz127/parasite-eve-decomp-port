/* VRAM 0x80050B48 / file 0x41348 / size 0x4C. era -O2 -G0. */
int func_80054288(void);
int func_800556E8(int index);
int func_8005DC9C(int value);
int func_8005F27C(int value);

int func_80050B48(int index) {
    int result;

    result = index < func_80054288();
    if (result != 0) {
        result = func_8005F27C(func_8005DC9C(func_800556E8(index) + 0xEB));
    }
    return result;
}
