typedef struct { char b[8]; } Blk8;
typedef struct { unsigned short vx, vy, vz, pad; } SVec;

extern unsigned char *D_800E27A8;
extern unsigned short D_800E2360;
extern unsigned short D_800E2362;
extern unsigned short D_800E2364;
extern unsigned char D_800C21C4[];
extern unsigned char D_800C21CC[];
extern void func_80078C34();

void func_800CAA38(int a0, int a1, unsigned char *a2) {
    Blk8 v10;
    Blk8 v18;
    SVec out;
    unsigned short *g;

    v10 = *(Blk8 *)D_800C21C4;
    v18 = *(Blk8 *)D_800C21CC;
    func_80078C34(*(unsigned char **)(D_800E27A8 + 0x238) + 0x260, &v10, &out);
    g = &D_800E2360;
    *(unsigned short *)(a2 + 8) = *g + out.vx;
    *(unsigned short *)(a2 + 0xA) = D_800E2362 + out.vy;
    {
        unsigned char *q = D_800E27A8;
        *(unsigned short *)(a2 + 0xC) = D_800E2364 + out.vz;
        func_80078C34(*(unsigned char **)(q + 0x238) + 0x260, &v18, &out);
    }
    *(unsigned short *)(a2 + 0x10) = *g + out.vx;
    *(unsigned short *)(a2 + 0x12) = D_800E2362 + out.vy;
    {
        unsigned short t = D_800E2364 + out.vz;
        *(unsigned short *)(a2 + 4) = 0x7F;
        *(unsigned short *)(a2 + 6) = 0;
        *(unsigned short *)(a2 + 0x14) = t;
    }
}
