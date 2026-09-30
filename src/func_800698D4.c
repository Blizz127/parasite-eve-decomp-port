/*
 * func_800698D4 -- disc mount / file search (Boot Rung 1): CD ready waits,
 * DsSearchFile for PEDISC01.IDF / PEDISC02.IDF each followed by PE.IMG,
 * setting D_800B0DCD bit 1 / bit 2.  Era -O2 -G0 (default era_o2_g0).
 *
 * Closed in lane exeF (2026-09-28).  The old 140/141 residual was the two
 * unfilled `beqz` delay slots of the second file pair.  reorg's
 * fill_slots_from_thread stops scanning the fall-through thread at an asm
 * insn (stop_search_p), so an empty `asm("")` between the `!= 0` and the
 * `!= -1` tests of searches #3/#4 leaves `li v0,-1` in place and the slot
 * gets a nop (the target thread's first insn is a 2-word `lb`), exactly as
 * retail.  Searches #1/#2 keep the stolen `li v0,-1`.
 */

extern signed char D_800B0DCD;    /* mount-status flag: |= 1, |= 2 */
extern int D_800B0DD8;              /* func_80080C48's return (opaque word) */
extern char D_80011330[];           /* "\\FMV1\\PEDISC01.IDF;1" */
extern char D_80011348[];           /* "\\PE.IMG;1" */
extern char D_80011354[];           /* "\\FMV2\\PEDISC02.IDF;1" */
extern int  func_8007F72C(void);
extern int  func_8007F778(void);    /* C */
extern int  func_80082314(void);
extern int  func_80081414(void *fp, char *name);  /* DsSearchFile (SDK) */
extern int  func_80080C48(void *fp);
extern void func_80073A44(int a);   /* VSync (SDK) */

int func_800698D4(void) {
    char local[0x18];   /* CdlFILE-shaped: pos(4) + size(4) + name(16) */
    int s0, t, v1;

    D_800B0DCD = 0;
    s0 = func_8007F72C();
    if (s0 != 1) {
        return 1;
    }
    if (func_8007F778() != 0) {
        return 1;
    }
    t = func_80082314();
    if (t == 1) {
        return 1;
    }
    if (t != 4) {
        return -1;
    }
    while (!(func_8007F72C() == 1 && func_8007F778() == 0)) {
        func_80073A44(0);
    }
    v1 = func_80081414(local, D_80011330);
    if (v1 != 0) {
      if (v1 != -1) {
        while (!(func_8007F72C() == 1 && func_8007F778() == 0)) {
            func_80073A44(0);
        }
        v1 = func_80081414(local, D_80011348);
        if (v1 != 0) {
          if (v1 != -1) {
            D_800B0DD8 = func_80080C48(local);
            D_800B0DCD |= 1;
          }
        }
      }
    }
    while (!(func_8007F72C() == 1 && func_8007F778() == 0)) {
        func_80073A44(0);
    }
    v1 = func_80081414(local, D_80011354);
    if (v1 != 0) {
      asm(""); if (v1 != -1) {
        while (!(func_8007F72C() == 1 && func_8007F778() == 0)) {
            func_80073A44(0);
        }
        v1 = func_80081414(local, D_80011348);
        if (v1 != 0) {
          asm(""); if (v1 != -1) {
            D_800B0DD8 = func_80080C48(local);
            D_800B0DCD |= 2;
          }
        }
      }
    }
    return (D_800B0DCD == 0) ? -2 : 0;
}
