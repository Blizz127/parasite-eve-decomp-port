/* VRAM 0x800C8D34 / file 0xB9534 / size 0x110. */
extern unsigned char D_800E0A50[];
extern unsigned char D_800E22E8;
extern unsigned char D_800E22E9;
extern unsigned char D_800E22EA;
extern unsigned char D_800E22EC;
extern unsigned char D_800E22ED;
extern unsigned char D_800E22EE;
extern unsigned short D_800E22F0;
extern unsigned short D_800E22F2;
extern unsigned char D_800E2328;
extern unsigned char D_800E2329;
extern unsigned char D_800E232A;
extern unsigned char D_800E232C;
extern unsigned char D_800E232D;
extern unsigned char D_800E232E;
extern unsigned short D_800E2330;
extern unsigned short D_800E2332;
extern unsigned char D_800F34A8;
extern unsigned char D_800F34A9;
extern unsigned char D_800F34AA;
extern unsigned char D_800F34AC;
extern unsigned char D_800F34AD;
extern unsigned char D_800F34AE;
extern short D_800F34B0;
extern unsigned short D_800F34B2;

extern void *func_800C22F8(void);

int func_800C8D34(void) {
    *(unsigned char **)func_800C22F8() = D_800E0A50;
    D_800E22EC = 0xBD;
    D_800E22ED = 9;
    D_800E22F0 = 0;
    D_800E22F2 = 0x80;
    D_800E22E8 = 0x80;
    D_800E22E9 = 0x80;
    D_800E22EA = 0x80;
    D_800F34AC = 0xAE;
    D_800F34AD = 7;
    D_800F34B0 = -0x32;
    D_800E22EE = 0;
    D_800F34B2 = 0x80;
    D_800F34A8 = 0x50;
    D_800F34A9 = 0x50;
    D_800F34AA = 0x50;
    D_800F34AE = 0;
    D_800E232C = 0x68;
    D_800E232D = 0;
    D_800E2330 = 0;
    D_800E2332 = 0x80;
    D_800E2328 = 0x50;
    D_800E2329 = 0x50;
    D_800E232A = 0x50;
    D_800E232E = 0;
    return 0;
}
