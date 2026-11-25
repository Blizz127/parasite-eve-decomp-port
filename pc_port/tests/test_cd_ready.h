/* libcd CD_ready 0x8007B290 (pe_plat_cd_ready_poll, pe_plat_cd_ready.c) and the
 * libds-block transfer-step gate 0x80081110 (pe_plat_ds_81110_value,
 * pe_plat_81110.c; cdgen36): production-path checks.  The black-box vectors
 * replace every callee with a model; here the real VSync, CheckCallback and
 * getintr providers run on the suite's host state (device detached, no
 * callback pending), so only the arms that need no CD hardware are driven. */
#include "pe_plat/cd.h"
#include "psx_compat.h"

static void test_CDREADY_nonblocking_empty(void)
{
    uint32_t r;
    TEST("CDREADY_nonblocking_empty");
    ResetTestState();
    PE_StoreU8(0x8009B295u, 0u);
    PE_StoreU8(0x8009B296u, 0u);
    r = (uint32_t)pe_plat_cd_ready_poll(1u, 0x80100000u);
    ASSERT(r == 0u, "nothing ready and mode nonzero returns 0");
    ASSERT(PE_LoadU32(0x800A3480u) == 0x80011BA8u, "name word stored");
    ASSERT(PE_LoadU32(0x800A347Cu) == 1u, "one pass counted");
    PASS();
}

static void test_CDREADY_ready_and_sync_bytes(void)
{
    uint32_t j, r;
    TEST("CDREADY_ready_and_sync_bytes");
    ResetTestState();
    for (j = 0; j < 8u; j++) {
        PE_StoreU8(0x800A3470u + j, (uint8_t)(0x11u + j));
        PE_StoreU8(0x800A3468u + j, (uint8_t)(0x21u + j));
        PE_StoreU8(0x80100000u + j, 0u);
        PE_StoreU8(0x80100010u + j, 0u);
    }
    PE_StoreU8(0x8009B296u, 0x55u);
    PE_StoreU8(0x8009B295u, 0x66u);
    r = (uint32_t)pe_plat_cd_ready_poll(0u, 0x80100000u);
    ASSERT(r == 0x55u, "the ready byte wins and is returned");
    ASSERT(PE_LoadU8(0x8009B296u) == 0u, "ready byte cleared");
    ASSERT(PE_LoadU8(0x8009B295u) == 0x66u, "sync byte untouched");
    for (j = 0; j < 8u; j++)
        ASSERT(PE_LoadU8(0x80100000u + j) == 0x11u + j, "ready response copied");
    r = (uint32_t)pe_plat_cd_ready_poll(1u, 0u);
    ASSERT(r == 0x66u, "then the sync byte is returned");
    ASSERT(PE_LoadU8(0x8009B295u) == 0u, "sync byte cleared");
    r = (uint32_t)pe_plat_cd_ready_poll(1u, 0x80100010u);
    ASSERT(r == 0u, "nothing left");
    for (j = 0; j < 8u; j++)
        ASSERT(PE_LoadU8(0x80100010u + j) == 0u, "no copy without a ready byte");
    PE_StoreU8(0x8009B295u, 0x77u);
    r = (uint32_t)pe_plat_cd_ready_poll(1u, 0x80100010u);
    ASSERT(r == 0x77u, "sync byte again");
    for (j = 0; j < 8u; j++)
        ASSERT(PE_LoadU8(0x80100010u + j) == 0x21u + j, "sync response copied");
    PASS();
}

static void test_CDREADY_stop_exit(void)
{
    uint32_t r;
    TEST("CDREADY_stop_exit");
    ResetTestState();
    PE_StoreU8(0x8009B295u, 0u);
    PE_StoreU8(0x8009B296u, 0u);
    PE_Port_RequestStop(PE_PORT_STOP_FRAME_LIMIT);
    r = (uint32_t)pe_plat_cd_ready_poll(0u, 0u);
    ASSERT(r == 0xFFFFFFFFu, "a requested stop ends the blocking wait with -1");
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_FRAME_LIMIT, "the earlier stop is kept");
    PE_Port_RunControlReset();
    PASS();
}

static void test_STEPGATE_81110_counts_down(void)
{
    uint32_t now, r;
    TEST("STEPGATE_81110_counts_down");
    ResetTestState();
    now = (uint32_t)func_80073A44(-1);
    PE_StoreU32(0x8009B6ACu, 3u);
    PE_StoreU32(0x8009B6B0u, 0x80100000u);
    PE_StoreU32(0x8009B6B4u, 5u);
    PE_StoreU32(0x8009B6C4u, now);
    r = (uint32_t)pe_plat_ds_81110_value();
    ASSERT(PE_LoadU32(0x8009B6B0u) == 0x8010000Cu, "cursor advanced by three words");
    ASSERT(PE_LoadU32(0x8009B6B4u) == 4u, "counter decremented");
    ASSERT(r >= now && r - now < 64u, "within the deadline a VSync value is returned");
    PASS();
}
