/*
 * func_8006F39C — command-record allocator + dispatch (VRAM 0x8006F39C,
 * file 0x5FB9C, 206 words / 0x338).
 *
 * id >= 0xC0 -> -7.  For ids 0x6C..0x72, when D_800B0CD8 bit 0x10000 is
 * clear, loads D_80093162 (dest D_80011618) through func_8006E6A8,
 * polls func_8006E7E8 (-1 restarts the read), installs the seven
 * D_800E10A0 handler pointers and sets bit 0x10000.  Then func_8006914C(0),
 * clamps id to 0x55 and looks the handler up in D_800942E0.  Finds a free
 * slot across the two arenas (inlined func_8006F224), returns -3 when full,
 * -1 for a bad slot, and otherwise formats the record (state 1, handler
 * byte = original id, +4 = 0, +8 = arg), calls func_800CE49C for id 0x55,
 * dispatches the handler field at +4 and returns the slot.
 *
 * ROM: era gcc-2.7.2-psx -O2 -G0.
 */
extern int D_800B0DD8;              /* mount base (written by 698D4) */
extern unsigned int D_800B0CD8;     /* flags word */
extern unsigned char *D_80011618;   /* read buffer pointer */
extern unsigned short D_80093162[]; /* offset/size pair table */
extern unsigned int D_800E10A0[];   /* handler-pointer table (ids 0..6) */
extern unsigned char *D_800942E4;   /* arena, stride 0xA0C, ids 0..0xA */
extern unsigned char *D_800942E8;   /* arena, stride 0x10C, ids 0xB..0x15 */
extern void **D_800942E0;           /* handler-pointer table by id */

/* Overlay handler descriptors installed into D_800E10A0[0..6]. */
extern unsigned char D_801F1BD8[], D_801F1C58[], D_801F1D00[], D_801F1D8C[],
    D_801F1E18[], D_801F1EA4[], D_801F1EF0[];

extern int func_8006E6A8(int lba, unsigned char *dest, int sectors);
extern int func_8006E7E8(void);
extern void func_8006914C(int a);
extern void func_80072714(void);
extern void func_800726C4(void);
extern void func_80072724(void);
extern void func_800CE49C(void *p, int a);

int func_8006F39C(int id, int arg) {
    unsigned char *p;
    unsigned short *tbl;
    unsigned char *q;
    unsigned int *flagsPtr;
    int slot = 0;
    int result;
    int s1;
    int r;
    int i;
    int hbyte;

    if ((unsigned int)id >= 0xC0)
        return -7;

    if ((unsigned int)(id - 0x6C) < 7) {
        if (!(D_800B0CD8 & 0x10000)) {
        retry:
            tbl = D_80093162;
            do {
                r = func_8006E6A8(D_800B0DD8 + tbl[0], D_80011618,
                                  tbl[1] - tbl[0]);
            } while (r == -1);
            for (;;) {
                r = func_8006E7E8();
                if (r == 0)
                    break;
                if (r == -1)
                    goto retry;
            }
            func_80072714();
            func_800726C4();
            func_80072724();
            D_800E10A0[0] = (unsigned int)D_801F1BD8;
            D_800E10A0[1] = (unsigned int)D_801F1C58;
            D_800E10A0[2] = (unsigned int)D_801F1D00;
            D_800E10A0[3] = (unsigned int)D_801F1D8C;
            D_800E10A0[4] = (unsigned int)D_801F1E18;
            D_800E10A0[5] = (unsigned int)D_801F1EA4;
            flagsPtr = &D_800B0CD8;
            D_800E10A0[6] = (unsigned int)D_801F1EF0;
            *flagsPtr |= 0x10000;
        }
    }

    func_8006914C(0);

    hbyte = id;
    if ((unsigned int)id >= 0x55) {
        slot = id - 0x55;
        id = 0x55;
    }

    if (D_800942E0[id] == 0)
        return -8;
    if (*(int *)((char *)D_800942E0[id] + 4) == 0)
        return -1;

    result = -1;
    if ((unsigned int)id >= 0xC0) {
        s1 = -1;
    } else {
        if ((unsigned int)(id - 0x46) < 0xF) {
            q = D_800942E8;
            for (i = 0; i < 0xB; i++) {
                if (q[0] == 0) {
                    result = i + 0xB;
                    break;
                }
                q += 0x10C;
            }
        } else {
            q = D_800942E4;
            for (i = 0; i < 0xB; i++) {
                if (q[0] == 0) {
                    result = i;
                    break;
                }
                q += 0xA0C;
            }
        }
        s1 = result;
    }

    if (s1 == -1)
        return -3;
    if ((unsigned int)s1 >= 0x16)
        return -1;

    if (s1 >= 0xB)
        p = D_800942E8 + (s1 - 0xB) * 0x10C;
    else
        p = D_800942E4 + s1 * 0xA0C;

    p[0] = 1;
    p[1] = hbyte;
    p[2] = 0;
    p[3] = 0;
    *(int *)(p + 4) = 0;
    *(int *)(p + 8) = arg;
    if (id == 0x55)
        func_800CE49C(p, slot);
    {
        void (*fn)(void *) = *(void (**)(void *))((char *)D_800942E0[id] + 4);
        fn(p);
    }
    return s1;
}
