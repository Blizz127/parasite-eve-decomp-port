/* Seven small libcd / libds callers (guest 0x800813E8, 0x800822BC,
 * 0x80082400, 0x80080F64, 0x8007FB04, 0x8007BBB0, 0x8007C2A0), written
 * clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Each function's
 * behaviour is exactly the guest loads/stores, calls and return value seen
 * in the vectors. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

/* Guest data words read or written by the callers. */
#define ADDR_DS_STATE      0x8009B70Cu  /* state word of 0x800822BC; 1 = callbacks held */
#define ADDR_DS_READY_CB   0x8009B700u  /* callback word passed to 0x800824C8 */
#define ADDR_DS_START_CB   0x8009B704u  /* callback word passed to 0x800824DC */
#define ADDR_DS_TEXT_STATE 0x800B28F8u  /* word set to 2 by 0x80082400 */
#define ADDR_CD_FB554      0x8009B554u  /* cleared by 0x8007FB04 */
#define ADDR_CD_AFB4       0x8009AFB4u  /* cleared by 0x8007FB04 and 0x8007BBB0 */
#define ADDR_CD_AFB8       0x8009AFB8u  /* cleared by 0x8007FB04 and 0x8007BBB0 */
#define ADDR_CD_AFC4       0x8009AFC4u  /* cleared by 0x8007BBB0 */
#define ADDR_CD_AFC8       0x8009AFC8u  /* cleared by 0x8007BBB0 */
#define ADDR_ST_BUSY       0x800A8020u  /* 0x8007C2A0 acts only while this is 0 */
#define ADDR_ST_BACKLOC    0x800A3490u  /* position record converted by 0x8007AA34 */
#define ADDR_ST_BACKVAL    0x800A3494u  /* word returned by 0x8007C2A0 */

/* Guest code addresses observed in the vectors as call arguments. */
#define CB_DS_TEXT_EC      0x80082444u  /* a0 of 0x80081D74 from 0x80082400 */
#define CB_DS_MODE2        0x80080F98u  /* a0 of 0x80081D74 from 0x80080F64 */
#define CB_CD_INTR         0x8007C13Cu  /* handler passed to 0x80073CC4 */

#define DS_STATE_HELD      1u   /* state value for which 0x800822BC calls out */
#define DS_TEXT_EC_KEY     2    /* argument value of 0x80082400 that calls out */
#define DS_MODE2_KEY       2    /* low byte of the 0x80080F64 argument that calls out */
#define CD_INTR_SOURCE     2    /* first argument of 0x80073CC4 */

/* 0x800813E8: calls 0x8007C564 and nothing else. */
void pe_plat_ds_813e8(void)
{
    func_8007C564();
}

/* 0x800822BC: when the state word is 1, call 0x800824C8 and 0x800824DC with
 * the two callback words (zero in every vector); the state word is cleared
 * in every case, after those calls. */
void pe_plat_ds_er_clear(void)
{
    if (PE_LoadU32(ADDR_DS_STATE) == DS_STATE_HELD) {
        int ready = (int)PE_LoadU32(ADDR_DS_READY_CB);
        int start = (int)PE_LoadU32(ADDR_DS_START_CB);
        func_800824C8(ready);
        func_800824DC(start);
    }
    PE_StoreU32(ADDR_DS_STATE, 0);
}

/* 0x80082400: argument 2 calls 0x80081D74(0x80082444, 0); any other
 * argument stores 2 into the state word. */
void pe_plat_ds_text_ec(unsigned char a0)
{
    if (a0 == DS_TEXT_EC_KEY)
        func_80081D74((int)CB_DS_TEXT_EC, 0);
    else
        PE_StoreU32(ADDR_DS_TEXT_STATE, 2);
}

/* 0x80080F64: when the low byte of the argument is 2, call
 * 0x80081D74(0x80080F98, -1); otherwise nothing. */
void pe_plat_cd_set_mode2_callback(int a0)
{
    if ((a0 & 0xFF) == DS_MODE2_KEY)
        func_80081D74((int)CB_DS_MODE2, -1);
}

/* 0x8007FB04: clear one word, call 0x8007B9EC, clear two more, then call
 * 0x80073D58(0, 0). */
void pe_plat_cd_7fb04(void)
{
    PE_StoreU32(ADDR_CD_FB554, 0);
    func_8007B9EC();
    PE_StoreU32(ADDR_CD_AFB4, 0);
    PE_StoreU32(ADDR_CD_AFB8, 0);
    func_80073D58(0, 0);
}

/* 0x8007BBB0: clear four words, call 0x80073C94, then register the handler
 * 0x8007C13C for source 2 through 0x80073CC4. */
void pe_plat_cd_init_intr(void)
{
    PE_StoreU32(ADDR_CD_AFB8, 0);
    PE_StoreU32(ADDR_CD_AFB4, 0);
    PE_StoreU32(ADDR_CD_AFC8, 0);
    PE_StoreU32(ADDR_CD_AFC4, 0);
    func_80073C94();
    func_80073CC4(CD_INTR_SOURCE, CB_CD_INTR);
}

/* 0x8007C2A0: returns -1 unless the busy word is 0.  Otherwise converts the
 * record at 0x800A3490 with 0x8007AA34, passes that value plus one and the
 * argument to 0x8007A930, and returns the word at 0x800A3494. */
int pe_plat_cd_st_get_backloc(int a0)
{
    int32_t pos;
    if (PE_LoadU32(ADDR_ST_BUSY) != 0)
        return -1;
    pos = func_8007AA34(ADDR_ST_BACKLOC);
    func_8007A930((int32_t)((uint32_t)pos + 1u), (pe_addr_t)a0);
    return (int)PE_LoadU32(ADDR_ST_BACKVAL);
}
