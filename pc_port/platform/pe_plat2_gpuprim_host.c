/* Host-only DrawPrim caller for command words the host holds (not a guest function and not covered by the vectors,
 * which never see a host buffer): the words are placed in a guest-RAM frame behind a packet header whose byte 3 is
 * the word count, then the clean-room DrawPrim (pe_plat2_gpuprim.c) runs exactly as for a guest packet, so slot 15 /
 * slot 5 go through the guest-code registry like any packet.  The frame bytes are restored afterwards (the host
 * words are transient, the caller must not leave guest RAM changed).  Written clean-room with the gpu2c provider: no
 * SDK source, decompiled source, tables or disassembly were read. */
#include <stdint.h>
#include <string.h>
#include "pe_guest_ram.h"
#include "pe_plat/gpuprim.h"

int PE_func_80075358_Transient(const uint32_t *words, uint8_t count)
{
    uint8_t saved[4 + 4 * 255];
    uint32_t size = 4u + 4u * (uint32_t)count, i;
    pe_addr_t frame = PE_GuestFrame_Enter(size);
    int result;
    memcpy(saved, PE_Translate(frame, size), size);
    PE_StoreU32(frame, (uint32_t)count << 24);
    for (i = 0; i < count; i++) PE_StoreU32(frame + 4u + 4u * i, words[i]);
    result = pe_plat_gpuprim_draw_prim(frame);
    memcpy(PE_Translate(frame, size), saved, size);
    PE_GuestFrame_Leave(frame, size);
    return result;
}
