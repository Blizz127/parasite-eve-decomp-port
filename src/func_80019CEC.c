/*
 * func_80019CEC — promote three halfwords to the high half of three words
 * (VRAM 0x80019CEC, file 0xA4EC, 14 words / 0x38).
 *
 * o = D_8009D2F0; o->w28 = o->h21C << 16; o->w2C = o->h21E << 16;
 * o->w30 = o->h220 << 16; return 1.
 *
 * The source order is load-bearing: writing 28 then 2C then 30 reproduces
 * retail's `lh 0x21C` / `lh 0x21E` / `sw 0x30` interleave. The natural
 * 28/30/2C order interleaves the loads and stores 4 words differently.
 *
 * ROM: era gcc-2.7.2-psx -O2 -G0.
 */
typedef struct {
    unsigned char pad_00[0x28];
    int w28;
    int w2C;
    int w30;
    unsigned char pad_34[0x21C - 0x34];
    short h21C;
    short h21E;
    short h220;
} Obj;

extern Obj *D_8009D2F0;

int func_80019CEC(void) {
    Obj *o = D_8009D2F0;

    o->w28 = o->h21C << 16;
    o->w2C = o->h21E << 16;
    o->w30 = o->h220 << 16;
    return 1;
}
