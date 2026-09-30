typedef struct { int p0[5]; int f14; int f18; int f1C; } C;
typedef struct { int p0[0x8E]; C *f238; } B;
typedef struct { int p0[2]; B *f8; } A;

extern A *D_800F32D0;
extern void func_8006DCE4(int a0, int a1, int a2, int a3, int a4);

void func_800D3F64(int a0, int a1)
{
    short v[3];
    B *b;

    b = D_800F32D0->f8;
    v[0] = b->f238->f14;
    v[1] = b->f238->f18;
    v[2] = b->f238->f1C;
    func_8006DCE4(a0, a1, v[0], v[1], v[2]);
}
