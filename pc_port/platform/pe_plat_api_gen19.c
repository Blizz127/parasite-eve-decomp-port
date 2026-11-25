/* Device-table first-file lookup (guest 0x800727B4) and the device hook it
 * installs (guest 0x80072950), written clean-room from black-box vectors of
 * the original code (pc_port/tools/pe_cd_leaf_vectors.py cdgen19, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Behaviour is
 * exactly the guest loads, stores and calls seen in the vectors. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "pe_decomp_boundary.h"
#include "pe_plat/bios_string.h"
#include "pe_plat/bios_file.h"

#define KRAM_TABLE_PTR     0x150u        /* word: address of the device table (kernel RAM) */
#define KRAM_TABLE_SIZE    0x154u        /* word: table size in bytes */
#define ENTRY_STRIDE       80u           /* bytes per table entry */
#define ENTRY_NAME_OFF     0x00u         /* word: pointer to the entry's name string */
#define ENTRY_FUNC_OFF     0x34u         /* word: the entry's function word */

#define NAME_BUF           0x800A32D8u   /* copy of the requested device name */
#define SAVED_FUNC         0x800A32D0u   /* word: the function word firstfile displaced */
#define HOOK_ENTRY         0x80072950u   /* the function word firstfile installs */
#define NAME_STOP_BELOW    59            /* the name copy stops at a signed byte below this */

/* Return the address of the first table entry whose name string compares
 * equal to NAME_BUF, or 0 when there is none.  The table holds size / 80
 * entries starting at the base word; the scan end is base + count * 80 in
 * 32-bit arithmetic, and nothing is scanned when that end is not above the
 * base (it wrapped, or the table is empty).  An entry with a null name is
 * skipped without a compare. */
static uint32_t find_device(void)
{
    uint32_t size = PE_LoadU32(KRAM_TABLE_SIZE);
    uint32_t base = PE_LoadU32(KRAM_TABLE_PTR);
    uint32_t end = base + (size / ENTRY_STRIDE) * ENTRY_STRIDE;
    uint32_t entry;

    for (entry = base; entry < end; entry += ENTRY_STRIDE) {
        uint32_t name = PE_LoadU32(entry + ENTRY_NAME_OFF);

        if (name != 0 && func_80072A54(name, NAME_BUF) == 0)
            return entry;
    }
    return 0;
}

/* 0x800727B4 (name, dir):
 *  1. Copy the name into NAME_BUF, one byte at a time (each byte is read
 *     twice), stopping before the first signed byte below 59; store a zero
 *     terminator after the copy.
 *  2. Scan the device table for an entry whose name equals the copy.  When
 *     there is none, return 0.
 *  3. Read that entry's function word and store it at SAVED_FUNC.
 *  4. Scan the table again from the start (same rule) and store HOOK_ENTRY into
 *     the function word of the entry found.
 *  5. Return the result of the BIOS call 0x80072A64 (name, dir) with the
 *     original name pointer. */
int pe_plat_api_firstfile(uint32_t name, int dir)
{
    uint32_t i = 0;
    uint32_t entry;

    while ((int8_t)PE_LoadU8(name + i) >= NAME_STOP_BELOW) {
        PE_StoreU8(NAME_BUF + i, (uint8_t)(int8_t)PE_LoadU8(name + i));
        i++;
    }
    PE_StoreU8(NAME_BUF + i, 0);

    entry = find_device();
    if (entry == 0)
        return 0;
    PE_StoreU32(SAVED_FUNC, PE_LoadU32(entry + ENTRY_FUNC_OFF));

    entry = find_device();
    if (entry != 0)
        PE_StoreU32(entry + ENTRY_FUNC_OFF, HOOK_ENTRY);

    return PE_Decomp_BoundaryFrom("func_800727B4", "func_80072A64", 0x80072A64u, 2,
                                  name, (uintptr_t)(uint32_t)dir, 0, 0);
}

/* 0x80072950 (a0, a1, a2): the device hook.
 *  1. When the word at a0 is zero, store 1 there.
 *  2. Scan the device table for an entry whose name equals NAME_BUF; when one
 *     is found, store the saved function word (SAVED_FUNC) into its function
 *     word.
 *  3. Call the saved function word through the guest dispatcher with
 *     (a0, a1, a2) (no null check) and return its result. */
int pe_plat_api_firstfile_hook(uint32_t a0, int a1, int a2)
{
    uint32_t entry;
    uint32_t saved;

    if (PE_LoadU32(a0) == 0)
        PE_StoreU32(a0, 1);

    (void)PE_LoadU32(SAVED_FUNC);
    entry = find_device();
    saved = PE_LoadU32(SAVED_FUNC);
    if (entry != 0)
        PE_StoreU32(entry + ENTRY_FUNC_OFF, saved);

    return PE_GuestCall("func_80072950_saved_device_hook", saved, 3,
                        a0, (uintptr_t)(uint32_t)a1, (uintptr_t)(uint32_t)a2, 0);
}
