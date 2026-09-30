/* VRAM 0x8003D834 / file 0x2E034 / size 0x118. */
extern int D_8009CDDC;
extern int D_800B1638;
extern int D_80091A38;

int func_8003DFD8();
int func_80039B74();
int func_8003A088();
int func_8003B97C();
int func_8003BCE0();

int func_8003D834(int arg0, int arg1, short arg2, int arg3) {
    int temp_v0;
    int temp_v0_2;

    if (arg1 != 0) {
        *(int *)(arg0 + 0xB0) = arg1;
        func_8003DFD8((int *)(arg0 + 0x34), &D_800B1638, 1);
        func_8003DFD8(&D_80091A38, (int *)(arg0 + 0x34), 1);
        func_80039B74(arg0, arg1, arg2, 0);
    }
    func_8003A088(arg0);
    func_8003DFD8(&D_800B1638, (int *)(arg0 + 0x34), 1);
    func_8003B97C(arg0, arg3);
    func_8003BCE0(arg0, 1, (short)D_8009CDDC);
    temp_v0_2 = D_8009CDDC ^ 1;
    D_8009CDDC = temp_v0_2;
    func_8003BCE0(arg0, 1, (short)temp_v0_2);
    temp_v0 = D_8009CDDC ^ 1;
    D_8009CDDC = temp_v0;
    return temp_v0;
}
