/* VRAM 0x80019D84 / file 0xA584 / size 0x34.
 * Call func_800375E0 with the first halfword of **a0, mode 1, -1 halfword. */
extern void func_800375E0(int a0, int a1, short *a2);
int func_80019D84(short **a0) {
    short buf[8];
    buf[0] = -1;
    func_800375E0(**a0, 1, buf);
    return 1;
}
