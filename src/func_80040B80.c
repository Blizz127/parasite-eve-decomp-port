extern void *memcpy(void *, const void *, unsigned int);

typedef struct {
    unsigned char pad0[0x18];
    unsigned char f18;
} Obj;

extern unsigned char *D_800A0ED0;
extern int D_800A5D50;
extern unsigned char D_800B8868[];
extern unsigned char D_800B886A;
extern unsigned char D_800B886B;
extern unsigned char D_800B88C8[];
extern unsigned short D_80010F48;
extern int D_800C0DE8;
extern unsigned char D_800C0DE0[];
extern unsigned char D_8009EED0[];
extern unsigned char *func_8005DE70(void);
extern void func_80071A24(void *, int);
extern void func_80071A14(void *, void *);
extern void *func_80040210(int, int);
extern void func_8003F800(void);

int func_80040B80(Obj *o)
{
    unsigned char *src;
    unsigned int crc;
    unsigned short i;
    unsigned short j;
    int x;
    unsigned char *p;

    src = func_8005DE70();
    D_800A5D50 = 0x2000;
    func_80071A24(D_800B8868, 0x100);
    D_800B886A = 0x11;
    D_800B886B = 1;
    *(unsigned short *)D_800B8868 = D_80010F48;
    func_80071A14(D_800B8868 + 4, func_80040210(o->f18 - 0x40, D_800C0DE8));
    memcpy(D_800B88C8, src + 0x14, 0x20);
    memcpy(D_800B8868 + 0x80, src + 0x40, 0x80);
    func_80071A24(D_8009EED0, 0x2000);
    D_800A0ED0 = D_8009EED0;
    memcpy(D_800A0ED0, D_800B8868, 0x100);
    D_800A0ED0 += 0x100;
    memcpy(D_800A0ED0, D_800C0DE0, 0x12E4);
    D_800A0ED0 += 0x12E4;
    func_8003F800();
    p = D_8009EED0;
    crc = 0xFFFF;
    for (i = 0; i < 0x2000; i++) {
        crc ^= p[i] << 8;
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    x = ~crc & 0xFFFF;
    memcpy(D_800A0ED0, &x, 4);
    D_800A0ED0 += 4;
    return 0;
}
