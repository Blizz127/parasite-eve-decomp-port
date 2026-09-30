/*
 * func_8002F7D8 — opcode 0x6F body create. Copies the 216-byte template
 * at D_800109B0 onto the stack, claims the first free 220-byte SlotRecord,
 * copies the template into the body, publishes the body pointer.
 *
 * VRAM 0x8002F7D8 / file 0x1FFD8 / size 0x198 (102 words). Non-leaf:
 * frame -0xF0, $ra at 0xE8, tmp at 0x10.
 *
 * 216-byte moves are gcc-2.7.2 aligned block copies of
 * struct { unsigned int w[54]; }: 13 x 16-byte 4-register lw/sw
 * (v0/v1/a0/a1) + addiu 0x10 in the bne delay, then a 2-word tail.
 * First-cut dst[0]=src[0] unrolls collapse to a single-$v0 walk.
 *
 * Slot-table stride/typing from 2F9CC/2F970 (unsigned char i, 220B
 * aggregate, 3-word D_800A5D58). Body base is the D_800A5D5C label,
 * not D_800A5D58+4. After the second copy, 220*i is recomputed into
 * off so lbu D_8009D2EC fills the last-sll delay (i in $a0, id in $v1).
 *
 * era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1.
 * Tail of 11718: prefix 0xE8C0, C 0x198, then existing 2F970.
 */
typedef struct {
    unsigned int inUse;
    unsigned char body[216];
} SlotRecord;

typedef struct {
    unsigned int w[54];
} Body216;

extern SlotRecord D_800A5D58[];
extern unsigned char D_800A5D5C[];
extern unsigned int D_800109B0[];
extern unsigned char D_8009D2EC;
extern unsigned char D_8009D2A0;
extern void func_8001A680(unsigned char *actor, int cmd);

void func_8002F7D8(unsigned char **out) {
    Body216 tmp;
    unsigned char i;
    unsigned char *body;
    unsigned int off;
    unsigned char id;

    tmp = *(Body216 *)D_800109B0;

    for (i = 0; i < 7; i++) {
        if (D_800A5D58[i].inUse == 0) {
            D_800A5D58[i].inUse = 1;
            *(Body216 *)(D_800A5D5C + 220 * i) = tmp;
            off = 220 * i;
            id = D_8009D2EC;
            body = D_800A5D5C + off;
            *out = body;
            D_8009D2EC = id + 1;
            body[7] = id + 1;
            *(unsigned int *)(*out + 8) = 1u << i;
            if ((*(unsigned int *)((unsigned char *)out + 0x98) & 0x2000) == 0) {
                unsigned int *slot = *(unsigned int **)out;
                slot[6] = (unsigned int)(slot + 7);
                func_8001A680((unsigned char *)out, 2);
                D_8009D2A0 = (unsigned char)(D_8009D2A0 + 1);
            }
            return;
        }
    }
}
