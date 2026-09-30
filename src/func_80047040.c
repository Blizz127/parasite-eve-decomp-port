extern int D_8009CFB8;
extern unsigned char D_800922B8[];
extern unsigned char D_800922BC[];
extern unsigned char D_800922C4[];
extern unsigned char D_800922CC[];

extern void func_8005E8A4(int, int);
extern void func_8005EB64(int);

void func_80047040(int a0) {
    int m;
    int s;

    m = D_8009CFB8;
    s = a0 + D_800922B8[m];
    if (m == 1) {
        func_8005E8A4(0, 2);
        func_8005EB64(D_800922BC[s]);
        func_8005E8A4(0x12, 2);
        func_8005EB64(0x22);
        func_8005E8A4(0xC, -2);
        func_8005EB64(D_800922C4[s]);
        if (D_8009CFB8 == 0) {
            func_8005E8A4(0x12, 2);
            func_8005EB64(0x22);
            func_8005E8A4(0xC, -2);
            func_8005EB64(D_800922CC[s]);
        }
    } else {
        func_8005EB64(D_800922BC[s]);
        func_8005EB64(0x68);
        func_8005E8A4(0x12, 4);
        func_8005EB64(0x22);
        func_8005E8A4(0xC, -4);
        func_8005EB64(D_800922C4[s]);
        func_8005EB64(0x68);
        if (D_8009CFB8 == 0) {
            func_8005E8A4(0x12, 4);
            func_8005EB64(0x22);
            func_8005E8A4(0xC, -4);
            func_8005EB64(D_800922CC[s]);
            func_8005EB64(0x68);
        }
    }
}
