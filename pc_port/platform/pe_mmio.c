/* MMIO dispatch table; contract in pe_mmio.h. */
#include "pe_mmio.h"
#include "pe_irq.h"
#include "pe_gpu.h"
#include "pe_timer1.h"
#include "pe_rcnt2.h"
#include "pe_sio0.h"
#include "pe_spu_dma.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t addr;          /* canonical 0x1F80xxxx */
    uint32_t width;         /* register width in bytes (2 or 4) */
    const char *name;
    uint32_t (*read)(void);         /* value read (may have side effects) */
    uint32_t (*peek)(void);         /* side-effect-free read (shadow) */
    int (*write)(uint32_t value);   /* 0 = value not modelled -> abort */
} PeMmioReg;

static uint32_t r_istat(void) { return PE_IRQ_ReadStatus(); }
static int w_istat(uint32_t v) { PE_IRQ_WriteStatus((uint16_t)v); return 1; }
static uint32_t r_imask(void) { return PE_IRQ_GetMask(); }
static int w_imask(uint32_t v) { (void)PE_IRQ_ExchangeMask((uint16_t)v); return 1; }
static uint32_t r_dpcr(void) { return PE_GPU_ReadDPCR(); }
static int w_dpcr(uint32_t v) { PE_GPU_WriteDPCR(v); return 1; }
static uint32_t r_dicr(void) { return PE_GPU_ReadDICR(); }
static int w_dicr(uint32_t v) { PE_GPU_WriteDICR(v); return 1; }
static uint32_t r_t1cnt(void) { return PE_Timer1_ReadCounter(); }
static uint32_t r_t1mode(void) { return PE_Timer1_ReadMode(); }
static uint32_t p_t1mode(void) { return PE_Timer1_PeekMode(); }
/* pe_timer1 models exactly the retail VBlank-counter program 0x107
 * (func_800743B4 -> 0x1F801114); any other RCNT1 mode is not modelled. */
static int w_t1mode(uint32_t v)
{
    if ((v & 0xFFFFu) != 0x107u)
        return 0;
    PE_Timer1_InitializeVBlankCounter();
    return 1;
}
static uint32_t r_t2cnt(void) { return PE_Rcnt2_ReadCounter(); }
static int w_t2cnt(uint32_t v) { PE_Rcnt2_WriteCounter((uint16_t)v); return 1; }
static uint32_t r_t2mode(void) { return PE_Rcnt2_ReadMode(); }
static uint32_t p_t2mode(void) { return PE_Rcnt2_PeekMode(); }
static int w_t2mode(uint32_t v) { PE_Rcnt2_WriteMode((uint16_t)v); return 1; }
static uint32_t r_t2tgt(void) { return PE_Rcnt2_ReadTarget(); }
static int w_t2tgt(uint32_t v) { PE_Rcnt2_WriteTarget((uint16_t)v); return 1; }

static uint32_t r_s0data(void) { return PE_Sio0_ReadData(); }
static uint32_t p_s0data(void) { return PE_Sio0_PeekData(); }
static int w_s0data(uint32_t v) { PE_Sio0_WriteData((uint8_t)v); return 1; }
static uint32_t r_s0stat(void) { return PE_Sio0_ReadStat(); }
static uint32_t r_s0mode(void) { return PE_Sio0_ReadMode(); }
static int w_s0mode(uint32_t v) { PE_Sio0_WriteMode((uint16_t)v); return 1; }
static uint32_t r_s0ctrl(void) { return PE_Sio0_ReadCtrl(); }
static int w_s0ctrl(uint32_t v) { PE_Sio0_WriteCtrl((uint16_t)v); return 1; }
static uint32_t r_s0baud(void) { return PE_Sio0_ReadBaud(); }
static int w_s0baud(uint32_t v) { PE_Sio0_WriteBaud((uint16_t)v); return 1; }

