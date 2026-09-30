/* VRAM 0x800347B4 / file 0x24FB4 / size 0x1A8. */
extern unsigned int D_8009D290;
extern unsigned char *D_8009D278;
extern unsigned char *D_8009D254;
extern int D_8009CDDC;
extern unsigned int D_8009CDD8;
extern unsigned char *D_800B0DF8[];
extern unsigned char *D_800B0E38[];
extern unsigned int D_800B0E58[];

extern void func_8006DE80(int id, int a1, int x, int y, int z);
extern void func_800661A4(void);
extern void func_800661CC(void);
extern int func_8007041C(unsigned char *, unsigned char *, int, int, unsigned char *, unsigned int);

void func_800347B4(void) {
    int s;
    int unused[4]; /* retail frame reserves 16 bytes of locals (0x38 frame) */
    int size;
    unsigned int n;

    n = D_8009D290;
    if (n < 10) {
        if ((*(int *)(D_8009D278 + 0x4C) & 0x30) == 0x10) {
            s = (n * 5) << 18;
        } else {
            s = ((*(short *)(*(unsigned char **)(D_8009D278 + 0x68) + 2) << 16) * n) / 10;
        }
        size = D_8009D290 << 6;
    } else {
        if ((*(int *)(D_8009D278 + 0x4C) & 0x30) == 0x10) {
            s = 0xC80000;
        } else {
            s = *(short *)(*(unsigned char **)(D_8009D278 + 0x68) + 2) << 16;
        }
        size = (D_8009D290 << 4) + 10;
    }
    if (D_8009D290 == 0) {
        func_8006DE80(0x457, 0, *(short *)(D_8009D254 + 0x2A), *(short *)(D_8009D254 + 0x2E), *(short *)(D_8009D254 + 0x32));
    }
    func_800661A4();
    D_8009CDD8 += func_8007041C(D_800B0DF8[0], D_8009D254 + 0x28, s, size & 0xFFFE,
                                   D_800B0E38[D_8009CDDC] + 8, D_800B0E58[D_8009CDDC] + D_8009CDD8);
    func_800661CC();
    D_8009D290++;
}
