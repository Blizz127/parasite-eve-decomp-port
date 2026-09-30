/* Phase 5FJ: Boot display init + pointer arena + poll loop + dispatch exit.
 * VRAM 0x8006E9A0 / file 0x5F1A0 / size 0x234 (141 words).
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * Register bindings (evidence-backed, per standing policy 7):
 *   - The arena cursors/constants use the exact func_8006A8D4 convention:
 *     a plain-local draft allocates the two cursors to $a0/$a1 and the
 *     retained constants to $v0/$v1; retail is the opposite.  The pins
 *     below preserve retail's $v0/$v1 cursor and $a0/$a1 constant
 *     allocation proven in 5EM.
 *   - saved_arg is pinned to $16 ($s0) and lookup to $18 ($s2): the same
 *     draft allocates saved_arg to $s2 and lookup to $s0, which also sinks
 *     the &D_800B0E38 materialization past func_8005E588/func_80066B60.
 *     Retail materializes lookup before the arena (words 29-30), saves
 *     $s0 first in the prologue, and compares $s0 in the dispatch exit;
 *     the pins reproduce that allocation.  The empty asm barrier after the
 *     lookup assignment stops the -O2 pre-RA scheduler from sinking the
 *     $s2 lui/addiu pair past func_8005E588 (proven: without it the pair
 *     lands after the first post-arena jal; with it, retail position).
 *     It emits no code.  Statements still describe the same semantics.
 */

/* --- SDK callees (docs/ai_context/sdk_map.md) --- */
extern void func_80073A44(int mode);        /* VSync */
extern void func_80074D28(int mask);        /* SetDispMask */
extern void func_800755F0(void *env);       /* PutDispEnv */
extern void func_80074F44(void *rect, unsigned char r,
                          unsigned char g, unsigned char b); /* ClearImage */
extern void func_80074DC0(int mode);        /* DrawSync */
extern void func_800752AC(void *otag, int n); /* ClearOTagR */

/* --- Game callees (call-site evidence: nop delay slots, no arg loads,
 *     except func_80066B60 whose jal delay slot loads $a0 = 2) --- */
extern void func_8005E588(void);
extern void func_80066B60(int a);
extern void func_80068E24(void);
extern void func_80070E54(void);
extern int  func_80038D1C(void);            /* matched C (src/func_80038D1C.c);
                                               return ignored here */

/* --- Globals --- */
extern unsigned char  D_800BCE80;           /* opaque DISP_ENV storage (PutDispEnv) */
extern unsigned char  D_800BCFEE;           /* retail lbu + andi 3 poll byte */
extern unsigned char  D_800B0DC6;           /* retail sb zero */
extern unsigned int   D_8009D280;           /* typed by src/func_8003E680.c */
extern int            D_8009CDDC;           /* typed by src/func_8003E680.c */
extern unsigned char *D_80011614;           /* typed by src/func_8006A8D4.c */

/* Arena pointer globals — exact types from src/func_8006A8D4.c */
extern unsigned char *D_800B0E24;
extern unsigned char *D_800B0E28;
extern unsigned char *D_800B0E2C;
extern unsigned char *D_800B0E30;
extern unsigned char *D_800B0E34;
extern unsigned char *D_800B0E38;
extern unsigned char *D_800B0E3C;
extern unsigned char *D_800B0E40;
extern unsigned char *D_800B0E44;
extern unsigned char *D_800B0E48;
extern unsigned char *D_800B0E4C;
extern unsigned char *D_800B0E50;
extern unsigned char *D_800B0E54;
extern unsigned char *D_800B0E58;
extern unsigned char *D_800B0E5C;
extern unsigned char *D_800B0E60;
extern unsigned char *D_800B0E64;
extern unsigned char *D_800B0E68;
extern unsigned char *D_800B0E6C;

extern unsigned char D_800F34F8;
extern unsigned char D_8010BD00;
extern unsigned char D_80120D08;
extern unsigned char D_801ED800;

int func_8006E9A0(int arg) {
    register unsigned char *cursor asm("$2");   /* $v0 arena cursor (5EM) */
    register unsigned char *next   asm("$3");   /* $v1 arena cursor (5EM) */
    register unsigned int step_8000  asm("$4"); /* $a0 retained 0x8000  (5EM) */
    register unsigned int step_48000 asm("$5"); /* $a1 retained 0x48000 (5EM) */
    register int saved_arg asm("$16");          /* $s0 input argument */
    register unsigned char **lookup asm("$18"); /* $s2 = &D_800B0E38 */

    int one = 1;                                /* $s1, hoisted poll constant */
    unsigned short rect[4];

    saved_arg = arg;

    /* 1. Display init */
    func_80073A44(0);                           /* VSync(0) */
    func_80074D28(0);                           /* SetDispMask(0) */
    func_800755F0(&D_800BCE80);                 /* PutDispEnv */

    rect[0] = 0;                                /* x */
    rect[1] = 0;                                /* y */
    rect[2] = 0x140;                            /* w */
    rect[3] = 0x1C0;                            /* h */
    func_80074F44(rect, 0, 0, 1);               /* ClearImage */

    func_80074DC0(0);                           /* DrawSync(0) */

    /* 2. Pointer arena — retail store/compute interleave (same as 6A8D4) */
    step_48000 = 0x48000;
    lookup = &D_800B0E38;
    /* Scheduling barrier (see header): keeps the $s2 materialization at
     * retail position; emits no code. */
    asm volatile("" : : "r"(lookup) : "memory");

    cursor = &D_800F34F8;
    next = cursor + 0x1800;
    D_800B0E24 = cursor;
    cursor += 0x6000;
    D_800B0E28 = next;
    next = (unsigned char *)0xE000;
    D_800B0E2C = cursor;
    cursor += (unsigned int)next;
    D_800B0E30 = cursor;

    cursor = &D_8010BD00;
    next = &D_80120D08;
    D_800B0E40 = cursor;
    cursor = next + 0x1C98;
    D_800B0E34 = next;
    next += 0x5C98;
    step_8000 = 0x8000;
    D_800B0E38 = cursor;
    cursor += step_8000;
    D_800B0E3C = next;
    next = cursor + 0x2400;
    D_800B0E44 = cursor;
    cursor += 0x4800;
    D_800B0E4C = cursor;
    cursor += step_48000;
    D_800B0E48 = next;
    next = cursor + 0x4000;
    D_800B0E50 = cursor;
    cursor += step_8000;
    D_800B0E54 = next;
    next = cursor + 0x3800;
    D_800B0E5C = next;
    next = D_80011614;
    D_800B0E58 = cursor;
    cursor += 0x7000;
    D_800B0E60 = cursor;

    cursor = &D_801ED800;
    D_800B0E6C = cursor;
    cursor = next - 8;
    D_800B0E64 = cursor;
    D_800B0E68 = next;

    /* 3. Post-arena calls */
    func_8005E588();
    func_80066B60(2);

    /* 4. Poll loop until (D_800BCFEE & 3) == 1 */
    do {
        func_800752AC(lookup[D_8009CDDC], 0x1000);  /* ClearOTagR */
        func_80068E24();
        func_80070E54();
    } while ((D_800BCFEE & 3) != one);

    /* 5. Post-loop */
    D_800B0DC6 = 0;
    func_80038D1C();

    /* 6. Dispatch exit */
    if (saved_arg == 1) {
        D_8009D280 = 0xA80830C8;
    } else if (saved_arg == 3) {
        D_8009D280 = 0xA80651C8;
    }
    return 0;
}
