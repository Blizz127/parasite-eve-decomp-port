/* Pointer-install + three callees after default-record init.
 * VRAM 0x8002F76C / file 0x1FF6C / size 0x6C (27 words).
 * gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * Stores &D_800B8A20 through *a0, &D_800B0CB0 at D_800B8A88,
 * &D_8009D1B0 at D_800B8A8C, then jal 5218C / 51980(0, B8A88) / 51E64(B8A8C).
 */
extern unsigned char D_800B8A20[];
extern unsigned char D_800B0CB0[];
extern unsigned int D_8009D1B0;
extern unsigned char *D_800B8A88;
extern unsigned char *D_800B8A8C;

extern void func_8005218C(void);
extern void func_80051980(int a0, unsigned char *a1);
extern void func_80051E64(unsigned char *a0);

void func_8002F76C(unsigned char **a0) {
    *a0 = D_800B8A20;
    D_800B8A88 = D_800B0CB0;
    D_800B8A8C = (unsigned char *)&D_8009D1B0;
    func_8005218C();
    func_80051980(0, D_800B8A88);
    func_80051E64(D_800B8A8C);
}
