/* room_m0418i func_80192D70 — LINK_EXACT era -O2 -G0.
 * Evidence: docs/evidence/overlay-small-2026-09-25/REPORT.md. */

void func_80192D70(int a0, int a1, unsigned char *a2)
{
    unsigned char *end = a2 + 0x20;
    do {
        *a2++ = 0;
    } while (a2 < end);
}
