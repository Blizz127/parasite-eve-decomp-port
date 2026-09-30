/*
 * func_8009090C — VRAM 0x8009090C, file 0x8110C, size 0x3C.
 *
 * Indexed u16 counter bump then a small-table word load published through
 * *a0.  The pointer sum keeps the retail operand order: cc1 emits
 * `addu v1,v1,a0` only when the scaled index is the left operand of the
 * integer add (`(idx * 2) + (unsigned int)a0`), not `a0 + idx * 2`.
 * The final `sw v0,0(a0)` fills the jr delay slot naturally.
 */
int func_8009090C(unsigned char *a0)
{
    unsigned short idx;
    unsigned short *p;
    unsigned int v;

    idx = *(unsigned short *)(a0 + 0xCE);
    p = (unsigned short *)((idx * 2) + (unsigned int)a0 + 0x62);
    *p = (unsigned short)(*p + 1);
    idx = *(unsigned short *)(a0 + 0xCE);
    v = *(unsigned int *)(a0 + idx * 4 + 4);
    *(unsigned int *)a0 = v;
    return (int)v;
}
