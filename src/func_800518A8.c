extern unsigned char **D_8009D254;
extern short D_800C0E08;
extern signed char D_800C0E20;
extern int func_80052F0C(void);
extern void func_80052E30(int);
extern short *func_8005332C(int);
extern int func_800574A8(void);
extern void func_80051980(int, int);

int func_800518A8(int a0)
{
    unsigned char *rec;
    short *r;
    int saved;
    int ret;

    if (D_8009D254 == 0) {
        goto done;
    }
    rec = *D_8009D254;
    if (rec == 0) {
        goto done;
    }
    D_800C0E08 = *(unsigned short *)(rec + 0xC);
    if (*(int *)(rec + 0x68) == 0) {
        goto done;
    }
    saved = func_80052F0C();
    func_80052E30(0);
    r = func_8005332C(D_800C0E20);
    if (r != 0) {
        r[5] = *(int *)(*(int *)(rec + 0x68) + 0xC) & 0x3FF;
    }
    func_80052E30(saved);
done:
    ret = func_800574A8();
    func_80051980(0, a0);
    return ret;
}
