/* VRAM 0x80042170 / file 0x32970 / size 0xB8 (46 words).
 *
 * Memory-card save LOAD entry: validates the card record, then re-arms it
 * for the read.  era -O2 -G0.
 *
 * The `*(s0 + 0x18) = D_8009EED0` store must be issued first: cc1 then hoists
 * the `la $a0,D_8009EED0` to immediately after func_80042798() (retail's
 * 0x329B8/0x329BC) and leaves it live until the closing func_80071A24 call;
 * writing it after the other stores sinks the la and shifts 9 words. */
extern unsigned char D_800A0ED4[];
extern unsigned char D_8009EED0[];
extern void func_80042798(void);
extern void func_80071A24(unsigned char *address, int size);

int func_80042170(int card, int slot) {
    unsigned char *s0 = D_800A0ED4 + card * 0x418;
    int s1 = slot;

    if (s0[0] != 1) {
        return 0;
    }
    func_80042798();
    *(unsigned int *)(s0 + 0x18) = (unsigned int)D_8009EED0;
    *(unsigned short *)(s0 + 0x14) = 0x2000;
    *(unsigned short *)(s0 + 0x16) = 0x0A;
    s0[7] = 2;
    s0[1] = 1;
    s0[0xB] = 5;
    s0[3] = (unsigned char)s1;
    *(unsigned int *)0x800A1854 = (unsigned int)s0;
    *(unsigned int *)0x800A1858 = 0x2000;
    func_80071A24(D_8009EED0, 0x2000);
    return 0;
}
