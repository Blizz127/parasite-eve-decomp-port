extern int D_8009CF60;
extern int D_8009CF64;
extern int D_8009CF68;
extern int D_8009CF6C;
extern int D_8009CF70;
extern int D_8009CF74;
extern int D_8009CF78;
extern int D_8009CF7C;
extern int D_8009CF80;
extern int D_8009CF40;
extern int D_8009CF84;
extern unsigned char D_800C0E0A;
extern unsigned short D_800C0E06;
extern int D_800C0E10;

extern void func_8005E8A4(int, int);
extern void func_8005EB64(int);
extern void func_80055668();
extern void func_8005E968(int);
extern void func_8006055C(int);
extern void func_800437B4(int);
extern void func_80052764();

void func_8004BF40(void) {
    int bumped;
    register int cur asm("$4");
    int cur3;
    register int t1 asm("$2");
    register int u1 asm("$3");
    int x3;
    register int u asm("$2");
    register int t asm("$3");
    int col;
    int i;

    func_8005E8A4(4, 4);
    func_8005EB64(0x97);
    bumped = 0;
    cur = D_8009CF60;
    if (cur < D_8009CF6C) {
        t1 = D_8009CF78;
        u1 = t1 + 1;
        D_8009CF78 = u1;
        bumped = 1;
        if (t1 >= 0x1E) {
            D_8009CF78 = 0;
            D_8009CF60 = cur + 1;
            func_80055668();
        }
    }
    col = 0x808080;
    if (D_800C0E0A < D_8009CF60) {
        col = 0x8080;
    }
    func_8005E968(col);
    func_8005E8A4(0xF, 0x10);
    func_8006055C(D_8009CF60 + 1);
    func_8005E968(0x808080);
    func_8005E8A4(-0x3C, 0x14);
    func_8005EB64(0x99);
    cur = D_8009CF64;
    if (cur < D_8009CF70) {
        t = D_8009CF7C;
        D_8009CF7C = t + 1;
        bumped = 1;
        if (t >= 0) {
            u = cur + 1;
            D_8009CF7C = 0;
            D_8009CF64 = u;
        }
    }
    col = 0x808080;
    if (D_800C0E06 < D_8009CF64) {
        col = 0x8080;
    }
    func_8005E968(col);
    func_8005E8A4(0xF, 0x10);
    func_8006055C(D_8009CF64);
    func_8005E8A4(-0x3C, 0x13);
    func_8005E968(0x808080);
    func_8005EB64(0x93);
    cur3 = D_8009CF68;
    if (cur3 < D_8009CF74) {
        D_8009CF68 = cur3 + 1;
        bumped = 1;
    }
    x3 = D_8009CF68;
    col = 0x808080;
    if (D_800C0E10 < x3) {
        col = 0x8080;
    }
    func_8005E968(col);
    func_8005E8A4(0xF, 0x11);
    func_8006055C(D_8009CF68);
    func_8005E8A4(0xA, -0x56);
    func_8005E968(0x808080);
    if (D_8009CF80 != 0) {
        t = D_8009CF40;
        if (t < 0x80) {
            u = t + D_8009CF84;
            D_8009CF40 = u;
        }
        if (bumped == 0) {
            if (D_8009CF40 >= 0x80) {
                D_8009CF40 = 0x80;
                D_8009CF80 = 0;
            }
        }
    }
    func_8005E8A4(0x78, 2);
    i = 1;
    do {
        func_800437B4(i);
        func_8005E8A4(0, 0x10);
        i++;
    } while (i < 7);
    if (D_8009CF80 == 0) {
        func_80052764();
    }
}
