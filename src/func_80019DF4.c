/* VRAM 0x80019DF4 / file 0xA5F4 / size 0x110. */
struct S {
    int *src;
    int *dst0;
    int *dst1;
    int *dst2;
};

int func_80019DF4(struct S *p) {
    *p->dst0 = *p->src / 216000;
    *p->dst1 = *p->src % 216000 / 3600;
    *p->dst2 = *p->src % 3600 / 60;
    return 1;
}
