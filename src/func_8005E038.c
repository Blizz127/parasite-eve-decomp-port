/* func_8005E038 — VRAM 0x8005E038, size 0xDC, file 0x4E838-0x4E914.
 *
 * Menu pad-state decoder: expands the 14 relevant bits of the raw pad word
 * D_8009D26C into a bitmask at the guest's own bit positions (bit 12 from
 * (pad<<9), then 0x20/0x40/0x10/0x20000000/0x40000000/0x10000000/sign/
 * 0x04000000/0x08000000/0x01000000/0x02000000/0x2/0x4).
 *
 * era -O2 -G0.  cc1 colours the loaded pad into $a0 and, at the point where
 * the middle group starts, keeps a second live copy in $a1 because the
 * original pad is still needed for the final `& 4` test; the source models
 * that with `w = v`.  The two AND results must land in $v0 rather than being
 * coalesced with their sources, so they are pinned through `y`.
 */
extern int D_8009D26C;

int func_8005E038(void) {
    register int v asm("$4");
    register int w asm("$5");
    register int y asm("$2");
    int r;

    v = D_8009D26C;
    r = (v << 9) & 0x1000;
    if (v & 0x20) r |= 0x4000;
    if (v & 0x40) r |= 0x8000;
    if (v & 0x10) r |= 0x2000;
    if (v & 0x20000000) r |= 0x20;
    if (v & 0x40000000) r |= 0x40;
    if (v & 0x10000000) r |= 0x10;
    if (v < 0) r |= 0x80;
    if (v & 0x04000000) r |= 4;
    if (v & 0x08000000) r |= 8;
    w = v;
    if (w & 0x01000000) r |= 1;
    if (w & 0x02000000) r |= 2;
    y = w & 2;
    if (y) r |= 0x100;
    y = v & 4;
    if (y) r |= 0x800;
    return r;
}
