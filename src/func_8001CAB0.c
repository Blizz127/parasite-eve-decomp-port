int func_8001CAB0(int px, int pz, short *v, unsigned short n) {
    int x = px >> 16;
    int z = pz >> 16;
    unsigned short i;
    register int inside asm("$11");
    int xc = v[n * 4 - 3];
    int zc = v[n * 4 - 1];
    int zp;
    int xp;
    int a;
    int c;
    short r;

    i = 0;
    inside = 0;
    do {
        zp = zc;
        xp = xc;
        zc = v[i * 4 + 3];
        xc = v[i * 4 + 1];
        c = z < zc;
        if ((!c && (z < zp)) || (c && !(z < zp))) {
            c = x < xc;
            if (c && (x < xp)) {
                inside = !inside;
            } else if (c || (x < xp)) {
                a = (xp - xc) * (z - zc);
                c = zp - zc;
                if (c < 0) {
                    c *= x - xc;
                    r = a < c;
                } else {
                    c *= x - xc;
                    r = c < a;
                }
                if (r) {
                    inside = !inside;
                }
            }
        }
        i++;
    } while (i < n);
    return inside;
}
