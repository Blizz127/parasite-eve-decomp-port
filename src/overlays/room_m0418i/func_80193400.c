/* The two +0x50 stores are written at 0x10 then 0x14. cc1 emits 0x14 then 0x10. */
void func_80193400(int a0, unsigned char *a1, unsigned char *a2)
{
    unsigned short u;

    u = *(unsigned short *)(a2 + 0xA);
    *(unsigned short *)(a2 + 0xA) = (unsigned short)(u + 0xC8);
    if (*(short *)(a1 + 2) < 0x10) {
        *(unsigned short *)(a2 + 0x18) = (unsigned short)(*(unsigned short *)(a2 + 0x18) + 8);
        *(unsigned short *)(a2 + 2) = (unsigned short)(*(unsigned short *)(a2 + 2) + 0x100);
    } else {
        *(unsigned short *)(a2 + 0x18) = (unsigned short)(*(unsigned short *)(a2 + 0x18) - 4);
        *(unsigned short *)(a2 + 0x10) = (unsigned short)(*(unsigned short *)(a2 + 0x10) + 0x50);
        *(unsigned short *)(a2 + 0x14) = (unsigned short)(*(unsigned short *)(a2 + 0x14) + 0x50);
    }
    if (*(short *)(a2 + 0x18) <= 0) {
        a1[1] = 2;
    }
}
