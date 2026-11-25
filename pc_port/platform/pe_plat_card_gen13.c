/* Two memory-card library start-up callers (guest 0x8007DDD4 and 0x8007DE40),
 * written clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen13, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Behaviour is
 * exactly the fixed call sequences seen in the vectors; no guest loads or
 * stores.  Only one callee result (the pad-initialised state in the init
 * routine) influences anything: it gates a single argument, never the order. */
#include <stdint.h>
#include "pe_sdk.h"

#include "pe_decomp_boundary.h"   /* PE_Decomp_BoundaryFrom */

#define CARD_PAD_CLEAR_OFF   0u          /* pad-clear flag value passed to func_80073C74 */

#define ADDR_CARD_INIT_CALL  0x8007E314u /* libcard entry taking the init argument */
#define ADDR_CARD_PATCH_CB   0x8007E3DCu /* libcard entry, no arguments */
#define ADDR_CARD_SETUP      0x8007E470u /* libcard entry, no arguments */
#define ADDR_CARD_START_CALL 0x8007E324u /* libcard entry, no arguments */

/* Guest 0x8007DDD4: with the pad-clear flag set to 0, enter the critical
 * section, query the pad-initialised state (func_8007DEB0), then call in
 * order the libcard entry 0x8007E314 with a single argument that is a0 when
 * the pad-initialised result is nonzero and 0 when it is zero, the card
 * patch routine 0x8007E4E0, and the argument-less entries 0x8007E3DC and
 * 0x8007E470, and leave the critical section.  Every call is unconditional;
 * only that one argument depends on a callee result. */
void pe_plat_card_init(int a0)
{
    int32_t pad_ready;

    func_80073C74(CARD_PAD_CLEAR_OFF);
    (void)func_80072714();
    pad_ready = func_8007DEB0();
    (void)PE_Decomp_BoundaryFrom("func_8007DDD4", "func_8007E314", ADDR_CARD_INIT_CALL, 1u,
                                 pad_ready != 0 ? (uintptr_t)(uint32_t)a0 : 0u, 0u, 0u, 0u);
    func_8007E4E0();
    (void)PE_Decomp_BoundaryFrom("func_8007DDD4", "func_8007E3DC", ADDR_CARD_PATCH_CB, 0u,
                                 0u, 0u, 0u, 0u);
    (void)PE_Decomp_BoundaryFrom("func_8007DDD4", "func_8007E470", ADDR_CARD_SETUP, 0u,
                                 0u, 0u, 0u, 0u);
    func_80072724();
}

/* Guest 0x8007DE40: enter the critical section, call the argument-less libcard
 * entry 0x8007E324, set the pad-clear flag to 0 and leave the critical
 * section.  Every call is unconditional; the enter result is unused. */
void pe_plat_card_start(void)
{
    (void)func_80072714();
    (void)PE_Decomp_BoundaryFrom("func_8007DE40", "func_8007E324", ADDR_CARD_START_CALL, 0u,
                                 0u, 0u, 0u, 0u);
    func_80073C74(CARD_PAD_CLEAR_OFF);
    func_80072724();
}
