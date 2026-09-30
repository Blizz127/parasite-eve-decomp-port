/* VRAM 0x80019DB8 / file 0xA5B8 / size 0x3C. */
typedef struct {
    int field_00;
    int field_04;
    int field_08;
} Record12;

extern Record12 D_800A76A4[];

int func_80019DB8(int **arg0) {
    *arg0[1] = D_800A76A4[*(int *)arg0[0]].field_00;
    return 1;
}
