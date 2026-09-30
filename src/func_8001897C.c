/* Record writer wrapper: gathers ten byte/short fields from the pointer table
 * at arg0 and forwards them to func_8002FA10 with D_8009D2F0 as context.
 * VRAM 0x8001897C / file 0x917C / size 0xD4. */
extern unsigned int *D_8009D2F0;

/* a6..a9 are declared signed here so cc1 emits retail's sign-extending `lb`
 * for the s8 fields; func_8002FA10 stores only the low byte, so the bits are
 * identical either way. */
void func_8002FA10(unsigned int *a0, unsigned int a1, unsigned int a2, unsigned int a3,
                   unsigned char a4, unsigned short a5, signed char a6, signed char a7,
                   signed char a8, signed char a9, unsigned char a10, unsigned char a11);

typedef struct {
    unsigned char *unk0;
    unsigned char *unk4;
    unsigned char *unk8;
    unsigned char *unkC;
    unsigned short *unk10;
    signed char *unk14;
    signed char *unk18;
    signed char *unk1C;
    signed char *unk20;
    unsigned char *unk24;
    unsigned char *unk28;
} Arg1897C;

int func_8001897C(Arg1897C *arg0) {
    func_8002FA10(D_8009D2F0, *arg0->unk0, *arg0->unk4, *arg0->unk8, *arg0->unkC,
                  *arg0->unk10, (signed char)*arg0->unk14, (signed char)*arg0->unk18,
                  (signed char)*arg0->unk1C, (signed char)*arg0->unk20, *arg0->unk24,
                  *arg0->unk28);
    return 1;
}