/* THE table of modelled registers.  Keep pe_mmio.h's list in sync. */
static const PeMmioReg g_pe_mmio_regs[] = {
    {0x1F801040u, 1, "SIO0 DATA",   r_s0data, p_s0data, w_s0data},
    {0x1F801044u, 2, "SIO0 STAT",   r_s0stat, r_s0stat, NULL},
    {0x1F801048u, 2, "SIO0 MODE",   r_s0mode, r_s0mode, w_s0mode},
    {0x1F80104Au, 2, "SIO0 CTRL",   r_s0ctrl, r_s0ctrl, w_s0ctrl},
    {0x1F80104Eu, 2, "SIO0 BAUD",   r_s0baud, r_s0baud, w_s0baud},
    {0x1F801070u, 2, "I_STAT",      r_istat,  r_istat,  w_istat},
    {0x1F801074u, 2, "I_MASK",      r_imask,  r_imask,  w_imask},
    {0x1F8010F0u, 4, "DPCR",        r_dpcr,   r_dpcr,   w_dpcr},
    {0x1F8010F4u, 4, "DICR",        r_dicr,   r_dicr,   w_dicr},
    {0x1F801110u, 2, "RCNT1 count", r_t1cnt,  r_t1cnt,  NULL},
    {0x1F801114u, 2, "RCNT1 mode",  r_t1mode, p_t1mode, w_t1mode},
    {0x1F801120u, 2, "RCNT2 count", r_t2cnt,  r_t2cnt,  w_t2cnt},
    {0x1F801124u, 2, "RCNT2 mode",  r_t2mode, p_t2mode, w_t2mode},
    {0x1F801128u, 2, "RCNT2 target",r_t2tgt,  r_t2tgt,  w_t2tgt},
};
#define PE_MMIO_NREGS (sizeof(g_pe_mmio_regs) / sizeof(g_pe_mmio_regs[0]))

/* Host-pointer shadow blocks (see pe_mmio.h). */
typedef struct { uint32_t base, size; } PeMmioBlock;
static const PeMmioBlock g_blocks[] = {
    {0x1F801040u, 0x10u},
    {0x1F801070u, 0x08u}, {0x1F8010F0u, 0x08u}, {0x1F801100u, 0x30u},
};
#define PE_MMIO_NBLOCKS (sizeof(g_blocks) / sizeof(g_blocks[0]))
#define SHADOW_BASE 0x1F801000u
static uint8_t g_shadow[0x200];
static uint8_t g_snapshot[0x200];
static int g_shadow_live;

static uint32_t canon(pe_addr_t a)
{
    uint32_t seg = a & 0xE0000000u;
    if (seg == 0x80000000u || seg == 0xA0000000u)
        return a & 0x1FFFFFFFu;
    return a;
}

/* SPU register file 0x1F801C00..0x1F801DFF (pe_spu_dma.h
 * PE_SpuRegister_{Load,Store}U16 -- the same authority the translated SPU
 * code and the audio lane's interpreter use).  Halfword registers; a 32-bit
 * access is two halfwords, low first. */
#define SPU_BASE 0x1F801C00u
#define SPU_SIZE 0x200u
static int is_spu(uint32_t a, size_t size)
{
    return a >= SPU_BASE && a - SPU_BASE < SPU_SIZE &&
           size <= SPU_SIZE - (a - SPU_BASE);
}
static uint8_t g_spu_shadow[SPU_SIZE];
static uint8_t g_spu_snapshot[SPU_SIZE];
static int g_spu_shadow_live;
/* KON/KOFF (0x188..0x18F) trigger on every write, including a repeat of the
 * value last written, so their shadow slots refresh as 0 (a zero write is a
 * no-op on hardware) and any non-zero pointer write is committed. */
static int spu_trigger_reg(uint32_t off) { return off >= 0x188u && off < 0x190u; }

static void spu_commit(void)
{
    if (!g_spu_shadow_live)
        return;
    g_spu_shadow_live = 0;
    for (uint32_t off = 0; off < SPU_SIZE; off += 2u) {
        uint16_t now = (uint16_t)(g_spu_shadow[off] | (g_spu_shadow[off + 1u] << 8));
        uint16_t was = (uint16_t)(g_spu_snapshot[off] | (g_spu_snapshot[off + 1u] << 8));
        if (now != was)
            PE_SpuRegister_StoreU16(off, now);
    }
}

