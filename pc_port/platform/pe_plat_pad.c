/* libpad PadGetState (guest 0x800825C0), written clean-room by an author who
 * saw neither SDK source nor this port's earlier code: 2670 black-box vectors
 * of the original (pc_port/tools/pe_pad_vectors.py); see
 * docs/evidence/native-pad-getstate-20261030/. */
/* libpad PadGetState host model, written from black-box vectors only
 * (vectors.txt: 2430 cases of the original code); no SDK source or
 * disassembly was consulted.  Record offsets below are named from what the
 * vectors show.  The port's low nibble never changes the result; only
 * (port & 0xF0) picks the record, via pe_plat_pad_record(). */
#include "pe_plat/pad.h"
#include "pe_guest_ram.h"

/* u32 guest pointer: the state can degrade only when it differs from the
 * record base (base+0x40, base+0x80, 0 and other values all count). */
#define PAD_REC_LINK      0x10u
/* u32 guest pointer to a byte; any nonzero byte there degrades the state. */
#define PAD_REC_FLAG_PTR  0x30u
/* u32: any value above 0xFFFF degrades the state (0x8000/0xFFFF do not;
 * 0x10000, 0x18000, 0x20000, 0xFFFF0000, 0x80000000 do). */
#define PAD_REC_COUNT     0x34u
/* u8: nonzero (0x01, 0x80 seen) degrades the state when link != base. */
#define PAD_REC_BUSY      0x38u
/* u8 raw state, 0..8; returned as is unless degraded. */
#define PAD_REC_STATE     0x49u

#define PAD_COUNT_LIMIT   0xFFFFu

/* Degraded state: 2 and 3 read as 1, 6 reads as 4; 0, 1, 4, 5, 7, 8 are
 * unchanged. */
static int pad_degrade(int state)
{
    if (state == 2 || state == 3) {
        return 1;
    }
    if (state == 6) {
        return 4;
    }
    return state;
}

int pe_plat_pad_get_state(uint32_t port)
{
    uint32_t rec = pe_plat_pad_record(port);
    int state;

    if (rec == 0) {
        return 0;
    }
    state = PE_LoadU8(rec + PAD_REC_STATE);

    if (PE_LoadU8(PE_LoadU32(rec + PAD_REC_FLAG_PTR)) != 0) {
        return pad_degrade(state);
    }
    if (PE_LoadU32(rec + PAD_REC_COUNT) > PAD_COUNT_LIMIT) {
        return pad_degrade(state);
    }
    if (PE_LoadU32(rec + PAD_REC_LINK) != rec &&
        PE_LoadU8(rec + PAD_REC_BUSY) != 0) {
        return pad_degrade(state);
    }
    return state;
}
