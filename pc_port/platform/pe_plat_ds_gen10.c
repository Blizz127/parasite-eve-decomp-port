/* A CD-streaming library debug status printout (guest 0x800809E0), written
 * clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen10, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.
 * Behaviour is exactly the guest loads and callee calls seen in the vectors:
 * eight printf calls in a fixed order, no RAM or register stores.  The format
 * strings live in guest RAM and are referred to by address only. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_bios_string.h"  /* declares PE_Printf_Guest (not pulled in by pe_sdk.h in this tree) */

/* Format strings (guest addresses). */
#define FMT_HEADER          0x80011D74u  /* no arguments */
#define FMT_WORDS3          0x80011D94u  /* three words */
#define FMT_BYTES5          0x80011DB0u  /* five bytes */
#define FMT_WORD_A          0x80011DD8u  /* one word */
#define FMT_WORD_B          0x80011DF8u  /* one word */
#define FMT_WORDS3_B        0x80011E0Cu  /* three words */
#define FMT_TRAILER         0x80011E4Cu  /* one argument: a string address chosen below */
#define STR_WHEN_ACTIVE     0x80011E3Cu  /* trailer argument when the poll result is nonzero */
#define STR_WHEN_IDLE       0x80011E44u  /* trailer argument when the poll result is zero */

/* State words read (32-bit), in print order. */
#define ADDR_W3_0           0x8009B574u  /* FMT_WORDS3 argument 1 */
#define ADDR_W3_1           0x8009B578u  /* FMT_WORDS3 argument 2 */
#define ADDR_W3_2           0x8009B57Cu  /* FMT_WORDS3 argument 3 */
#define ADDR_WORD_A         0x8009B598u  /* FMT_WORD_A argument */
#define ADDR_WORD_B         0x8009B594u  /* FMT_WORD_B argument */
#define ADDR_W3B_0          0x800A36A0u  /* FMT_WORDS3_B argument 1 */
#define ADDR_W3B_1          0x800A36A4u  /* FMT_WORDS3_B argument 2 */
#define ADDR_W3B_2          0x800A36A8u  /* FMT_WORDS3_B argument 3 */

/* Five consecutive unsigned bytes read for FMT_BYTES5. */
#define ADDR_BYTES5         0x8009B558u

/* Guest callees declared in pe_sdk.h:
 *   PE_Printf_Guest(fmt, a1..a7)  printf; unused trailing arguments are zero
 *   func_800822AC()               poll returning a status word */
void pe_plat_ds_status(void)
{
    uint32_t w0, w1, w2;
    uint32_t b0, b1, b2, b3, b4;
    uint32_t word_a, word_b;
    uint32_t x0, x1, x2;

    PE_Printf_Guest(FMT_HEADER, 0, 0, 0, 0, 0, 0, 0);

    w0 = PE_LoadU32(ADDR_W3_0);
    w1 = PE_LoadU32(ADDR_W3_1);
    w2 = PE_LoadU32(ADDR_W3_2);
    PE_Printf_Guest(FMT_WORDS3, w0, w1, w2, 0, 0, 0, 0);

    b0 = PE_LoadU8(ADDR_BYTES5 + 0);
    b1 = PE_LoadU8(ADDR_BYTES5 + 1);
    b2 = PE_LoadU8(ADDR_BYTES5 + 2);
    b3 = PE_LoadU8(ADDR_BYTES5 + 3);
    b4 = PE_LoadU8(ADDR_BYTES5 + 4);
    PE_Printf_Guest(FMT_BYTES5, b0, b1, b2, b3, b4, 0, 0);

    word_a = PE_LoadU32(ADDR_WORD_A);
    PE_Printf_Guest(FMT_WORD_A, word_a, 0, 0, 0, 0, 0, 0);

    word_b = PE_LoadU32(ADDR_WORD_B);
    PE_Printf_Guest(FMT_WORD_B, word_b, 0, 0, 0, 0, 0, 0);

    x0 = PE_LoadU32(ADDR_W3B_0);
    x1 = PE_LoadU32(ADDR_W3B_1);
    x2 = PE_LoadU32(ADDR_W3B_2);
    PE_Printf_Guest(FMT_WORDS3_B, x0, x1, x2, 0, 0, 0, 0);

    /* The poll result only selects the trailer's string argument. */
    PE_Printf_Guest(FMT_TRAILER,
                    func_800822AC() ? STR_WHEN_ACTIVE : STR_WHEN_IDLE,
                    0, 0, 0, 0, 0, 0);
}