static void spu_refresh(void)
{
    for (uint32_t off = 0; off < SPU_SIZE; off += 2u) {
        uint16_t v = spu_trigger_reg(off) ? 0u : PE_SpuRegister_LoadU16(off);
        g_spu_shadow[off] = (uint8_t)v;
        g_spu_shadow[off + 1u] = (uint8_t)(v >> 8);
    }
    memcpy(g_spu_snapshot, g_spu_shadow, sizeof(g_spu_shadow));
    g_spu_shadow_live = 1;
}

static uint32_t spu_load(uint32_t a, size_t size)
{
    uint32_t off = a - SPU_BASE;
    if (size == 2 && !(off & 1u))
        return PE_SpuRegister_LoadU16(off);
    if (size == 4 && !(off & 3u))
        return PE_SpuRegister_LoadU16(off) | ((uint32_t)PE_SpuRegister_LoadU16(off + 2u) << 16);
    if (size == 1) {
        uint16_t h = PE_SpuRegister_LoadU16(off & ~1u);
        return (off & 1u) ? (uint32_t)(h >> 8) : (uint32_t)(h & 0xFFu);
    }
    return 0xFFFFFFFFu;   /* unaligned: caller aborts */
}

static int spu_store(uint32_t a, size_t size, uint32_t v)
{
    uint32_t off = a - SPU_BASE;
    if (size == 2 && !(off & 1u)) { PE_SpuRegister_StoreU16(off, (uint16_t)v); return 1; }
    if (size == 4 && !(off & 3u)) {
        PE_SpuRegister_StoreU16(off, (uint16_t)v);
        PE_SpuRegister_StoreU16(off + 2u, (uint16_t)(v >> 16));
        return 1;
    }
    return 0;   /* byte / unaligned SPU writes are not modelled */
}

static void mmio_fatal(const char *what, pe_addr_t a, size_t size)
{
    fprintf(stderr, "FATAL: MMIO %s: address 0x%08X size %zu is not a "
            "modelled register (pe_mmio.h)\n", what, a, size);
    abort();
}

static const PeMmioReg *find_reg(uint32_t a, size_t size)
{
    for (size_t i = 0; i < PE_MMIO_NREGS; i++) {
        const PeMmioReg *r = &g_pe_mmio_regs[i];
        /* whole-register access, or a 32-bit access to a 16-bit register */
        if (a == r->addr && (size == r->width || (size == 4 && r->width == 2)))
            return r;
    }
    return NULL;
}

int PE_MMIO_IsModelled(pe_addr_t address, size_t size)
{
    uint32_t a = canon(address);
    if (is_spu(a, size))
        return (size == 2 && !(a & 1u)) || (size == 4 && !(a & 3u)) || size == 1;
    return find_reg(a, size) != NULL;
}

int PE_MMIO_InShadowBlock(pe_addr_t address, size_t size)
{
    uint32_t a = canon(address);
    if (is_spu(a, size))
        return 1;
    for (size_t i = 0; i < PE_MMIO_NBLOCKS; i++)
        if (a >= g_blocks[i].base && a - g_blocks[i].base < g_blocks[i].size &&
            size <= g_blocks[i].size - (a - g_blocks[i].base))
            return 1;
    return 0;
}

uint32_t PE_MMIO_Load(pe_addr_t address, size_t size)
{
    if (is_spu(canon(address), size)) {
        uint32_t v;
        PE_MMIO_Commit();
        v = spu_load(canon(address), size);
        if (v == 0xFFFFFFFFu && size != 4)
            mmio_fatal("load (unaligned SPU)", address, size);
        return v;
    }
    const PeMmioReg *r = find_reg(canon(address), size);
    if (!r || !r->read)
        mmio_fatal("load", address, size);
    PE_MMIO_Commit();
    uint32_t v = r->read();
    return r->width == 1 ? (v & 0xFFu) : r->width == 2 ? (v & 0xFFFFu) : v;
}

