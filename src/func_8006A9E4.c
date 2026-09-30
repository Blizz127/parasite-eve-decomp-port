/*
 * func_8006A9E4 - vram 0x8006A9E4, size 0x35C. Scene data load (main/Scene_LoadSceneData):
 * clears VRAM, then four CD read+poll sequences (retry on error) - common area block, the
 * 0x10A50-byte scene block (copied to D_800E2858 and its two resource tables located via
 * func_8006E498), the sound bank (func_80087090) and a 0x1400-byte block copied to *(+0x130).
 * era: cc1 2.7.2 -O2 -G0 (default profile).
 * Levers: each block is `retryN: while (read(...) == -1) {} s = 1; do { if (s == -1) goto retryN;
 * s = poll(); } while (s != 0);` (inner loops hoist the table pointer and -1); byte-aligned
 * block structs so the copies take the lwl/lwr dual path; the last poll written as
 * `while ((s = poll() != 0) != 0)` (retail tests the fresh value, then moves it to s0).
 */
typedef struct {
    short x, y, w, h;
} RECT;

typedef struct {
    unsigned char b[0x10A50];
} BigBlk;

typedef struct {
    unsigned char b[0x1400];
} SmallBlk;

extern unsigned char D_800B0CD8[];
extern int D_800B0DD8;
extern unsigned short D_800930DC[];
extern unsigned short D_800930DE[];
extern unsigned short D_800930E4[];
extern unsigned short D_800930E6[];
extern unsigned char D_800A8028[];
extern BigBlk D_800E2858;
extern int D_800B0E6C;
extern void func_80074F44(RECT *, int, int, int);
extern int func_8006E6A8(int, void *, int);
extern int func_8006E7E8(void);
extern void func_800527C8(void);
extern int func_8006E498(void *, unsigned int);
extern void func_80087090(int, int);

void func_8006A9E4(void)
{
    RECT r;
    int base;
    unsigned char *st;
    int s;
    int once;

    r.w = 0x3FF;
    st = D_800B0CD8;
    base = D_800B0DD8;
    r.x = 0;
    r.y = 0;
    r.h = 0x1FF;
    func_80074F44(&r, 0, 0, 1);
retry1:
    while (func_8006E6A8(base + D_800930DC[0], D_800A8028, D_800930DC[1] - D_800930DC[0]) == -1) {
    }
    s = 1;
    do {
        if (s == -1) {
            goto retry1;
        }
        s = func_8006E7E8();
    } while (s != 0);
    once = 0;
retry2:
    while (func_8006E6A8(base + D_800930DE[0], *(void **)(st + 0x194), D_800930DE[1] - D_800930DE[0]) == -1) {
    }
    s = 1;
    do {
        if (once == 0) {
            func_800527C8();
            once = 1;
        }
        if (s == -1) {
            goto retry2;
        }
        s = func_8006E7E8();
    } while (s != 0);
    D_800E2858 = **(BigBlk **)(st + 0x194);
    *(BigBlk **)(st + 0x148) = &D_800E2858;
    *(int *)(st + 0x140) = func_8006E498(&D_800E2858, 0x57D40D84);
    *(int *)(st + 0x144) = func_8006E498(*(void **)(st + 0x148), 0x57D41D84);
retry3:
    while (func_8006E6A8(base + D_800930E4[0], *(void **)(st + 0x194), D_800930E4[1] - D_800930E4[0]) == -1) {
    }
    s = 1;
    do {
        if (s == -1) {
            goto retry3;
        }
        s = func_8006E7E8() != 0;
    } while (s != 0);
    func_80087090(D_800B0E6C, 1);
retry4:
    while (func_8006E6A8(base + D_800930E6[0], *(void **)(st + 0x194), D_800930E6[1] - D_800930E6[0]) == -1) {
    }
    s = 1;
    do {
        if (s == -1) {
            goto retry4;
        }
    } while ((s = func_8006E7E8() != 0) != 0);
    **(SmallBlk **)(st + 0x130) = **(SmallBlk **)(st + 0x194);
}
