extern unsigned char *D_8009D254;
extern unsigned char *D_8009D20C;

int func_80014694(int **a0) {
    unsigned char *e;
    int id;
    int t;

    t = *a0[1];
    if (t == 0) {
        unsigned char *p = D_8009D254;
        if (p == 0) {
            goto notfound;
        }
        e = p;
        goto found;
    }
    id = t;
    e = D_8009D20C;
    if (e == 0) {
        goto notfound;
    }
loop:
    if (e[0xC] == id && e[0xD] == *a0[2] && (*(int *)(e + 0x98) & 0x10) == 0) {
        goto endloop;
    }
    e = *(unsigned char **)(e + 4);
    if (e != 0) {
        goto loop;
    }
endloop:
    if (e != 0) {
        goto found;
    }
notfound:
    *a0[6] = -1;
    return 1;
found:
    *a0[6] = 1;
    switch (*a0[0]) {
    case 0:
        *a0[3] = *(int *)(e + 0x28);
        *a0[4] = *(int *)(e + 0x2C);
        *a0[5] = *(int *)(e + 0x30);
        break;
    case 1:
        *a0[3] = *(int *)(e + 0x40);
        *a0[4] = *(int *)(e + 0x44);
        *a0[5] = *(int *)(e + 0x48);
        break;
    case 2:
        *a0[3] = *(int *)(e + 0x68);
        *a0[4] = *(int *)(e + 0x6C);
        *a0[5] = *(int *)(e + 0x70);
        break;
    case 3:
        *a0[3] = *(int *)(e + 0x78);
        *a0[4] = *(int *)(e + 0x7C);
        *a0[5] = *(int *)(e + 0x80);
        break;
    case 4:
        *a0[3] = *(int *)(e + 0x88);
        *a0[4] = *(int *)(e + 0x8C);
        *a0[5] = *(int *)(e + 0x90);
        break;
    case 5:
        *a0[3] = *(short *)(e + 0x38);
        *a0[4] = *(short *)(e + 0x3A);
        *a0[5] = *(short *)(e + 0x3C);
        break;
    case 6:
        *a0[3] = *(int *)(e + 0x58);
        *a0[4] = *(int *)(e + 0x5C);
        *a0[5] = *(int *)(e + 0x60);
        break;
    }
    return 1;
}
