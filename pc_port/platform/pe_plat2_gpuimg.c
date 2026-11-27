/* Image-transfer wrappers (guest 0x8007506C LoadImage, 0x800750CC StoreImage, 0x8007512C MoveImage), written
 * clean-room from the black-box vector sets gpu2b (docs/evidence/native-gpu2-20261126), the port's host GPU/DMA
 * model API and psx-spx; no SDK source, decompiled source, tables or disassembly were read.
 *
 * Contract (every line traced to the vectors):
 *  - all three first call the rectangle report 0x80074E28 with a label (0x800118D4 LoadImage, 0x800118E0
 *    StoreImage, 0x800118EC MoveImage) and the rectangle, and ignore its result.
 *  - the library jump table pointer is the word 0x80095744.  LoadImage / StoreImage dispatch through the table:
 *    worker = table[+0x20] (LoadImage) or table[+0x1C] (StoreImage), enqueue function = table[+8], and call
 *    enqueue(worker, rect, 8, data); the result of that call is returned.
 *  - MoveImage returns -1 without further access when the rectangle's width halfword is zero, or (after reading the
 *    height halfword) its height is zero.  Otherwise it fills the three data words of the 0x14-byte command block
 *    0x800957E4 (the first two words are left alone): +0x8 = the rectangle's position word, +0xC =
 *    (destination_y << 16) | (destination_x & 0xFFFF), +0x10 = the rectangle's size word (read after the
 *    first two stores), then calls enqueue(table[+0x18], 0x800957E4, 0x14, 0) and returns its result.
 *  - Host-only (not observable in the vectors): the rectangle arrives as a host pointer.  A NULL rectangle is refused
 *    with -1 before anything else.  One inside guest RAM or the scratchpad is passed on by its guest address; a host
 *    object has no guest address, so LoadImage / StoreImage hand its two words to the port's inline enqueue
 *    (pe_plat_gpu_swap_inline8) when the enqueue slot is the native one, and through the bootstrap indirect-call
 *    boundary (target, worker, 0, 8, data + the two words) otherwise.  A slot function that is not the native
 *    enqueue is called through the guest-code registry (PE_GuestCall), whose boundary names the calling wrapper. */
#include <stdint.h>
#include "pe_guestcode.h"
#include "game_port.h"
#include "pe_plat/gpu_swap.h"
#include "pe_plat2_gpuimg_view.h"

void PE_Decomp_NoteStopDetail(const char *text);   /* pe_guest_decomp.h (not included: it pulls the game prototypes) */

#define W_TABLE      0x80095744u
#define SLOT_ENQUEUE 0x08u
#define SLOT_MOVE    0x18u
#define SLOT_STORE   0x1Cu
#define SLOT_LOAD    0x20u
#define NATIVE_ENQUEUE 0x80076C34u
#define LABEL_LOAD   0x800118D4u
#define LABEL_STORE  0x800118E0u
#define LABEL_MOVE   0x800118ECu
#define MOVE_BLOCK   0x800957E4u

static int32_t enqueue(const char *caller, uint32_t enqueue_fn, pe_addr_t worker, pe_addr_t arg, int32_t bytes, uint32_t aux)
{
    if (enqueue_fn == NATIVE_ENQUEUE) return pe_plat_gpu_swap(worker, arg, bytes, aux);
    return PE_GuestCall(caller, enqueue_fn, 4, worker, arg, (uintptr_t)(uint32_t)bytes, aux);
}

static int32_t transfer(const char *caller, uint32_t label, uint32_t slot, const RECT *rect, pe_addr_t data)
{
    GpuImgRect v;
    uint32_t table, worker, fn, words[2];
    if (rect == NULL) return -1;
    func_80074E28(label, rect);
    v = gpuimg_view(rect);
    table = PE_LoadU32(W_TABLE);
    worker = PE_LoadU32(table + slot);
    fn = PE_LoadU32(table + SLOT_ENQUEUE);
    if (v.host == NULL) return enqueue(caller, fn, worker, v.guest, 8, data);
    words[0] = gpuimg_rd32(&v, 0);
    words[1] = gpuimg_rd32(&v, 4);
    if (fn == NATIVE_ENQUEUE) return pe_plat_gpu_swap_inline8(worker, words[0], words[1], data);
    {
        int32_t r = Bootstrap_ReturnInt4Indirect("func_80076C34", caller, 0, fn, worker, 0, 8, data, words, sizeof words);
        PE_Decomp_NoteStopDetail("image rectangle is a host object with no guest address; enqueue slot is not the native one");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return r;
    }
}

int pe_plat_gpuimg_load_image(const RECT *rect, pe_addr_t data)
{
    return (int)transfer("func_8007506C", LABEL_LOAD, SLOT_LOAD, rect, data);
}

int pe_plat_gpuimg_store_image(const RECT *rect, pe_addr_t data)
{
    return (int)transfer("func_800750CC", LABEL_STORE, SLOT_STORE, rect, data);
}

int pe_plat_gpuimg_move_image(const RECT *rect, int destination_x, int destination_y)
{
    GpuImgRect v;
    uint32_t position, table, worker, fn;
    if (rect == NULL) return -1;
    func_80074E28(LABEL_MOVE, rect);
    v = gpuimg_view(rect);
    if ((int16_t)gpuimg_rd16(&v, 4) == 0) return -1;
    if ((int16_t)gpuimg_rd16(&v, 6) == 0) return -1;
    position = gpuimg_rd32(&v, 0);
    table = PE_LoadU32(W_TABLE);
    PE_StoreU32(MOVE_BLOCK + 0xC, ((uint32_t)destination_y << 16) | ((uint32_t)destination_x & 0xFFFFu));
    PE_StoreU32(MOVE_BLOCK + 0x8, position);
    PE_StoreU32(MOVE_BLOCK + 0x10, gpuimg_rd32(&v, 4));
    worker = PE_LoadU32(table + SLOT_MOVE);
    fn = PE_LoadU32(table + SLOT_ENQUEUE);
    return (int)enqueue("func_8007512C", fn, worker, MOVE_BLOCK, 0x14, 0);
}
