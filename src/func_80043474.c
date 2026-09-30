int func_80043474(int a0) {
    int r;

    if (a0 < 0x80) {
        r = 1;
    } else if (a0 < 0x138) {
        r = 2;
    } else if (a0 < 0x1B0) {
        r = 3;
    } else if (a0 >= 0x218) {
        if (a0 < 0x2D3) {
            r = 5;
        } else {
            r = 6;
        }
    } else {
        r = 4;
    }
    return r;
}
