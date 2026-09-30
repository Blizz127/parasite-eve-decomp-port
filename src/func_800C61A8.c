typedef struct { short vx; short vy; short vz; short pad; } SV4;

extern unsigned char D_800F3310[];
extern void func_80078C34(unsigned char *a0, unsigned char *a1, SV4 *a2);
extern int func_800C653C(void *a0, SV4 *a1);

int func_800C61A8(void *a0, unsigned char *a1) {
    SV4 v[4];

    func_80078C34(a1, D_800F3310, &v[0]);
    func_80078C34(a1, D_800F3310 + 8, &v[1]);
    func_80078C34(a1, D_800F3310 + 0x10, &v[2]);
    func_80078C34(a1, D_800F3310 + 0x18, &v[3]);
    v[0].vx += *(int *)(a1 + 0x14);
    v[1].vx += *(int *)(a1 + 0x14);
    v[2].vx += *(int *)(a1 + 0x14);
    v[3].vx += *(int *)(a1 + 0x14);
    v[0].vy = 0;
    v[1].vy = 0;
    v[2].vy = 0;
    v[3].vy = 0;
    v[0].vz += *(int *)(a1 + 0x1C);
    v[1].vz += *(int *)(a1 + 0x1C);
    v[2].vz += *(int *)(a1 + 0x1C);
    v[3].vz += *(int *)(a1 + 0x1C);
    return func_800C653C(a0, &v[0]) != 0;
}
