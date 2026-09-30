typedef struct {
    unsigned char pad0[0x12];
    short f12;
    unsigned char pad1[8];
} E;

extern E D_8009EC38[];
extern unsigned char *D_800B0E38[];
extern int D_8009CDDC;
extern unsigned char D_8009D235;
extern void func_80077AC4(void *a0, void *a1);

void func_80033430(void) {
    int i;

    i = D_8009CDDC;
    D_8009EC38[i].f12 -= 2;
    func_80077AC4(D_800B0E38[i] + 0x1C, &D_8009EC38[i]);
    D_8009D235--;
}
