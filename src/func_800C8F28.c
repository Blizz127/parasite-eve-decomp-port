typedef struct { int p0[0x9D]; int f274; int f278; int f27C; } C;
typedef struct { int p0[0x8E]; C *volatile f238; } B;
typedef struct { int p0[2]; B *f8; } A;

extern volatile unsigned short D_800E2350;
extern volatile unsigned short D_800E2352;
extern volatile unsigned short D_800E2354;
extern B *D_800E27A0;
extern void func_800CEDA8(int a0);

void func_800C8F28(A *a0)
{
    B *b;

    b = a0->f8;
    D_800E2350 = b->f238->f274;
    D_800E2352 = b->f238->f278;
    D_800E2354 = b->f238->f27C;
    D_800E27A0 = b;
    func_800CEDA8(0);
}
