typedef struct {
    unsigned int f0;
    unsigned int flags;
    unsigned int f8;
    unsigned int fC;
    unsigned int f10;
    unsigned int f14;
    unsigned int f18;
    unsigned short f1C;
    unsigned short f1E;
    unsigned short f20;
    unsigned short f22;
    unsigned short f24;
    unsigned short f26;
    short f28;
    short f2A;
} VoiceRegs;

extern void func_800877BC();
extern void func_80087798();
extern void func_800877D4();
extern void func_800877F0();
extern void func_8008788C();
extern void func_8008780C();
extern void func_800878C0();
extern void func_8008783C();
extern void func_80087864();

void func_800878F0(int a0, VoiceRegs *a1)
{
    unsigned int fl;

    fl = a1->flags;
    if (fl == 0) {
        return;
    }
    if (fl & 0x10) {
        func_800877BC(a0, a1->f1C);
        if ((a1->flags &= ~0x10) == 0) {
            return;
        }
    }
    if (fl & 3) {
        func_80087798(a0, a1->f28, a1->f2A);
        if ((a1->flags &= ~3) == 0) {
            return;
        }
    }
    if (fl & 0x80) {
        func_800877D4(a0, a1->f8);
        if ((a1->flags &= ~0x80) == 0) {
            return;
        }
    }
    if (fl & 0x10000) {
        func_800877F0(a0, a1->fC);
        if ((a1->flags &= ~0x10000) == 0) {
            return;
        }
    }
    if (fl & 0x2200) {
        func_8008788C(a0, a1->f24, a1->f14);
        if ((a1->flags &= ~0x2200) == 0) {
            return;
        }
    }
    if (fl & 0x900) {
        func_8008780C(a0, a1->f1E, a1->f10);
        if ((a1->flags &= ~0x900) == 0) {
            return;
        }
    }
    if (fl & 0x4400) {
        func_800878C0(a0, a1->f26, a1->f18);
        if ((a1->flags &= ~0x4400) == 0) {
            return;
        }
    }
    if (fl & 0x9000) {
        func_8008783C(a0, a1->f20);
        func_80087864(a0, a1->f22);
    }
    a1->flags = 0;
}
