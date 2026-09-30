/* Slot-table clear + default-record init wrapper.
 * VRAM 0x80029388 / file 0x19B88 / size 0x6C (27 words).
 * gcc-2.7.2-psx -O2 -G8 + maspsx 2.21 --dont-expand-li
 * + MASPSX_THREE_WORD_SYMBOL_STORE=1.
 *
 * jal func_8002F658, then the same 7 x 220-byte SlotRecord in-use clear
 * as func_8002F9CC (unsigned char counter, sltiu 7, 3-word symbol store),
 * then gp byte zeros D_8009D2A0 / D_8009D2EC and jal func_80020EFC.
 * Back-branch delay slot is FILLED (andi), inverse of 2F9CC's nop.
 */
typedef struct {
    unsigned int inUse;
    unsigned char body[216];
} SlotRecord;

extern SlotRecord D_800A5D58[];
extern unsigned char D_8009D2A0;
extern unsigned char D_8009D2EC;

extern void func_8002F658(void);
extern void func_80020EFC(void);

void func_80029388(void) {
    unsigned char i;

    func_8002F658();
    for (i = 0; i < 7; i++) {
        D_800A5D58[i].inUse = 0;
    }
    D_8009D2A0 = 0;
    D_8009D2EC = 0;
    func_80020EFC();
}
