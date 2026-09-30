typedef struct { char b[8]; } Blk;
typedef struct { int p0[0x8E]; char *f238; } B;

extern Blk D_800C217C;
extern B *D_800E27A0;
extern unsigned short D_800E2350;
extern unsigned short D_800E2352;
extern unsigned short D_800E2354;
extern void func_80078C34(char *a0, Blk *a1, unsigned short *a2);

void func_800C91A8(int a0, int a1, short *a2)
{
    Blk buf;
    int v1;
    int u;
    unsigned short out[4];

    buf = D_800C217C;
    func_80078C34(D_800E27A0->f238 + 0x260, &buf, out);
    *(volatile short *)&a2[4] = D_800E2350 + out[0];
    *(volatile short *)&a2[5] = D_800E2352 + out[1];
    v1 = D_800E2354;
    u = out[2];
    *(volatile short *)&a2[2] = 0x7F;
    *(volatile short *)&a2[3] = 0;
    a2[6] = v1 + u;
}
