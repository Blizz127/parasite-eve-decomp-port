/* Host-name entry points (pe_port_compat.h) for the rotation-matrix helpers,
 * forwarding to the clean-room models in pe_plat_gte_trig.c.  Lowered room
 * code calls PE_RotMatrixYXZ_Guest by name (gen_overlay_ports host
 * replacement for guest 0x80079754); hand ports use the host-array forms.
 * Evidence: docs/evidence/native-gte-yxz-20261031/. */
#include "pe_plat/gte.h"
#include "pe_guest_ram.h"

#define MATRIX_HALVES 9u

pe_addr_t PE_RotMatrixYXZ_Guest(pe_addr_t r, pe_addr_t m);
void PE_RotMatrix79754_values(const int16_t angles[3], int16_t out[9]);
void PE_RotMatrix79754(const int16_t angles[3], pe_addr_t out);
void PE_RotMatrix794C4_values(const int16_t angles[3], int16_t out[9]);

pe_addr_t PE_RotMatrixYXZ_Guest(pe_addr_t r, pe_addr_t m)
{
    return pe_plat_gte_rot_matrix_yxz(r, m);
}

void PE_RotMatrix79754_values(const int16_t angles[3], int16_t out[9])
{
    pe_plat_gte_rot_matrix_yxz_values(angles, out);
}

/* RotMatrixYXZ from host angles into a guest matrix (nine halfwords). */
void PE_RotMatrix79754(const int16_t angles[3], pe_addr_t out)
{
    int16_t m[MATRIX_HALVES];
    uint32_t i;

    pe_plat_gte_rot_matrix_yxz_values(angles, m);
    for (i = 0; i < MATRIX_HALVES; i++)
        PE_StoreU16(out + 2u * i, (uint16_t)m[i]);
}

void PE_RotMatrix794C4_values(const int16_t angles[3], int16_t out[9])
{
    pe_plat_gte_rot_matrix_values(angles, out);
}
