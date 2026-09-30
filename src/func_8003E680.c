/*
 * func_8003E680 — boot subsystem-init dispatcher (Boot Rung 1, main's
 * callee). Zeroes five state globals, runs a 2000-pass hardware-poll loop,
 * registers func_8003E91C as a callback (func_80073D24 = callback
 * registration, per context), then fires the subsystem inits.
 * First leaf to pass a function pointer to a still-asm callee.
 * Global types from Stage-0 reader evidence: D_8009D1C4/D280 unsigned
 * compares (sltu, region-bound pair), D_8009D1A0 andi-0x2 flags, D_8009D250
 * opaque (passed as arg), D_8009CDDC int (x36 array-index arithmetic).
 * ROM: asm/disc1/2EE80.s @ file 0x2EE80, 53 words (0xD4 bytes), frame 0x18.
 */

extern unsigned int D_8009D1C4, D_8009D280, D_8009D1A0, D_8009D250;
extern int D_8009CDDC;
extern void func_80070D10(void);
extern void func_80070D6C(void);
extern void func_8003E974(void);
extern void func_80036DC8(void);
extern void func_80073D24();        /* callback register (asm) — called (int) and (fn-ptr) */
extern void func_8003E91C(void);    /* asm, passed by address */
extern void func_800371A4(int);     /* C */
extern void func_80029388(void);
extern void func_8005BCA8(void);    /* C */
extern void func_80068D28(void);
extern void func_800124F8(void);
extern void func_8001A890(void);
extern void func_80034F10(void);
extern void func_8006536C(void);
extern void func_80038D1C(void);    /* C */

void func_8003E680(void) {
    unsigned int i;
    D_8009D1C4 = 0;
    D_8009D280 = 0;
    D_8009D1A0 = 0;
    D_8009D250 = 0;
    D_8009CDDC = 0;
    func_80070D10();
    for (i = 0; i < 0x7D0; i++) {
        func_80070D6C();
    }
    func_8003E974();
    func_80036DC8();
    func_80073D24(0);
    func_80073D24(func_8003E91C);   /* register &func_8003E91C as callback */
    func_800371A4(0);
    func_80029388();
    func_8005BCA8();
    func_80068D28();
    func_800124F8();
    func_8001A890();
    func_80034F10();
    func_8006536C();
    func_80038D1C();
}
