/* room_m0005i (PE.IMG room m0005i chunk 2, VRAM 0x8018EFE8)
 * func_80190A6C — blob offset 0x1A84, 0x2C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * When a0 == 1 set +0x12 = 1 on *D_800E2368; return D_80190B80. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void *D_800E2368;
extern char D_80190B80[];
char *func_80190A6C(int a0)
{
    if (a0 == 1) {
        H(D_800E2368, 0x12) = 1;
    }
    return D_80190B80;
}
