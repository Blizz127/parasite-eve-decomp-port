/* func_80085644 — VRAM 0x80085644, size 0xE4, file 0x75E44-0x75F28.
 *
 * CD/stream subsystem boot: bring up the low-level driver, register the
 * D_800B6958 buffer with id 4, clear and size D_8009B7FC, then poll the
 * three hardware handshakes and the D_8009CDE0 event object until each
 * reports ready.  The 0xF2000002 command word keeps its hoisted `lui` in
 * the loop heads (retail re-does only the `ori` in the delay slot).
 *
 * era -O2 -G0.  D_8009B7FC and D_8009CDE0 are inside gp range but retail
 * addresses them absolutely, so -G0 is required.
 */
extern int D_8009CDE0;
extern unsigned char D_800B6958[];
extern unsigned char D_8009B7FC[];
extern void func_8007D15C(void);
extern void func_80085A04(int, void *);
extern void func_80085F14(int);
extern void func_80085EB4(int);
extern void func_800850F4(void *, int);
extern void func_80085174(void);
extern void func_80085290(void);
extern void func_80085C44(int);
extern void func_80085D84(int);
extern int func_80085814(int, int, int);
extern int func_800858E8(int);
extern int func_800726E4(int, int, int, void *);
extern int func_80072704(int);
extern void func_8008E23C(void);

int func_80085644(void) {
    int t;

    func_8007D15C();
    func_80085A04(4, D_800B6958);
    func_80085F14(0);
    func_80085EB4(0x1010);
    func_800850F4(D_8009B7FC, 0x20);
    func_80085174();
    func_80085290();
    func_80085C44(0);
    func_80085D84(0);
    do {
    } while (func_80085814(0xF2000002, 0x44E8, 0x1000) == 0);
    do {
    } while (func_800858E8(0xF2000002) == 0);
    do {
        t = func_800726E4(0xF2000002, 2, 0x1000, func_8008E23C);
        D_8009CDE0 = t;
    } while (t == -1);
    do {
        t = func_80072704(D_8009CDE0);
    } while (t == 0);
    return t;
}
