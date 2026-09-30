typedef struct {
    unsigned int f0;
    unsigned char pad4[0xFC];
    int f100;
} Sys;

typedef struct {
    int addr;
    int f4;
} Req;

extern unsigned int D_800B0CD8[];
extern unsigned short D_8009315E[];
extern int D_8001160C[];

extern void func_80074DC0(int a0);
extern void func_80073A44(int a0);
extern void func_80074D28(int a0);
extern void func_80074A44(int a0);
extern int func_8006E6A8(int a0, int a1, int a2);
extern int func_8006E7E8(void);
extern void func_80072714(void);
extern void func_800726C4(void);
extern void func_80072724(void);
extern void func_801216C4(int a0, Req *a1);
extern void func_80121C04(int a0);
extern void func_801223A8(int a0);

int func_80014E30(short **a0) {
    Sys *s = (Sys *)D_800B0CD8;
    Req req;
    int r;

    s->f0 |= 0x8200;
    func_80074DC0(0);
    func_80073A44(0);
    func_80074D28(0);
    func_80074A44(1);
retry1:
    while (func_8006E6A8(s->f100 + D_8009315E[0], D_8001160C[0], D_8009315E[1] - D_8009315E[0]) == -1) {
    }
    while ((r = func_8006E7E8()) != 0) {
        if (r == -1) {
            goto retry1;
        }
    }
    func_80072714();
    func_800726C4();
    func_80072724();
retry2:
    while (func_8006E6A8(s->f100 + D_8009315E[1], D_8001160C[1], D_8009315E[2] - D_8009315E[1]) == -1) {
    }
    while ((r = func_8006E7E8()) != 0) {
        if (r == -1) {
            goto retry2;
        }
    }
    func_80072714();
    func_800726C4();
    func_80072724();
    req.f4 = 0;
    req.addr = D_8001160C[1] + ((D_8009315E[2] - D_8009315E[1]) << 11);
    func_801216C4(1, &req);
    func_80121C04(**a0);
    func_801223A8(1);
    return 1;
}
