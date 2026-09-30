/*
 * func_8006EC84 — offset-table walker (VRAM 0x8006EC84, file 0x5F484,
 * 26 words / 0x68).
 *
 * Calls func_800718D0 once per entry on the absolute target address formed
 * by func_8006EC6C's rule: base + *(int *)(base + (short)i * 4). The loop
 * counter is a full `int` (signed `slt`/`blez` comparison), while the table
 * index is `short` — retail sign-extends it with the combined
 * `sll $v0,$s0,16 ; sra $v0,$v0,14` sequence, so the index is cast to short
 * at the use (same rule as the neighbouring func_8006EC6C reader).
 *
 * ROM: era gcc-2.7.2-psx -O2 -G0.
 */
extern void func_800718D0(int);

void func_8006EC84(int base, int count)
{
    int i;

    for (i = 0; i < count; i++)
        func_800718D0(base + *(int *)(base + (short)i * 4));
}
