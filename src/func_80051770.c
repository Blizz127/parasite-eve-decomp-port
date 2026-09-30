extern unsigned char **D_8009D254;
extern short D_800C0E08;
extern signed char D_800C0E20;
extern void func_80024250(int, unsigned char **);
extern int func_80052F0C(void);
extern void func_80052E30(int);
extern short *func_8005332C(int);

void func_80051770(int a0)
{
    unsigned char *rec;
    short *r;
    int saved;

    func_80024250(a0, D_8009D254);
    if (D_8009D254 == 0) {
        return;
    }
    rec = *D_8009D254;
    if (rec == 0) {
        return;
    }
    D_800C0E08 = *(unsigned short *)(rec + 0xC);
    if (*(int *)(rec + 0x68) == 0) {
        return;
    }
    saved = func_80052F0C();
    func_80052E30(0);
    r = func_8005332C(D_800C0E20);
    if (r != 0) {
        r[5] = *(int *)(*(int *)(rec + 0x68) + 0xC) & 0x3FF;
    }
    func_80052E30(saved);
}
