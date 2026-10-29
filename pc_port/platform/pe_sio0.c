/* SIO0 register model; contract and psx-spx quotes in pe_sio0.h. */
#include "pe_sio0.h"
#include "pe_irq.h"
#include <string.h>
#include <stdlib.h>

#define SR_TXRDY 0x0001u
#define SR_RXRDY 0x0002u
#define SR_TXU   0x0004u
#define SR_IRQ   0x0200u

static uint8_t fifo[8];
static unsigned fifo_n;
static uint16_t stat = SR_TXRDY | SR_TXU, mode, ctrl, baud;
static int tx_pending; /* one TX holding slot; transmitted value sees no device */

void PE_Sio0_Reset(void)
{
    fifo_n = 0;
    memset(fifo,0,sizeof fifo);
    tx_pending = 0;
    stat = SR_TXRDY | SR_TXU;     /* TX FIFO empty, transmitter idle */
    mode = ctrl = baud = 0;
}

static void update_rxrdy(void)
{
    stat = (uint16_t)((stat & ~SR_RXRDY) | (fifo_n ? SR_RXRDY : 0u));
}

static void update_irq(void)
{
    unsigned threshold=1u<<((ctrl>>8u)&3u);
    int requested=((ctrl&0x0400u) && (stat&(SR_TXRDY|SR_TXU))) ||
                  ((ctrl&0x0800u) && fifo_n>=threshold);
    /* DSRIEN alone cannot trigger without an attached device's /ACK.
     * TX/RX conditions are independent. SR.IRQ is sticky; only its rising
     * edge latches I_STAT, regardless of I_MASK. ERRRST permits a new edge
     * immediately if the enabled condition remains true. */
    if(requested && !(stat&SR_IRQ)) {
        stat|=SR_IRQ;
        PE_IRQ_AssertSources(0x0080u);
    }
}

static void transmit_pending(void)
{
    /* The transfer needs TXEN; SIO0 receives when /CS is low (CTRL.1) or
     * RXEN forces a single byte (CTRL.2).  Nothing attached: FFh. */
    if (!tx_pending || !(ctrl & 0x0001u))
        return;
    tx_pending = 0;
    if (ctrl & 0x0006u) {
        /* Incoming data fills every unused slot, not just the next
         * counted entry. A full FIFO overwrites its last byte. SIO0 has
         * no readable overrun flag. No device attached: all bits high. */
        unsigned first=fifo_n<8u?fifo_n:7u;
        for(unsigned i=first;i<8u;i++) fifo[i]=0xFFu;
        if(fifo_n<8u) fifo_n++;
        /* RXEN forces one received byte, including when the FIFO is
         * full. /CS remains an independent receive-enable condition. */
        ctrl &= (uint16_t)~0x0004u;
    }
    stat |= SR_TXRDY | SR_TXU;
    update_rxrdy();
}

void PE_Sio0_WriteData(uint8_t value)
{
    (void)value;                  /* sent into the High-Z line: no device */
    /* A full holding slot is overwritten, not discarded or queued again.
     * If TXEN is clear, retain it until a later control write enables TX. */
    tx_pending = 1;
    stat &= (uint16_t)~(SR_TXRDY | SR_TXU);
    transmit_pending();
    update_irq();
}

uint8_t PE_Sio0_PeekData(void)
{
    /* Uncounted slots retain the last received byte until shifted out. */
    return fifo[0];
}

uint8_t PE_Sio0_ReadData(void)
{
    uint8_t v = PE_Sio0_PeekData();
    for(unsigned i=1;i<8u;i++) fifo[i-1u]=fifo[i];
    fifo[7]=0;
    if(fifo_n) fifo_n--;
    update_rxrdy();
    return v;
}

uint32_t PE_Sio0_ReadDataWide(unsigned bytes)
{
    if(bytes!=2u && bytes!=4u) abort(); /* invalid host API width */
    uint32_t value=0;
    for(unsigned i=0;i<bytes;i++) value|=(uint32_t)fifo[i]<<(8u*i);
    /* Halfword read previews entry 1 but pops only entry 0. Word reads
     * remove four entries, including retained slots after RXRDY clears. */
    unsigned pops=bytes==4u?4u:1u;
    for(unsigned i=0;i<pops;i++) (void)PE_Sio0_ReadData();
    return value;
}

uint16_t PE_Sio0_ReadStat(void) { return stat; }
uint16_t PE_Sio0_ReadMode(void) { return mode; }
/* Stop-bit selection (bits 6..7) belongs to SIO1, not this SIO0 port. */
void PE_Sio0_WriteMode(uint16_t value) { mode = (uint16_t)(value & 0x013Fu); }
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
    transmit_pending();
    update_irq();
}
