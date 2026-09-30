/* SIO0 register model; contract and psx-spx quotes in pe_sio0.h. */
#include "pe_sio0.h"

#define SR_TXRDY 0x0001u
#define SR_RXRDY 0x0002u
#define SR_TXU   0x0004u
#define SR_IRQ   0x0200u

static uint8_t fifo[8];
static unsigned fifo_n;
static uint16_t stat = SR_TXRDY | SR_TXU, mode, ctrl, baud;

void PE_Sio0_Reset(void)
{
    fifo_n = 0;
    stat = SR_TXRDY | SR_TXU;     /* TX FIFO empty, transmitter idle */
    mode = ctrl = baud = 0;
}

static void update_rxrdy(void)
{
    stat = (uint16_t)((stat & ~SR_RXRDY) | (fifo_n ? SR_RXRDY : 0u));
}

void PE_Sio0_WriteData(uint8_t value)
{
    (void)value;                  /* sent into the High-Z line: no device */
    /* The transfer needs TXEN; SIO0 receives when /CS is low (CTRL.1) or
     * RXEN forces a single byte (CTRL.2).  Nothing attached: FFh. */
    if (!(ctrl & 0x0001u))
        return;
    if ((ctrl & 0x0006u) && fifo_n < 8u)
        fifo[fifo_n++] = 0xFFu;
    stat |= SR_TXRDY | SR_TXU;
    update_rxrdy();
}

uint8_t PE_Sio0_PeekData(void)
{
    /* Reading an empty FIFO is not described by psx-spx; the only byte that
     * can ever enter the FIFO under the no-device model is FFh. */
    return fifo_n ? fifo[0] : 0xFFu;
}

uint8_t PE_Sio0_ReadData(void)
{
    uint8_t v = PE_Sio0_PeekData();
    if (fifo_n) {
        for (unsigned i = 1; i < fifo_n; i++)
            fifo[i - 1] = fifo[i];
        fifo_n--;
    }
    update_rxrdy();
    return v;
}

uint16_t PE_Sio0_ReadStat(void) { return stat; }
uint16_t PE_Sio0_ReadMode(void) { return mode; }
void PE_Sio0_WriteMode(uint16_t value) { mode = (uint16_t)(value & 0x01FFu); }
uint16_t PE_Sio0_ReadCtrl(void) { return ctrl; }
uint16_t PE_Sio0_ReadBaud(void) { return baud; }
void PE_Sio0_WriteBaud(uint16_t value) { baud = value; }

void PE_Sio0_WriteCtrl(uint16_t value)
{
    if (value & 0x0040u) {        /* INTRST: reset most registers to zero */
        PE_Sio0_Reset();
        value &= (uint16_t)~0x0040u;
    }
    if (value & 0x0010u)          /* ERRRST: reset SR bits 3,4,5,9 */
        stat &= (uint16_t)~(0x0008u | 0x0010u | 0x0020u | SR_IRQ);
    /* W-only bits 4 and 6 do not read back; 14-15 always zero. */
    ctrl = (uint16_t)(value & 0x3FAFu);
}
