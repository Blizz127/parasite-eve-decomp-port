/* func_80191B64 (ovl_03D2 movie-issue poll worker) runs the x0/x1 latch on
 * EVERY call.  Retail's frame-size-unchanged branch at 0x80191CC0
 * (beq v1,v0 -> 0x80191D38) lands on that latch, and
 * src/overlays/ovl_03D2/func_80191B64.c writes s->x0 = s->x1 without a
 * condition.  The hand port used to jump past it, straight to the y copy at
 * 0x80191D8C, whenever the frame size was unchanged.
 *
 * The fixture is guest-RAM only: a finished (state 2) stream record that
 * func_8007C484 promotes, a movie record whose end frame keeps the volume
 * fade and the end latch out of the way, and D_801D0DE0/DE2 already equal to
 * the slice's size. */
#include "pe_str_feed.h"

int func_80191B64(pe_addr_t state);

#define MV1A_S    0x80150000u   /* the sprite block func_80191B64 fills (a0) */
#define MV1A_BASE 0x80140000u   /* stream record table, record 0 active */
#define MV1A_REC  0x80141000u   /* movie record: end frame at +8 */

static void MV1A_Seed(uint8_t hires)
{
    ResetTestState();
    PE_StrFeed_Reset();
    PE_CdReg_Reset();
    PE_StoreU32(0x800C0DC8u, MV1A_BASE);    /* record base */
    PE_StoreU32(0x800BE9ECu, 0u);           /* active index */
    PE_StoreU16(MV1A_BASE, 2u);             /* slice finished */
    PE_StoreU32(MV1A_BASE + 8u, 5u);        /* frame */
    PE_StoreU16(MV1A_BASE + 16u, 320u);     /* width */
    PE_StoreU16(MV1A_BASE + 18u, 240u);     /* height */
    PE_StoreU32(0x801D11ACu, MV1A_REC);
    PE_StoreU16(MV1A_REC + 8u, 1000u);      /* end frame */
    PE_StoreU16(0x801D0DE0u, 320u);         /* size unchanged */
    PE_StoreU16(0x801D0DE2u, 240u);
    PE_StoreU8(0x800B0DBBu, hires);
    PE_StoreU16(MV1A_S + 26u, 0xBEEFu);
    PE_StoreU16(MV1A_S + 34u, 0xBEEFu);
}

static void test_MV1A_size_unchanged_latches_x(void)
{
    TEST("MV1A_size_unchanged_latches_x");
    MV1A_Seed(0u);
    ASSERT(func_80191B64(MV1A_S) == (int)MV1A_BASE, "poll did not return the slice address");
    ASSERT(PE_LoadU16(MV1A_BASE) == 4u, "slot not promoted to 4");
    ASSERT(PE_LoadU16(MV1A_S + 26u) == 320u && PE_LoadU16(MV1A_S + 34u) == 320u,
           "x0/x1 not latched when the frame size is unchanged (0x80191D38 skipped)");
    ASSERT(PE_LoadU16(MV1A_S + 28u) == 240u && PE_LoadU16(MV1A_S + 36u) == 240u &&
           PE_LoadU16(MV1A_S + 46u) == 240u, "y copy differs");
    MV1A_Seed(1u);
    ASSERT(func_80191B64(MV1A_S) == (int)MV1A_BASE, "hi-res poll did not return the slice address");
    ASSERT(PE_LoadU16(MV1A_S + 26u) == 480u && PE_LoadU16(MV1A_S + 34u) == 480u,
           "hi-res x0/x1 not latched as width*3/2");
    PASS();
}
