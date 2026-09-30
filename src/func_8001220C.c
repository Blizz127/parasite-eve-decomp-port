/*
 * func_8001220C — main (boot keystone). VRAM 0x8001220C, ROM 0x2A0C, 0x2EC bytes.
 * era -O2 -G0 (default profile). Init, then the mount/read/dispatch loop with the
 * A8-code state switch, the 7-code skip chain and the volume/media gate.
 * The scratchpad stack handoff (sp -> 0x1F8003FC around the overlay call
 * func_8019234C) has no C spelling and stays a fenced inline-asm block.
 */

extern void func_800725DC(void);
extern void func_8003E610(void);
extern void func_8003E680(void);
extern void func_8006A5BC(void);
extern void func_8006A64C(void);
extern void func_8006A9E4(void);
extern void func_8006AD40(void);
extern void func_8006ECEC(void);
extern void func_8006F044(void);
extern void func_8006E834(void);
extern void func_80069B08(int);
extern void func_8003F3C4(void);
extern void func_801235DC(void);
extern void func_8019234C(void);
extern int  func_801909B4(void);
extern int  func_8006E9A0(int);
extern int  func_800698D4(void);
extern void func_80073A44(int);
extern void func_80074D28(int);

typedef struct {
    unsigned int flags;
    unsigned char pad4[0xF1];
    unsigned char fF5;
} State;

extern State D_800B0CD8;
extern unsigned int D_8009D280;
extern unsigned int D_8009D1C4;
extern unsigned int D_800A7918;

void func_8001220C(void) {
    int            dispatch;
    unsigned int v;

    func_800725DC();
    dispatch = 0;
    func_8003E610();

    for (;;) {
        func_8006A5BC();

        while (func_800698D4()) {
            func_80073A44(0);
        }

        func_8006A64C();
        func_8003E680();
        func_8006A9E4();
        D_8009D280 = 0xA9400048u;

        while (1) {
            if (D_800B0CD8.flags & 0x00100000u) {
                func_80069B08(dispatch);
                D_800B0CD8.flags &= 0xFFEFFFFFu;
            }

            func_8006AD40();
            v = D_8009D280;
            D_8009D1C4 = v;

            switch (v) {
            case 0xA8000048u:
                func_8006ECEC();
                    /* === FENCED INLINE ASM ===
                     * ROM 0x2B20..0x2B44, 10 words.
                     * Re-points $sp to PS1 scratchpad top 0x1F8003FC,
                     * calls overlay 8019234C on the foreign stack, restores.
                     * Fenced, documented exception (register-pinning precedent).
                     * EXACT ROM mnemonics. */
                    __asm__ volatile (
                        "lui    $a1, 0x1F80\n\t"
                        "ori    $a1, $a1, 0x3FC\n\t"
                        "addu   $t0, $a1, $zero\n\t"
                        "sw     $sp, 0($t0)\n\t"
                        "addiu  $t0, $t0, -4\n\t"
                        "addu   $sp, $t0, $zero\n\t"
                        "jal    func_8019234C\n\t"
                        "addiu  $sp, $sp, 4\n\t"
                        "lw     $sp, 0($sp)"
                        :
                        :
                        : "$1", "$2", "$3", "$4", "$5", "$6", "$7",
                          "$8", "$9", "$10", "$11", "$12", "$13",
                          "$14", "$15", "$24", "$25", "$31",
                          "memory"
                    );
                D_800B0CD8.flags |= 0x1;
                break;
            case 0xAA108448u:
                if (!(D_800B0CD8.fF5 & 0x2)) {
                    dispatch = 2;
                    D_800B0CD8.flags |= 0x00100000u;
                } else {
                    func_8006F044();
                    func_801235DC();
                    D_8009D280 = 0xA80830C8u;
                    D_800B0CD8.flags |= 0x1;
                }
                break;
            case 0xA9400048u:
                func_8006E834();
                v = func_801909B4();
                func_8006E9A0(v);
                D_800B0CD8.flags |= 0x3;
                break;
            default:
                func_8003F3C4();
                break;
            }

            /* 7-constant skip chain */
            if (D_8009D280 == 0xA80651C8u) goto chk;
            if (D_8009D280 == 0xA8065248u) goto chk;
            if (D_8009D280 == 0xA80652C8u) goto chk;
            if (D_8009D280 == 0xA80660C8u) goto chk;
            if (D_8009D280 == 0xA8066148u) goto chk;
            if (D_8009D280 == 0xA80661C8u) goto chk;
            if (D_8009D280 == 0xA8066348u) goto chk;

            /* volume gate */
            if (D_800A7918 < 0x258u) {
                if (!(D_800B0CD8.fF5 & 0x1)) {
                    dispatch = 1;
                    D_800B0CD8.flags |= 0x00100000u;
                }
            } else {
                if (!(D_800B0CD8.fF5 & 0x2)) {
                    dispatch = 2;
                    D_800B0CD8.flags |= 0x00100000u;
                }
            }

        chk:
            if (D_800B0CD8.flags & 0x100) {
                func_80073A44(0);
                func_80074D28(0);
                D_800B0CD8.flags &= ~0x100u;
                break;
            }
        }
    }
}