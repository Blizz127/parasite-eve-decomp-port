/*
 * Retail addresses of routines that cc1 / the C front end call by their
 * library name rather than an address-named symbol.  cc1 2.7.2 emits the
 * soft-float helpers (`jal __floatsidf`, ...) itself, and `memset` is a
 * libc name.  The EXE link (tools/build/disc1_build.py abs_syms.ld) and the
 * leaf checks (tools/analysis/era_link_check.py, leaf_strong_check.py) read
 * this file, so a leaf that calls one links at the retail address.
 *
 * A wrong entry cannot produce a false match: the `jal` target is encoded in
 * the compared word, so a mis-aliased call is a word mismatch.
 *
 * Evidence (asm/disc1, retail SLUS_006.62):
 *   __floatsidf 0x80073454  matched leaf func_80073454 (int -> double);
 *                           called at 0x800285DC/0x800285F4 in func_80028574
 *   __muldf3    0x800735C4  called at 0x80028610/0x80028638 in func_80028574
 *   __adddf3    0x80072A74  called at 0x80028624 in func_80028574
 *   __divdf3    0x80072F64  called at 0x8002864C in func_80028574
 *   __fixdfsi   0x80073354  matched leaf func_80073354; called at 0x80028658
 *   memset      0x80071A44  BIOS stub `li $t2,0xA0; jr $t2; li $t1,0x2B`
 *                           (A(2Bh) = memset)
 * Each soft-float address has exactly one caller in the image, and every
 * one of those calls is inside func_80028574, whose LINK_EXACT result
 * checks all seven jal words (docs/evidence/func-80028574/REPORT.md).
 */
__floatsidf = 0x80073454;
__muldf3 = 0x800735C4;
__adddf3 = 0x80072A74;
__divdf3 = 0x80072F64;
__fixdfsi = 0x80073354;
memset = 0x80071A44;
