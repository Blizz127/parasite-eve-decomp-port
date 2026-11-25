/* GTE default initialisation (guest 0x80077F7C, libgte InitGeom), written
 * clean-room from the psx-spx GTE documentation and the port's host GTE model,
 * with no SDK source read.
 *
 * What it does here: loads the seven projection/depth control registers of the
 * port's host GTE model (ctrl 24..30: OFX, OFY, H, DQA, DQB, ZSF3, ZSF4).  Every
 * other control and data register is left as it was.
 *
 * Provenance: the register numbering and formats are per psx-spx, which states
 * no reset/default values.  The seven loaded values are the 2026-11-04
 * black-box interpreter observations of the original (6000 randomized cases
 * plus the 24-case helper survey,
 * docs/evidence/native-gte-init-investigation-20261104/NOTICE.txt), confirmed
 * by the suite test 77F7C_initgeom_constants.
 *
 * Not modelled (the host has no CP0/COP2-enable state and no BIOS patching):
 * the original also saves its return address in guest RAM, sets the CP0 status
 * CU2 bit, and calls a helper that inspects/patches a BIOS handler and writes a
 * callback address into guest word 0x800A3450.  This provider leaves guest RAM
 * untouched; see docs/evidence/native-initgeom-20261125/REPORT.md.
 * Evidence: docs/evidence/native-initgeom-20261125/. */
#include "pe_plat/gte.h"
#include "pe_sdk.h"

/* GTE control registers (psx-spx numbering, ctc2 $24..$30). */
#define GTE_CTRL_OFX    24u /* screen offset X, 16.16 */
#define GTE_CTRL_OFY    25u /* screen offset Y, 16.16 */
#define GTE_CTRL_H      26u /* projection plane distance */
#define GTE_CTRL_DQA    27u /* depth-cue coefficient, 8.8 */
#define GTE_CTRL_DQB    28u /* depth-cue offset, 32-bit (psx-spx: 1,7,24? fraction width uncertain) */
#define GTE_CTRL_ZSF3   29u /* average-of-3 Z scale (psx-spx: N/30h; 0x155 = 0x400/3) */
#define GTE_CTRL_ZSF4   30u /* average-of-4 Z scale (psx-spx: N/40h; 0x100 = 0x400/4) */

#define INIT_OFX        0u
#define INIT_OFY        0u
#define INIT_H          0x3E8u
#define INIT_DQA        ((uint32_t)-0x1062)
#define INIT_DQB        0x01400000u
#define INIT_ZSF3       0x155u
#define INIT_ZSF4       0x100u

void pe_plat_gte_init_geom(void)
{
    PE_GTE_WriteCtrl(GTE_CTRL_OFX, INIT_OFX);
    PE_GTE_WriteCtrl(GTE_CTRL_OFY, INIT_OFY);
    PE_GTE_WriteCtrl(GTE_CTRL_H, INIT_H);
    PE_GTE_WriteCtrl(GTE_CTRL_DQA, INIT_DQA);
    PE_GTE_WriteCtrl(GTE_CTRL_DQB, INIT_DQB);
    PE_GTE_WriteCtrl(GTE_CTRL_ZSF3, INIT_ZSF3);
    PE_GTE_WriteCtrl(GTE_CTRL_ZSF4, INIT_ZSF4);
}
