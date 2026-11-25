/* libpress MDEC reset wrapper (guest 0x8010BE3C, movie module overlay
 * ovl_039F), written clean-room as a native from black-box vectors of the
 * original code (batch cdgen37, pc_port/tools/pe_mdec_reset_vectors.py, checked by
 * pc_port/tools/pe_mdec_reset_check.c).  Independent re-implementation: the
 * behaviour below is exactly what the 6000 vectors show; no SDK source and
 * nothing from any other implementation was used, and no claim is made about
 * the original's source.
 *
 * Behaviour: the argument is tested as a whole 32-bit word (vectors with only
 * the low byte, halfword, or bit 31 equal to zero / set and negative words all
 * go the other way).  When it is zero the libetc ResetCallback (guest
 * 0x80073C94, bound to pe_plat_reset_callback) is called first, with no
 * argument that matters.  In every case the MDEC reset core (guest 0x8010C0FC)
 * is then called with the unchanged mode word.  The wrapper itself makes no
 * guest load or store (L and S are empty on every vector).  The original
 * returns the core's v0 untouched; the port's prototype for this row is
 * void, so no result is returned here (the checker does not compare it).
 *
 * The table/DMA work belongs to the core 0x8010C0FC and the rows behind it,
 * not to this wrapper. */
#include <stdint.h>

#include "pe_plat/irq_callback.h"
#include "pe_plat/mdec_callback.h"

void pe_plat_mdec_reset(int mode)
{
    if (mode == 0)
        (void)func_80073C94();
    func_8010C0FC(mode);
}
