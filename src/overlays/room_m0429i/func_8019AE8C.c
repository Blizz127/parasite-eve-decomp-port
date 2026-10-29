/* room_m0429i — func_8019AE8C, blob offset 0xBEA4, 0x34C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * Effect callback (called through func_800D413C; m0349i's script moves to
 * this room). Mode 0 opens a 20-entry particle pool through func_800CE560
 * with the room's particle callback D_8019AC30. Mode 1, while the player
 * actor is the current one and its state word is 11 or more, takes the
 * actor position (func_800CE870) and up to 3 times (on odd rand()) spawns
 * a spark: start 110 higher each time, position scattered by
 * +-200/+-400/+-200, a random spin, life 682..1193, velocity +-12 / 0..-23
 * / +-12. Mode 2 sets the sprite globals (16x16, page from D_800E2850).
 * Matching notes: the mode-2 block follows the vendor template (room_m0089i
 * func_80192A30): volatile sprite globals, the index read first, and the
 * register fence after the table load. */
typedef struct { short vx, vy, vz, pad; } SVECTOR;

extern int *D_800F32D0;
extern int D_800942E4;
extern int *D_800F33E0;
extern volatile short D_800F3368;
extern volatile short D_800F336A;
extern volatile short D_800F336C;
extern volatile short D_800F336E;
extern volatile short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern volatile short D_800F3376;
extern volatile short D_800F3378;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern int D_8019AC30;
extern int func_80071A54();
extern int func_800CE560();
extern int func_800CE610();
extern void func_800CE870();

int func_8019AE8C(int mode)
{
    SVECTOR pos;
    short *p;
    int **e;
    int i;

    switch (mode) {
    case 0:
        return func_800CE560(D_800F33E0[2], 20, 10, &D_8019AC30);
    case 1:
        if ((int)D_800F32D0 != D_800942E4)
            return 1;
        e = (int **)D_800F32D0[2];
        if ((*e)[4] < 11)
            return 1;
        func_800CE870(e, 0, &pos);
        for (i = 0; i < 3; i++) {
            if (func_80071A54() & 1) {
                p = (short *)func_800CE610(D_800F33E0[2]);
                if (p != 0) {
                    pos.vy += 110;
                    p[0] = pos.vx;
                    p[1] = pos.vy;
                    p[2] = pos.vz;
                    p[0] += func_80071A54() % 400 - 200;
                    p[1] += func_80071A54() % 800 - 400;
                    p[2] += func_80071A54() % 400 - 200;
                    p[7] = (func_80071A54() & 0x1FF) + 682;
                    p[3] = func_80071A54();
                    p[4] = func_80071A54() % 24 - 12;
                    p[5] = -(func_80071A54() % 24);
                    p[6] = func_80071A54() % 24 - 12;
                    p[8] = 0;
                    p[9] = 0;
                }
            }
        }
        return 0;
    case 2:
    {
        int value;
        int index;
        index = D_800E11EA;
        D_800F3368 = 16;
        D_800F336A = 1;
        D_800F3376 = 16;
        D_800F3378 = 16;
        value = D_800E2850[index];
        asm("" : : "r"(value) : "$2", "memory");
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 32;
        D_800F3370 = value;
    }
        break;
    }
    return 0;
}
