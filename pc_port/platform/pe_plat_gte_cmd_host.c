/* Host-name entry points for the GTE matrix-command leaves, forwarding to
 * the clean-room models in pe_plat_gte_cmd.c.  Lowered room code calls
 * PE_MulMatrix0_Guest (guest 0x800785D4) and PE_ApplyMatrix_Guest (guest
 * 0x80078BE4) by name (gen_overlay_ports host replacements).
 * Evidence: docs/evidence/native-gte-cmd-20261031/. */
#include "pe_plat/gte.h"
#include "pe_guest_ram.h"

pe_addr_t PE_MulMatrix0_Guest(pe_addr_t left, pe_addr_t right, pe_addr_t out);
pe_addr_t PE_ApplyMatrix_Guest(pe_addr_t m, pe_addr_t v, pe_addr_t out);

pe_addr_t PE_MulMatrix0_Guest(pe_addr_t left, pe_addr_t right, pe_addr_t out)
{
    return pe_plat_gte_mul_matrix0(left, right, out);
}

pe_addr_t PE_ApplyMatrix_Guest(pe_addr_t m, pe_addr_t v, pe_addr_t out)
{
    return pe_plat_gte_apply_matrix(m, v, out);
}
