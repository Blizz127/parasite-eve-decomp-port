/* Opaque-word setter (Stage 3 batch): set D_800A1830 = 1, return 0.
 * VRAM 0x80042C50 / file 0x33450 / size 0x14.
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * Type provenance (lead opaque-word ruling): every writer/reader of
 * D_800A1830 is bare 32-bit sw/lw. Typed unsigned int (u32); refine if a
 * narrowing reader is later found. Pilot proof: func_80042C28 matched.
 */
extern unsigned int D_800A1830;

int func_80042C50(void) {
    D_800A1830 = 1;
    return 0;
}
