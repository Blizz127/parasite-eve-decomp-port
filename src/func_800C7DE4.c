typedef struct { int p0[0x9D]; int f274; int f278; int f27C; } C;
typedef struct { int p0[0x8E]; C *volatile f238; } B;
typedef struct { int p0[2]; B *f8; } A;

extern volatile unsigned short D_800E2348;
extern volatile unsigned short D_800E234A;
extern volatile unsigned short D_800E234C;
extern B *D_800E279C;
extern void func_800CEDA8(int a0);

void func_800C7DE4(A *a0)
{
    B *b;

    b = a0->f8;
    D_800E2348 = b->f238->f274;
    D_800E234A = b->f238->f278;
    D_800E234C = b->f238->f27C;
    D_800E279C = b;
    func_800CEDA8(0);
}
