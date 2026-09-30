/* room_m0418i func_80192788 — LINK_EXACT era -O2 -G0.
 * Evidence: docs/evidence/overlay-small-2026-09-25/REPORT.md. */

void func_80192788(int a0, int a1, unsigned char *a2)
{
    unsigned int i = 0;
    do {
        *(short *)(a2 + 0x1C0) = 0;
        i += 1;
        a2 += 2;
    } while (i < 0x10);
}
