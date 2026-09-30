/*
 * SIO0 model (pc_port/platform/pe_sio0.h) and the memory-card byte exchange
 * leaves func_800830DC / func_800832B4 (decomp_hand/sio_port.c).
 * Expectations: psx-spx "Serial Interfaces (SIO)" (quoted in pe_sio0.h) and
 * the matched src/func_800830DC.c / src/func_800832B4.c.
 * Guest scratch 0x80178000..0x8017BFFF.
 */
#include "pe_sio0.h"
#include "pe_rcnt2.h"
#define PA_SIO_S 0x80178000u        /* card record a0 */
#define PA_SIO_B 0x80178400u        /* a0->+0x3C frame buffer */
#define PA_SIO_C 0x80178800u        /* a0->+0x40 byte */

#define PA_SIO_EXPECT_ABORT(body, msg) do { \
    pid_t sio_pid = fork(); int sio_st = 0; \
    ASSERT(sio_pid >= 0, "fork failed"); \
    if (sio_pid == 0) { freopen("/dev/null", "w", stderr); body; _exit(99); } \
    ASSERT(waitpid(sio_pid, &sio_st, 0) == sio_pid && WIFSIGNALED(sio_st) && \
           WTERMSIG(sio_st) == SIGABRT, msg); \
} while (0)

static void pa_sio_reset(void)
{
    ResetTestState();
    PE_Sio0_Reset();
    PE_Rcnt2_Reset();
    PE_MMIO_Reset();
    PE_Decomp_ResetBoundaries();
    Bootstrap_ResetArg4CallLog();
}

/* psx-spx register semantics as modelled; nothing attached -> FFh, no /ACK */
static void test_PA_sio_registers(void)
{
    TEST("PA_sio_registers");
    pa_sio_reset();
    ASSERT(PE_LoadU16(0x1F801044u) == 0x0005u, "reset: TXRDY | TXU, RX empty");
    PE_StoreU16(0x1F801048u, 0x000Du);
    PE_StoreU16(0x1F80104Eu, 0x0088u);
    ASSERT(PE_LoadU16(0x1F801048u) == 0x000Du && PE_LoadU16(0x1F80104Eu) == 0x0088u,
           "MODE / BAUD read back");
    PE_StoreU16(0x1F801048u, 0xFFFFu);
    ASSERT(PE_LoadU16(0x1F801048u) == 0x01FFu, "MODE bits 9-15 always zero");
    /* TXEN off: no transfer */
    PE_StoreU16(0x1F80104Au, 0x0002u);
    PE_StoreU8(0x1F801040u, 0x81u);
    ASSERT(!(PE_LoadU16(0x1F801044u) & 2u), "no TXEN: nothing received");
    /* TXEN + DTR (/CS low): the byte goes out, FFh (High-Z) comes back */
    PE_StoreU16(0x1F80104Au, 0x1003u);
    ASSERT(PE_LoadU16(0x1F80104Au) == 0x1003u, "CTRL reads back");
    PE_StoreU8(0x1F801040u, 0x81u);
    ASSERT((PE_LoadU16(0x1F801044u) & 0x0007u) == 0x0007u, "TXRDY, RXRDY, TXU after the byte");
    ASSERT(!(PE_LoadU16(0x1F801044u) & 0x0280u), "no /ACK: DSR and IRQ stay clear");
    ASSERT(PE_LoadU8(0x1F801040u) == 0xFFu, "nothing attached -> FFh");
    ASSERT(!(PE_LoadU16(0x1F801044u) & 2u), "RXRDY clears after the read");
    ASSERT(!(PE_LoadU16(0x1F801070u) & 0x80u), "IRQ7 never asserted");
    /* TXEN + RXEN without /CS: RXEN forces receiving the byte */
    PE_StoreU16(0x1F80104Au, 0x0005u);
    PE_StoreU8(0x1F801040u, 0x00u);
    ASSERT(PE_LoadU16(0x1F801044u) & 2u, "RXEN forces a received byte");
    /* INTRST (bit 6) resets the FIFO and registers; W bits don't read back */
    PE_StoreU16(0x1F80104Au, 0x0040u);
    ASSERT(PE_LoadU16(0x1F801044u) == 0x0005u && PE_LoadU16(0x1F80104Au) == 0u &&
           PE_LoadU16(0x1F801048u) == 0u, "INTRST resets");
    PE_StoreU16(0x1F80104Au, 0x1013u);
    ASSERT(PE_LoadU16(0x1F80104Au) == 0x1003u, "ERRRST (bit 4) is write-only");
    /* unmodelled SIO registers stay loud */
    PA_SIO_EXPECT_ABORT(PE_StoreU16(0x1F801044u, 0u), "STAT is read-only");
    PA_SIO_EXPECT_ABORT((void)PE_LoadU16(0x1F80104Cu), "SIO0 0x1F80104C aborts");
    PA_SIO_EXPECT_ABORT((void)PE_LoadU32(0x1F801040u), "DATA 32-bit preview aborts");
    PASS();
}

