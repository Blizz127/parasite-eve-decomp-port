/* VRAM 0x8001D268 / file 0xDA68 / size 0xD8.
 * Actor contact part check: walk the four signed part bytes at
 * record+0x7C+((record[0]>>19)&0x1C) and, when one equals the requested
 * part, raise bit 0x4000 on the actor body and mark the record
 * (bit 31). A negative part byte aborts the scan.
 * era -O2 -G0, maspsx 2.21 --dont-expand-li. All accesses are through
 * arguments, so no symbol/gp addressing is involved. */
void func_8001D268(void *actor, int own_part, void **other, int part) {
    unsigned int *record;
    unsigned char *action;
    unsigned int *body;
    int value;
    unsigned int i;

    record = (unsigned int *)*other;
    if (record == 0)
        return;
    action = (unsigned char *)record[6];
    if (action[0] != 1)
        return;
    if ((int)((record[0] >> 21) & 7) >= 3)
        return;
    if ((int)record[4] <= 0)
        return;
    i = 0;
    part = (short)part;
    for (;;) {
        value = *(signed char *)((char *)record + 0x7C + ((record[0] >> 19) & 0x1C) + (i & 0xFFFF));
        if (part == value) {
            body = (unsigned int *)*(unsigned int *)actor;
            body[0x4C / 4] |= 0x4000;
            record[0] |= 0x80000000u;
            return;
        }
        if (value < 0)
            return;
        i++;
        if ((i & 0xFFFF) >= 4)
            return;
    }
}
