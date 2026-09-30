/* room_m0418i — func_80191B14, blob offset 0x2B2C, 0x2A4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * 16-slot orbiting spark spawner/updater; struct-array views (MEM_IN_STRUCT) let the D_8019956C/7C loads schedule above the slot stores; rand result local before the 0x260 clear. */

extern int D_8019956C;
extern int D_8019957C;
extern int func_80071A54();
extern int func_80077CF4(), func_80077DC4();

typedef struct { short h[0x140]; } HS;
#define H(off) ((HS *)a2)->h[i + (off) / 2]
#define W(off) *(int *)(a2 + i * 4 + (off))
typedef struct { short h[12]; } RS;
#define R(off) ((RS *)a2)[i].h[(off) / 2]

void func_80191B14(int a0, unsigned char *a1, unsigned char *a2)
{
    unsigned int i;

    for (i = 0; i < 16; i++) {
        if (H(0x1C0) == 0 && func_80071A54() % 3 == 0) {
            H(0x220) = func_80071A54() % 4096;
            H(0x200) = 0x80;
            H(0x1C0) = 1;
            H(0x1E0) = 0x5A;
            W(0x180) = 0;
            { int t = func_80071A54(); H(0x260) = 0; H(0x240) = t % 200 + 100; }
            H(0x260) = func_80071A54() % 1500 + 300;
            R(2) = 0;
            R(0) = D_8019956C;
            R(4) = D_8019957C;
        }
        if (H(0x1C0) == 1) {
            W(0x180) += 1;
            if (--H(0x1E0) == 0) {
                H(0x1C0) = 0;
            }
            R(0x10) = (func_80077CF4(H(0x220)) * H(0x260) >> 12) + D_8019956C;
            R(0x12) = func_80077DC4(H(0x220)) * H(0x260) >> 12;
            R(0x14) = (func_80077DC4(H(0x220)) * H(0x260) >> 12) + D_8019957C;
            R(8) = R(0) + R(0x10);
            R(0xA) = R(2) + R(0x12);
            R(0xC) = R(4) + R(0x14);
        }
    }
    if (*(short *)(a1 + 2) >= 0xB5) {
        a1[1] = 2;
    }
}
