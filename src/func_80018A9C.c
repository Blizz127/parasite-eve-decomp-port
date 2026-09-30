typedef struct {
    char pad[0x14];
    int field_14;
} Header;

extern Header *volatile D_800B1624;
extern void func_800671C8(void *a0, int a1, int a2, int a3);

int func_80018A9C(int **a0) {
    unsigned char *p;

    p = (unsigned char *)D_800B1624 + D_800B1624->field_14 + *a0[0] * 56;
    func_800671C8(p, *(short *)a0[1], *(short *)a0[2], *(short *)a0[3]);
    return 1;
}
