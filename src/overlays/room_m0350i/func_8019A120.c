/* room_m0350i func_8019A120 — LINK_EXACT era -O2 -G0.
 * Evidence: docs/evidence/overlay-small-2026-09-25/REPORT.md. */

extern short D_8019A8B8;
short *func_8019A120(int a0, short a1)
{
    short *p = &D_8019A8B8;
    *p = a1;
    return p - 0x24;
}
