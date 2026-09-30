typedef struct DB {
    unsigned char draw[0x5C];
    struct {
        short disp[4];
        short screen[4];
        unsigned char isinter;
        unsigned char isrgb24;
        unsigned char pad0;
        unsigned char pad1;
    } disp;
    int ot;
    int buf;
} DB;

extern DB D_800A2180[2];
extern int D_800B0E50[];
extern int D_800B0E54[];
extern int D_800B0E38[];
extern int D_800B0E3C[];
extern int D_8009D124;
extern int D_8009D128;
extern int *D_8009D12C;
extern int D_8009D130;
extern int D_8009D134;
extern int D_800A2270[];
extern void func_80074924(unsigned char *, int, int, int, int);
extern void func_800749D8(unsigned char *, int, int, int, int);
extern void func_8005E968(int);
extern void func_8005F844(int);

void func_8005E588(void)
{
    D_800A2180[0].buf = D_800B0E50[0];
    D_800A2180[1].buf = D_800B0E54[0];
    D_800A2180[0].ot = D_800B0E38[0];
    D_800A2180[1].ot = D_800B0E3C[0];
    func_80074924(D_800A2180[0].draw, 0, 0, 0x140, 0xE0);
    func_80074924(D_800A2180[1].draw, 0, 0xE0, 0x140, 0xE0);
    D_800A2180[1].draw[0x18] = 1;
    D_800A2180[0].draw[0x18] = 1;
    D_800A2180[0].draw[0x19] = 0;
    D_800A2180[0].draw[0x1A] = 0;
    D_800A2180[0].draw[0x1B] = 0;
    D_800A2180[1].draw[0x19] = 0;
    D_800A2180[1].draw[0x1A] = 0;
    D_800A2180[1].draw[0x1B] = 0;
    func_800749D8((unsigned char *)&D_800A2180[0].disp, 0, 0xE0, 0x140, 0xE0);
    func_800749D8((unsigned char *)&D_800A2180[1].disp, 0, 0, 0x140, 0xE0);
    {
        register char *b asm("$16");
        int color;
        b = (char *)&D_800A2180[0].buf;
        asm volatile("" : "=r"(b) : "0"(b));
        color = 0x808080;
        *(short *)(b + 0x6A) = 8;
        *(short *)(b - 0xE) = 8;
        *(short *)(b + 0x6E) = 0xE0;
        *(short *)(b - 0xA) = 0xE0;
        D_8009D128 = 0;
        D_8009D124 = 0;
        D_8009D12C = D_800A2270;
        func_8005E968(color);
    }
    D_8009D130 = 0;
    func_8005F844(0);
    D_8009D134 = 0;
}
