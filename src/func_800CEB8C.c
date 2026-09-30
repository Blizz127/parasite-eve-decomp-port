/* VRAM 0x800CEB8C / file 0xBF38C / size 0x120. */
int func_80079FB4();
int func_80077CF4();
int func_80077DC4();
int func_800C6B20();

int func_800CEB8C(short *arg0, short *arg1, int arg2) {
    short v[4][4];
    register int heading asm("$16");
    register int dz asm("$6");
    int dx;
    int a;
    int b;
    int c;
    int d;

    a = arg1[0];
    b = arg1[2];
    c = arg0[2];
    d = arg0[0];
    heading = -func_80079FB4(b - c, a - d);
    dx = func_80077CF4(heading) * arg2 / 4096;
    dz = func_80077DC4(heading) * arg2 / 4096;
    v[0][0] = (unsigned short)arg0[0] - dx;
    v[0][2] = (unsigned short)arg0[2] - dz;
    v[1][0] = (unsigned short)arg0[0] + dx;
    v[1][2] = (unsigned short)arg0[2] + dz;
    v[2][0] = (unsigned short)arg1[0] - dx;
    v[2][2] = (unsigned short)arg1[2] - dz;
    v[3][0] = (unsigned short)arg1[0] + dx;
    v[3][2] = (unsigned short)arg1[2] + dz;
    return func_800C6B20(v);
}
