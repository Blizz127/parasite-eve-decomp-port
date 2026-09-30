/* VRAM 0x800362B8 / file 0x26AB8 / size 0x13C. */
typedef struct {
    unsigned int count;
    unsigned char *ptr;
} Slot;

extern Slot D_800A7620[16];
extern unsigned char *D_800B0E4C[];

unsigned char *func_800362B8(unsigned int size) {
    unsigned char n;
    unsigned char i;
    unsigned int j;
    int ok;

    n = 8;
    if (size < 0x47E1) {
        n = 1;
    } else if (size <= 0x8FC0) {
        n = 2;
    } else if (size <= 0x11F80) {
        n = 4;
    }
    for (i = 0; i < 16;) {
        if (D_800A7620[i].ptr == 0) {
            ok = 1;
            for (j = i; j < i + n; j++) {
                if (D_800A7620[j].ptr != 0) {
                    ok = 0;
                    i += n;
                    break;
                }
            }
            if (ok) {
                D_800A7620[i].count = n;
                D_800A7620[i].ptr = D_800B0E4C[0] + i * 0x47E0;
                return D_800A7620[i].ptr;
            }
        } else if (n < D_800A7620[i].count) {
            i += D_800A7620[i].count;
        } else {
            i += n;
        }
    }
    return 0;
}
