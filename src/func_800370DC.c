/* VRAM 0x800370DC / file 0x278DC / size 0x64. */
extern void func_80077C84(unsigned char *packet, int a1, int a2, int a3);
extern void func_80077C04(unsigned char *packet);
extern int func_80077CB4(unsigned char *head, unsigned char *packet);
extern void func_800719E4(int code);

void func_800370DC(unsigned char *head, int tag) {
    unsigned char *packet = head + 8;

    func_80077C84(head, 0, 1, tag);
    func_80077C04(packet);
    if (func_80077CB4(head, packet) != 0) {
        func_800719E4(-1);
    }
}