void PE_MMIO_Store(pe_addr_t address, size_t size, uint32_t value)
{
    if (is_spu(canon(address), size)) {
        PE_MMIO_Commit();
        if (!spu_store(canon(address), size, value))
            mmio_fatal("store (byte/unaligned SPU)", address, size);
        return;
    }
    const PeMmioReg *r = find_reg(canon(address), size);
    if (!r || !r->write)
        mmio_fatal("store", address, size);
    PE_MMIO_Commit();
    if (!r->write(r->width == 1 ? (value & 0xFFu) :
                  r->width == 2 ? (value & 0xFFFFu) : value))
        mmio_fatal("store (value not modelled)", address, size);
}

static int is_reg_byte(uint32_t a)
{
    for (size_t i = 0; i < PE_MMIO_NREGS; i++)
        if (a >= g_pe_mmio_regs[i].addr &&
            a < g_pe_mmio_regs[i].addr + g_pe_mmio_regs[i].width)
            return 1;
    return 0;
}

static uint32_t shadow_get(const uint8_t *buf, uint32_t a, uint32_t w)
{
    uint32_t v = 0;
    for (uint32_t i = 0; i < w; i++)
        v |= (uint32_t)buf[a - SHADOW_BASE + i] << (8 * i);
    return v;
}

static void shadow_put(uint8_t *buf, uint32_t a, uint32_t w, uint32_t v)
{
    for (uint32_t i = 0; i < w; i++)
        buf[a - SHADOW_BASE + i] = (uint8_t)(v >> (8 * i));
}

void PE_MMIO_Commit(void)
{
    spu_commit();
    if (!g_shadow_live)
        return;
    g_shadow_live = 0;
    for (size_t b = 0; b < PE_MMIO_NBLOCKS; b++)
        for (uint32_t a = g_blocks[b].base; a < g_blocks[b].base + g_blocks[b].size; a++)
            if (!is_reg_byte(a) &&
                g_shadow[a - SHADOW_BASE] != g_snapshot[a - SHADOW_BASE])
                mmio_fatal("host-pointer write", a, 1);
    for (size_t i = 0; i < PE_MMIO_NREGS; i++) {
        const PeMmioReg *r = &g_pe_mmio_regs[i];
        uint32_t now = shadow_get(g_shadow, r->addr, r->width);
        if (now == shadow_get(g_snapshot, r->addr, r->width))
            continue;
        if (!r->write || !r->write(now))
            mmio_fatal("host-pointer write (not modelled)", r->addr, r->width);
    }
}

static void refresh(void)
{
    memset(g_shadow, 0xDE, sizeof(g_shadow));
    for (size_t i = 0; i < PE_MMIO_NREGS; i++) {
        const PeMmioReg *r = &g_pe_mmio_regs[i];
        uint32_t v = r->peek ? r->peek() : 0u;
        shadow_put(g_shadow, r->addr, r->width,
                   r->width == 1 ? (v & 0xFFu) : r->width == 2 ? (v & 0xFFFFu) : v);
    }
    memcpy(g_snapshot, g_shadow, sizeof(g_shadow));
    g_shadow_live = 1;
}

void *PE_MMIO_Translate(pe_addr_t address, size_t size)
{
    uint32_t a = canon(address);
    if (!PE_MMIO_InShadowBlock(address, size))
        mmio_fatal("translate", address, size);
    PE_MMIO_Commit();
    if (is_spu(a, size)) {
        spu_refresh();
        return &g_spu_shadow[a - SPU_BASE];
    }
    refresh();
    return &g_shadow[a - SHADOW_BASE];
}

void PE_MMIO_Reset(void)
{
    g_spu_shadow_live = 0;
    g_shadow_live = 0;
    memset(g_shadow, 0xDE, sizeof(g_shadow));
    memcpy(g_snapshot, g_shadow, sizeof(g_shadow));
}
