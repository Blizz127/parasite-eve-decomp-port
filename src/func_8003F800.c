extern void *memcpy(void *, const void *, unsigned int);

extern unsigned char *D_800A0ED0;
extern unsigned char D_800A77F0[];
extern unsigned int D_8009D2E8;
extern unsigned int D_8009D280;
extern unsigned int D_8009D1A0;
extern unsigned int D_800B0CDC;
extern unsigned char D_800B0CE0[];
extern unsigned char D_800B0CE2[];
extern unsigned char D_800B0CE4[];
extern unsigned char D_800B0CE6;
extern unsigned char D_800BCFEE;
extern unsigned char D_800B8A20[];
extern unsigned char D_800B0CB0[];
extern unsigned char D_8009D1B0[];

void func_8003F800(void)
{
    memcpy(D_800A0ED0, D_800A77F0, 0x800);
    D_800A0ED0 += 0x800;
    memcpy(D_800A0ED0, &D_8009D2E8, 4);
    D_800A0ED0 += 4;
    memcpy(D_800A0ED0, &D_8009D280, 4);
    D_800A0ED0 += 4;
    memcpy(D_800A0ED0, &D_8009D1A0, 4);
    D_800A0ED0 += 4;
    memcpy(D_800A0ED0, &D_800B0CDC, 4);
    D_800A0ED0 += 4;
    memcpy(D_800A0ED0, D_800B0CE0, 2);
    D_800A0ED0 += 2;
    memcpy(D_800A0ED0, D_800B0CE2, 2);
    D_800A0ED0 += 2;
    memcpy(D_800A0ED0, D_800B0CE4, 2);
    D_800A0ED0 += 2;
    *D_800A0ED0 = D_800B0CE6;
    D_800A0ED0++;
    *D_800A0ED0 = D_800BCFEE;
    D_800A0ED0++;
    memcpy(D_800A0ED0, D_800B8A20, 0x70);
    D_800A0ED0 += 0x70;
    memcpy(D_800A0ED0, D_800B0CB0, 0x18);
    D_800A0ED0 += 0x18;
    memcpy(D_800A0ED0, D_8009D1B0, 8);
    D_800A0ED0 += 8;
}