/* Fixture: SIO0 / I_STAT at their retail addresses; card record. */
static void pa_sio_card(void)
{
    PE_StoreU32(0x8009B788u, 0x1F801040u);
    PE_StoreU32(0x8009B784u, 0x1F801070u);
    PE_StoreU16(0x1F80104Au, 0x1003u);           /* TXEN | DTR | DSRIEN */
    PE_StoreU32(PA_SIO_S + 0x3Cu, PA_SIO_B);
    PE_StoreU32(PA_SIO_S + 0x40u, PA_SIO_C);
    PE_Rcnt2_WriteTarget(0x44E8u);
    PE_Rcnt2_WriteMode(0x258u);                  /* the retail RCNT2 program */
}

/* src/func_800830DC.c */
static void test_PA_sio_830DC(void)
{
    TEST("PA_sio_830DC");
    pa_sio_reset();
    pa_sio_card();
    /* a1 < 0: r = DATA (empty -> FFh), +0x44 = 0xFF, +0x45 = 1,
     * *(+0x40) = ~a1, wait TXRDY, RCNT2 timeout, DATA = ~a1. */
    PE_StoreU32(0x800BD02Cu, 0x3Cu);
    PE_StoreU32(0x800A76D0u, PE_Rcnt2_ReadCounter());
    ASSERT(func_800830DC(PA_SIO_S, -2) == 0xFF, "a1 < 0 returns the DATA byte");
    ASSERT(PE_LoadU8(PA_SIO_S + 0x44u) == 0xFFu && PE_LoadU8(PA_SIO_S + 0x45u) == 1u &&
           PE_LoadU8(PA_SIO_C) == 1u, "a1 < 0 record fields, *(+0x40) = ~a1");
    ASSERT(PE_LoadU16(0x1F801044u) & 2u, "the ~a1 byte went out (FFh received)");
    ASSERT(!PE_Port_ShouldStop(), "the RCNT2 wait ended by timeout");
    /* a1 >= 0: timeout 0x1AE on RCNT2, BAUD = 0x88, no /ACK -> -0x14 */
    PE_StoreU8(PA_SIO_B, 0x51u);
    PE_StoreU8(PA_SIO_S + 0x44u, 0u);
    PE_StoreU8(PA_SIO_S + 0x45u, 0u);
    ASSERT(func_800830DC(PA_SIO_S, 0x42) == -0x14, "no /ACK -> RCNT2 timeout -0x14");
    ASSERT(PE_LoadU32(0x800BD02Cu) == 0x1AEu, "timeout limit 0x1AE");
    ASSERT(PE_LoadU16(0x1F80104Eu) == 0x88u, "BAUD = 0x88");
    ASSERT(PE_LoadU8(PA_SIO_S + 0x45u) == 0u, "timeout: no record update");
    /* mode 0x22 when the frame's first byte has high nibble 8 and +0x44 >= 9 */
    PE_StoreU8(PA_SIO_B, 0x81u);
    PE_StoreU8(PA_SIO_S + 0x44u, 9u);
    PE_StoreU8(0x1F801040u, 0u);                 /* a byte to collect */
    ASSERT(func_800830DC(PA_SIO_S, 0x42) == -0x14 && PE_LoadU16(0x1F80104Eu) == 0x22u,
           "BAUD = 0x22 for a data frame");
    PASS();
}

/* src/func_800832B4.c */
static void test_PA_sio_832B4(void)
{
    TEST("PA_sio_832B4");
    pa_sio_reset();
    pa_sio_card();
    PE_StoreU8(PA_SIO_B, 0x51u);
    PE_StoreU8(0x1F801040u, 0u);                 /* the previous send's byte */
    ASSERT(func_800832B4(PA_SIO_S, 0x33) == -2, "no /ACK -> inline RCNT2 timeout -2");
    ASSERT(PE_LoadU32(0x800BD02Cu) == 0x190u, "func_80084FC4(0x190)");
    ASSERT(PE_LoadU16(0x1F80104Eu) == 0x88u,
           "r = FFh (nibble F != 8) -> BAUD = mode (0x88 for frame byte 0x51)");
    /* a0[0x44] == 0 and r nibble 8 -> BAUD 0x22: not reachable with no
     * device (r is always FFh); covered by the mode rule above. */
    PASS();
}

static void test_PA_sio_all(void)
{
    test_PA_sio_registers();
    test_PA_sio_830DC();
    test_PA_sio_832B4();
}
