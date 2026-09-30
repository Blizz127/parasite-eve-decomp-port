/* VRAM 0x800254BC / file 0x15CBC / size 0xAC.
 * Command dispatch used by the battle-results teardown.  Unless
 * D_8009D1A0 bit 1 is set, latch the actor pointer (D_8009D254[0]) into
 * D_8009D278 and play effect 0x453 at the actor's position
 * (three sign-extended 16-bit offsets 0x2A/0x2E/0x32).  Then command 0x197
 * runs the cancel pair and stores 13 into record byte 0x12; command 0x198
 * runs func_80021AF8.
 *
 * The direct table form (no cached `aya` local) is what makes cc1 emit
 * retail's schedule: `lw $v0,D_8009D254`, the arg `li $a0`, then the
 * dependent `lw $v1,0($v0)` in the load-delay slot, and the D_8009D278
 * store hoisted to just before the jal.
 *
 * era -O2 -G0 (all four globals absolute; no gp-relative access). */
extern unsigned int D_8009D1A0;
extern unsigned int *D_8009D254;
extern unsigned int *D_8009D278;

extern void func_8006DE80(int a, int b, int c, int d, int e);
extern void func_800218D8(void);
extern void func_800209F0(void);
extern void func_80021AF8(void);

void func_800254BC(int command)
{
    if (!(D_8009D1A0 & 2)) {
        D_8009D278 = (unsigned int *)D_8009D254[0];
        func_8006DE80(0x453, 1, ((short *)D_8009D254)[21],
                      ((short *)D_8009D254)[23], ((short *)D_8009D254)[25]);
    }
    switch (command) {
    case 0x197:
        func_800218D8();
        func_800209F0();
        *(unsigned char *)((char *)D_8009D278 + 0x12) = 13;
        break;
    case 0x198:
        func_80021AF8();
        break;
    }
}
