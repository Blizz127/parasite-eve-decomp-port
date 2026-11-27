/* Independent provider adapted from the retained WP2a host implementation.
 * Public memmove semantics and black-box retail vectors; see pe_plat/libc.h. */
#include "pe_plat/libc.h"
#include "pe_guest_ram.h"

/* Guest memory fills from the documented call contracts and black-box tests. */
void pe_plat_fill_bytes(pe_plat_guest_addr dst, uint8_t value, int32_t count)
{
    uint32_t remaining = (uint32_t)count;
    while (remaining != 0u) {
        PE_StoreU8(dst++, value);
        --remaining;
    }
}

void pe_plat_clear_words(pe_plat_guest_addr dst, int32_t count)
{
    uint32_t remaining = (uint32_t)count;
    while (remaining != 0u) {
        PE_StoreU32(dst, 0u);
        dst += 4u;
        --remaining;
    }
}

/* Sequential byte exchange for inventory sorting. Overlapping ranges and
 * ordering were checked against black-box retail input/output, not SDK code. */
void pe_plat_swap_bytes(pe_plat_guest_addr a, pe_plat_guest_addr b, uint32_t n)
{
    for (uint32_t i = 0; i < n; ++i) {
        uint8_t left = PE_LoadU8(a + i);
        uint8_t right = PE_LoadU8(b + i);
        PE_StoreU8(a + i, right);
        PE_StoreU8(b + i, left);
    }
}

pe_plat_guest_addr pe_plat_libc_move_bytes(pe_plat_guest_addr dst, pe_plat_guest_addr src, uint32_t n)
{
    uint32_t i;
    if ((int32_t)n <= 0)
        return dst;
    if (dst >= src) {
        for (i = n; i != 0u; i--)
            PE_StoreU8(dst + i - 1u, PE_LoadU8(src + i - 1u));
        return dst;
    }
    for (i = 0; i < n; i++)
        PE_StoreU8(dst + i, PE_LoadU8(src + i));
    return dst + n;
}

void pe_plat_libsn_main(void)
{
    if (PE_LoadU32(0x80094538u) == 0u)
        PE_StoreU32(0x80094538u, 1u);
}

/* This image's sound initialisation calls the three existing native timer
 * initialisers in order; the retained WP2a provider uses the same contract. */
extern void func_80036DF8(void);
extern void func_80036E34(void);
extern void func_80036E58(void);

void pe_plat_snd_init(void)
{
    func_80036DF8();
    func_80036E34();
    func_80036E58();
}
