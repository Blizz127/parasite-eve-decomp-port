typedef struct {
    char b[0x12E4];
} SaveBlk;

typedef struct {
    char b[4];
} U4;

extern SaveBlk D_8009EFD0;
extern SaveBlk D_800C0DE0;
extern unsigned char *D_800A0ED0;
extern unsigned char D_8009EED0[];
extern int D_800A185C;

extern void func_8003FBD8();
extern void func_8005C374();
extern void func_8004D9D8();
extern void func_8004CC50(int, int);
extern void func_8004D024(void (*)());
extern void func_8004CE28(int, int);
extern void func_80042228();

void func_80042264(void) {
    int stored;
    unsigned char *p;
    int crc;
    unsigned short i;
    unsigned short j;
    unsigned char *tab;

    D_800A0ED0 = (unsigned char *)&D_8009EFD0;
    D_800C0DE0 = D_8009EFD0;
    D_800A0ED0 = D_800A0ED0 + 0x12E4;
    func_8003FBD8();
    tab = D_8009EED0;
    crc = 0xFFFF;
    i = 0;
    p = D_800A0ED0;
    *(U4 *)&stored = *(U4 *)p;
    D_800A0ED0 = D_800A0ED0 + 4;
    *(int *)p = 0;
    do {
        crc = crc ^ (tab[i] << 8);
        j = 0;
        do {
            if ((crc & 0x8000) != 0) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc = crc << 1;
            }
            j++;
        } while (j < 8);
        i++;
    } while (i < 0x2000);
    if ((~crc & 0xFFFF) == stored) {
        func_8005C374();
        D_800A185C = 1;
        func_8004D9D8();
        func_8004CC50(0x54, 0);
        func_8004D024(func_80042228);
    } else {
        func_8004D9D8();
        func_8004CE28(0x55, 0x56);
    }
}
