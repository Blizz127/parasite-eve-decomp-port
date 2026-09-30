/* VRAM 0x800701B4 / file 0x609B4 / size 0x128. */
extern int D_800942E8;
extern int D_800942E4;
extern unsigned int D_800B0CD8;
extern unsigned char D_800E0EF0[];

int func_8006FC18();

int func_800701B4(void) {
    int temp_v0;
    int result;
    int n;
    unsigned char *base;
    int off_a;
    int off_b;
    int i;
    unsigned char *p;
    unsigned int j;
    unsigned char *q;

    n = 0;
    base = D_800E0EF0;
    off_a = 0x6E84;
    off_b = 0;
    i = 0xB;
loop:
    temp_v0 = func_8006FC18(i, 0, 1);
    if (temp_v0 != 0) { result = temp_v0; goto out; }
    if ((unsigned int)i < 0x16) {
        if ((unsigned int)i >= 0xB) { p = (unsigned char *)(D_800942E8 + off_b); }
        else { p = (unsigned char *)(D_800942E4 + off_a); }
        if (p[1] == 0x72) {
            j = 0x6C; q = base + 0x1B0;
            do { *(int *)q = 0; j++; q += 4; } while (j < 0x73);
            D_800B0CD8 &= 0xFFFEFFFF;
        }
        p[0]=0; p[1]=0xFF; p[2]=0xFF; p[3]=0xFF; *(int *)(p+4)=0; *(int *)(p+8)=0;
    }
    off_a += 0xA0C;
    off_b += 0x10C;
    n++;
    i++;
    if (n < 0xB) goto loop;
    result = temp_v0;
out:
    return result;
}
