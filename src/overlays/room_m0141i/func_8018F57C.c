/* room_m0141i (PE.IMG room m0141i chunk 2, VRAM 0x8018EFE8)
 * func_8018F57C — blob offset 0x594, 0x80 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Effect init at +0x108.. then func_800C4E50(p + 0x108). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_800C4E50();
void func_8018F57C(int a0, int a1, void *p)
{
    H(p, 0x120) = 1500;
    H(p, 0x122) = 255;
    H(p, 0x114) = 16;
    B(p, 0x10C) = 64;
    B(p, 0x10D) = 32;
    B(p, 0x10E) = 16;
    B(p, 0x110) = 128;
    B(p, 0x111) = 128;
    B(p, 0x112) = 128;
    H(p, 0x116) = 240;
    H(p, 0x118) = 10;
    H(p, 0x124) = 0;
    H(p, 0x11A) = 0;
    P(p, 0x108) = (char *)p + 8;
    func_800C4E50((char *)p + 0x108);
}
