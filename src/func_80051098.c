typedef struct Rec {
    int w[9];
} Rec;

extern Rec *D_8009D014;
extern Rec D_800A1B30;

Rec *func_80051098(void)
{
    Rec *p;

    p = D_8009D014;
    if (p < &D_800A1B30) {
        D_8009D014 = p + 1;
        return p;
    }
    p = &D_800A1B30 - 4;
    while (p < &D_800A1B30 - 1) {
        *p = p[1];
        p++;
    }
    return p;
}
