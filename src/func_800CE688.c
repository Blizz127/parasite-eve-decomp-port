/* Phase 7A: func_800CE688 — VRAM 0x800CE688, size 0x104, file 0xBEE88-0xBEF8C.
 * Effect-slot callback list update. Walks the `arg0->4` records of `arg0->0`
 * bytes from arg0+0xC; for each live slot (halfword at +0) it saves
 * D_800E27EC, publishes the halfword at +2, invokes the callback at arg0+8
 * with (1, slot+4, D_800E2368->8) and either clears the slot or bumps the +2
 * counter, then restores D_800E27EC. Returns the live-slot count.
 * era -O2 -G0. Levers: the 8-byte dead local reproduces retail's reserved
 * frame slot; `char *slot = arg0 + 0xC;` must carry its initializer on the
 * declaration so cc1 emits the `addiu` in the prologue (a separate assignment
 * is scheduled into the blez delay slot and swaps the whole save order); and
 * `register int stride asm("$21")` pins retail's $s5 home (with $s4 for the
 * live count) — pinning both instead makes cc1 fail. */
extern int D_800E27EC;
extern char *D_800E2368;

int func_800CE688(char *arg0)
{
    int tmp[2];
    int saved;
    char *slot = arg0 + 0xC;
    int i = 0;
    int hits = 0;
    int (*cb)(int, void *, int);
    register int stride asm("$21");

    (void)tmp;
    saved = D_800E27EC;
    cb = *(int (**)(int, void *, int))(arg0 + 0x8);
    stride = *(int *)(arg0 + 0x0);
    if (*(int *)(arg0 + 0x4) > 0) {
        char *counter = arg0 + 0xE;
        do {
            if (*(short *)slot != 0) {
                hits++;
                D_800E27EC = *(short *)counter;
                if (cb(1, slot + 4, *(int *)(D_800E2368 + 8)) != 0) {
                    *(short *)slot = 0;
                } else {
                    *(short *)counter = *(unsigned short *)counter + 1;
                }
            }
            i++;
            counter += stride;
            slot += stride;
        } while (i < *(int *)(arg0 + 0x4));
    }
    D_800E27EC = saved;
    return hits;
}
