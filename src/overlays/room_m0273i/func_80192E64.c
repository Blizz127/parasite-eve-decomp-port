/* room_m0273i — func_80192E64, blob offset 0x3E7C, 0xF0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * 6-case command switch on a2 (jtbl_8018F1CC fold); record base a0+0xC pinned to $5 (reuses the dead a1 register). */

extern int func_8005186C();
extern int func_80193114();

int func_80192E64(char *a0, char *a1, unsigned int a2, int a3, int a4, int a5)
{
    register char *b asm("$5") = a0 + 0xC;
    char *p;


    switch (a2) {
    case 0:
        *(short *)(b + 0x14) = a3;
        if (a4 != 0) {
            *(int *)(b + 0xC) = 0;
        }
        break;
    case 1:
        *(short *)(b + 0x16) = a3;
        *(short *)(b + 0x18) = a4;
        *(int *)(b + 0x10) = a5;
        break;
    case 2:
        p = *(char **)(a0 + 8);
        *(int *)a5 = func_8005186C(((*(int *)a3 - *(int *)(p + 0x28)) >> 16) * ((*(int *)a3 - *(int *)(p + 0x28)) >> 16)
                                   + ((*(int *)a4 - *(int *)(p + 0x30)) >> 16) * ((*(int *)a4 - *(int *)(p + 0x30)) >> 16));
        break;
    case 3:
        *(int *)a4 = func_80193114(*(int *)a3);
        break;
    case 4:
        *(int *)(b + 0) = a3;
        *(int *)(b + 4) = a4;
        break;
    case 5:
        *(int *)(b + 8) = a3;
        *(int *)a3 = 1;
        break;
    }
    return 0;
}
