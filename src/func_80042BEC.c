/* Opaque-word setter (Stage 3 batch): set D_800A1824 = 1, return 0.
 * VRAM 0x80042BEC / file 0x333EC / size 0x14.
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * Type provenance (lead opaque-word ruling): every writer/reader of
 * D_800A1824 is bare 32-bit sw/lw. Typed unsigned int (u32); refine if a
 * narrowing reader is later found. Pilot proof: func_80042C28 matched.
 */
extern unsigned int D_800A1824;

int func_80042BEC(void) {
    D_800A1824 = 1;
    return 0;
}
