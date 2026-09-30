/* room_m0273i — func_80192D8C, blob offset 0x3DA4, 0xB0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * teardown: flags linked records =4, optional func_80020CE4, then func_80192C00 */

extern unsigned char *D_8009D254;
extern void func_80020CE4();
extern void func_80192C00();

int func_80192D8C(unsigned char *a0)
{
    unsigned char *s = a0 + 0xC;
    unsigned char *e;

    e = **(unsigned char ***)(a0 + 8);
    if (e != 0) {
        **(unsigned char **)(e + 0x18) = 4;
    }
    **(int **)(a0 + 0x10) = 0;
    a0[0] = 4;
    if (a0[0x4B] != 0) {
        e = *(unsigned char **)D_8009D254;
        if (e != 0 && *(short *)(e + 0xC) > 0) {
            func_80020CE4();
        }
        func_80192C00(*(void **)(a0 + 8), s);
    }
    return 0;
}
