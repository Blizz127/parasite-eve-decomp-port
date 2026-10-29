/*
 * Phase 6E-B53B — bounded deterministic GPU/DMA2 hardware substrate.
 *
 * This module owns hardware state only.  The retail libgpu command ring,
 * producer/consumer indices, copied packets, and callback slots remain in
 * guest RAM and are intentionally absent here.
 */
#ifndef PE_GPU_H
#define PE_GPU_H

#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_irq.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PE_GPU_VRAM_WIDTH       1024u
#define PE_GPU_VRAM_HEIGHT       512u
#define PE_GPU_VRAM_PIXELS      (PE_GPU_VRAM_WIDTH * PE_GPU_VRAM_HEIGHT)

#define PE_GPU_STATUS_IRQ 0x01000000u
#define PE_GPU_STATUS_DMA_REQUEST 0x02000000u
#define PE_GPU_STATUS_READY_GP0 0x04000000u
#define PE_GPU_STATUS_READY_READ 0x08000000u
#define PE_GPU_STATUS_READY_DMA 0x10000000u

#define PE_GPU_DMA2_CHCR_IDLE   0x00000401u
#define PE_GPU_DMA2_CHCR_LOAD   0x01000201u
#define PE_GPU_DMA2_CHCR_STORE  0x01000200u
#define PE_GPU_DMA2_CHCR_LIST   0x01000401u
#define PE_GPU_DMA2_CHCR_BUSY   0x01000000u
#define PE_GPU_DMA2_DPCR_ENABLE 0x00000800u
#define PE_GPU_DMA2_DICR_ENABLE 0x00040000u
#define PE_GPU_DICR_MASTER      0x00800000u
#define PE_GPU_DMA2_DICR_FLAG   0x04000000u
#define PE_GPU_DICR_FORCE       0x00008000u
#define PE_GPU_DICR_FLAGS       0x7F000000u
#define PE_GPU_DICR_MASTER_FLAG 0x80000000u

typedef enum {
    PE_GPU_GP0_IDLE = 0,
    PE_GPU_GP0_EXPECT_POSITION,
    PE_GPU_GP0_EXPECT_SIZE,
    PE_GPU_GP0_IMAGE_DATA,
    PE_GPU_GP0_RECT_EXPECT_POSITION,
    PE_GPU_GP0_RECT_EXPECT_UV_CLUT,
    PE_GPU_GP0_RECT_EXPECT_SIZE,
    PE_GPU_GP0_FILL_EXPECT_POSITION,
    PE_GPU_GP0_FILL_EXPECT_SIZE,
    PE_GPU_GP0_MONO_RECT_EXPECT_POSITION,
    PE_GPU_GP0_MONO_RECT_EXPECT_SIZE,
    PE_GPU_GP0_POLYGON_DATA,
    PE_GPU_GP0_LINE_DATA,
    PE_GPU_GP0_READ_EXPECT_POSITION,
    PE_GPU_GP0_READ_EXPECT_SIZE,
    PE_GPU_GP0_READ_DATA,
    PE_GPU_GP0_COPY_EXPECT_SOURCE,      /* GP0(80h..9Fh) VRAM-to-VRAM copy */
    PE_GPU_GP0_COPY_EXPECT_DESTINATION,
    PE_GPU_GP0_COPY_EXPECT_SIZE
} PeGpuGp0State;

/* Read-only snapshot.  Every address is a 32-bit guest address; no native
 * pointer is retained or exposed. */
typedef struct {
    uint32_t status;
    uint32_t gp1_dma_direction;
    uint8_t hblank_level,vblank_level;
    uint64_t hblank_edge_count,vblank_edge_count;
    uint32_t display_start,display_horizontal_range,display_vertical_range,display_mode;
    /* Value-only command-order evidence; never read by the device. */
    uint32_t display_commands[4];
    uint64_t display_command_count;
    PeGpuGp0State gp0_state;
    uint32_t image_x;
    uint32_t image_y;
    uint32_t image_width;
    uint32_t image_height;
    uint32_t image_current_pixel;
    uint32_t image_remaining_pixels;

    /* GP0(E1h) draw-mode state. The full command word is retained because
     * later primitive decoding consumes texture-page/depth fields. */
    uint32_t draw_mode;
    uint64_t draw_mode_count;

    /* GP0(E2h..E6h) drawing-environment registers.  Full command words are
     * retained so tests can distinguish an unset register from a command
     * whose payload is zero. */
    uint32_t texture_window;
    uint32_t drawing_area_top_left;
    uint32_t drawing_area_bottom_right;
    uint32_t drawing_offset;
    uint32_t mask_setting;
    uint64_t texture_window_count;
    uint64_t drawing_area_top_left_count;
    uint64_t drawing_area_bottom_right_count;
    uint64_t drawing_offset_count;
    uint64_t mask_setting_count;
    uint64_t nop_count;

    /* Last completed GP0(64h) variable textured rectangle. In-progress
     * packet words remain private to the parser. */
    uint32_t rectangle_command;
    uint32_t rectangle_position;
    uint32_t rectangle_uv_clut;
    uint32_t rectangle_size;
    uint64_t rectangle_count;

    /* Last completed GP0(02h) VRAM fill. Represented subset: raw 15-bit
     * color write clamped to VRAM, no mask-bit set, drawing area and
     * offset ignored. In-progress packet words stay parser-private. */
    uint32_t fill_command;
    uint32_t fill_position;
    uint32_t fill_size;
    uint64_t fill_count;

    /* Last completed GP0(60h..7Bh) untextured ("monochrome") rectangle:
     * variable (60h/62h), 1x1 (68h/6Ah), 8x8 (70h/72h), 16x16 (78h/7Ah).
     * Command bit 25 selects semi-transparency through the GP0(E1h) ABR
     * mode.  `mono_rectangle_size` holds the effective width|height<<16 for
     * fixed-size opcodes too. */
    uint32_t mono_rectangle_command;
    uint32_t mono_rectangle_position;
    uint32_t mono_rectangle_size;
    uint64_t mono_rectangle_count;

    uint64_t polygon_count;
    uint64_t polygon_pixel_count;
    uint64_t line_count;
    uint64_t line_pixel_count;

    pe_addr_t dma2_madr;
    uint32_t dma2_bcr;
    uint32_t dma2_chcr;
    uint32_t dma6_madr, dma6_bcr, dma6_chcr;
    uint64_t dma6_event_count;
    uint32_t dpcr;
    uint32_t dicr;

    int dma2_active;
    pe_addr_t dma2_source;
    uint32_t dma2_word_count;
    uint64_t dma2_event_token;

    uint32_t vsync_count;
    uint64_t dma_event_count;
    uint64_t dma_data_order;
    uint64_t dma_completion_order;
    int dicr_rising_edge_pending;

    /* Synchronous GP0(80h) VRAM-to-VRAM copies.  These fields are
     * value-only hardware telemetry; VRAM remains the sole pixel authority. */
    uint64_t move_count;
    uint32_t move_source;
    uint32_t move_destination;
    uint32_t move_size;
} PeGpuState;

/* Host lifecycle.  Init clears VRAM and hardware.  Reset cancels hardware
 * activity but deliberately preserves VRAM, matching the distinction
 * between platform initialization and a GP1/control reset. */
void PE_GPU_Init(void);
void PE_GPU_Reset(void);
/* Host presentation only. Scale 1 uses native VRAM; 2..4 own a separate
 * render surface. Changes seed it from native VRAM; allocation failure falls
 * back to 1. Init clears both surfaces, Reset preserves both. */
void PE_GPU_SetRenderScale(int scale);
int PE_GPU_GetRenderScale(void);
/* 0 nearest (default), 1 conservative bilinear on shadow polygons only.
 * Rectangles and native VRAM always retain nearest sampling. */
void PE_GPU_SetTextureFilter(int filter);
int PE_GPU_GetTextureFilter(void);
/* Shadow dithering: 0 retail (native pattern magnified, default), 1 fine
 * (pattern at shadow resolution), 2 off.  Native VRAM is never affected. */
void PE_GPU_SetShadowDither(int mode);
int PE_GPU_GetShadowDither(void);
/* Sub-pixel geometry on the shadow surface: a polygon whose every vertex
 * word has a fraction from lookup (x | y << 8, 1/256 px) is placed at it at
 * the shadow's resolution.  Other polygons, native VRAM and scale 1 are
 * unchanged.  NULL (the default) is off. */
void PE_GPU_SetSubpixelLookup(int (*lookup)(uint32_t xy, uint32_t *frac));
/* Invalidates cached host scan-out when Init or a scale change reseeds VRAM. */
uint64_t PE_GPU_GetRenderGeneration(void);
/* Coordinates are in scaled VRAM units; never wraps. */
int PE_GPU_ReadRenderPixel(uint32_t x, uint32_t y, uint16_t *pixel);

/* Status derives command/write readiness and DREQ from the synchronous
 * parser. SetReady controls host word acceptance, not command boundaries.
 * Reads never advance execution; this is not cycle-accurate FIFO timing. */
uint32_t PE_GPU_ReadStatus(void);
void PE_GPU_SetReady(int ready);

/* Supported GP0 subset: 00h NOP, 0x01000000 cache clear, 1Fh IRQ, GP0(E1h..E6h)
 * drawing-environment registers, GP0(02h) fill,
 * GP0(20h..3Fh) flat/shaded, textured/untextured triangles and quads,
 * GP0(40h..47h,50h..57h) flat/shaded single lines,
 * GP0(64h..67h,7Ch..7Fh) raw/modulated, opaque/semi-transparent variable
 * and 16x16 textured rectangles,
 * GP0(60h..7Bh) untextured rectangles (opaque and semi-transparent, all
 * four size classes), and the A0 upload and C0 readback streams.
 * Supported GP1 subset: 00/01 reset, 02 IRQ ack, 03 display enable,
 * 04 DMA direction, display registers05..08, and v2 info readback10..1F.
 * Return 1 on acceptance, 0 when the command is unsupported or invalid for
 * the current state. */
int PE_GPU_WriteGP0(uint32_t value);
/* C0 readback consumes two pixels per word; an unused odd high half is zero.
 * Outside a readback transfer, returns the latched info/last data word.
 * Active DMA owns the read port until explicit completion. */
int PE_GPU_ReadGP0(uint32_t *value);
/* GP1 resets may abort GPU work during a raw DMA2 transfer, retaining DMA
 * registers/event ownership. The high-level snapshot helper remains atomic. */
int PE_GPU_WriteGP1(uint32_t value);

/* Word count (command word included) of a GP0 drawing primitive that this
 * substrate can execute from the idle state, or 0 when the command is not
 * supported.  Environment/NOP words report 1; A0h/80h transfers are not
 * primitives and report 0.  Pure query: no state changes.  Command-stream
 * walkers use it so admission and execution can never disagree. C0 readback
 * also returns zero here; it runs through the image-transfer workers. */
uint32_t PE_GPU_GP0_PacketWords(uint32_t command);

/* GP0(80h) VRAM-to-VRAM copy payload.  Coordinates and dimensions use the
 * retail GPU's 10/9-bit masks, including raw zero meaning 1024/512.  The
 * operation is accepted only while GP0 is ready/idle, DMA2 is idle, and
 * GP1 direction is CPU->GP0.  It is synchronous and never creates a DMA2
 * completion event or guest callback. */
int PE_GPU_MoveImage(uint32_t source, uint32_t destination, uint32_t size);

/* Native safety preflight shared by A0 uploads and C0 readbacks. This is a pure
 * query: it never changes readiness, parser, DMA, VRAM, or event state. */
int PE_GPU_CanBeginImageLoad(int needs_dma);

/* Upload/readback block DMA: SyncMode1, 1..16 words per block; zero block
 * count means 65536. Total words must match the complete image transfer.
 * Validation precedes all register/state mutation.
 * Upload issue snapshots guest source words; readback issue does not read
 * VRAM. Neither writes its destination or completes DMA at issue time.
 * MADR exposes 24 bits; completion advances it and clears BCR block count. */
int PE_GPU_DMA2Issue(pe_addr_t madr, uint32_t bcr, uint32_t chcr);
/* Raw MADR/BCR/CHCR offsets 0/4/8. Image/command request mode or linked-list
 * RAM->GP0 (01000401h). List service consumes one node (0..255 words) per
 * call and retains GP0 parser state between nodes; FFFFFFh ends the DMA.
 * BCR is ignored/unchanged in list mode. Cycles, out-of-model RAM and
 * unsupported GP0 stop at an explicit boundary, never report completion.
 * CHCR arms an event; service obeys live DPCR and GPUSTAT DREQ, accepts
 * GP1 direction 1/2 for writes and 3 for reads, and reads RAM then. Clearing busy cancels the event. Unsupported setup/races return 0;
 * Request writes feed GP0 in its current parser state: an image payload may
 * be followed by commands in the same transfer. The image cursor survives
 * a partial transfer. All raw request transfers (image upload,
 * readback or commands/parameters) consume one BCR block (1..16 words) per
 * service, retaining parser state and live DREQ gating between blocks.
 * Image readback requires C0 setup.
 * Readback extending past its image remains unsupported.
 * Raw request mode supports increment/decrement word addressing, validating
 * the full span before starting. List mode retains its forward-only subset.
 * Request bit28 forces the first block without DREQ (still needs DPCR),
 * clearing at block start; subsequent blocks obey live request gates.
 * No chopping or RAM-mirror/bus-error modeling. */
/* Raw readback can wait for a later CPU GP0 read command. RAM is admitted
 * before arming; the pending read must fit the available image when serviced.
 * Forced empty reads and reads spanning multiple images remain boundaries. */
int PE_GPU_WriteDMA2Register(uint32_t offset,uint32_t value);
pe_addr_t PE_GPU_ReadDMA2MADR(void);
uint32_t PE_GPU_ReadDMA2BCR(void);
uint32_t PE_GPU_ReadDMA2CHCR(void);
int PE_GPU_DMA2Pending(void);
int PE_GPU_DMA2CompletionPending(void);
int PE_GPU_DMA2InterruptAsserted(void);
uint64_t PE_GPU_DMA2EventToken(void);
int PE_GPU_ServiceDMA2Completion(uint64_t event_token);

/* Native DMA6 ordering-table clear: word offsets 0/4/8, fixed decrement,
 * synchronous burst when CHCR busy+trigger and DPCR channel6 enable agree.
 * Writes descending 24-bit links and a final FFFFFFh tag. BC=0 means 65536;
 * MADR/BCR retain their burst-mode values. CHCR only exposes bits24/28/30
 * and fixed bit1. Bus snooping, RAM mirrors/underflow and bus timing remain
 * explicit boundaries; all destination RAM is checked before any write. */
uint32_t PE_GPU_ReadDMA6Register(uint32_t offset);
int PE_GPU_WriteDMA6Register(uint32_t offset,uint32_t value);

uint32_t PE_GPU_ReadDPCR(void);
void PE_GPU_WriteDPCR(uint32_t value);
void PE_GPU_EnableDMA2(void);

uint32_t PE_GPU_ReadDICR(void);
/* Stored state excludes the read-only, physically derived bit 31. */
uint32_t PE_GPU_ReadStoredDICR(void);
void PE_GPU_WriteDICR(uint32_t value);
void PE_GPU_SetDMA2InterruptEnabled(int enabled);
void PE_GPU_AcknowledgeDMA2Interrupt(void);

/* Every DICR mutation recomputes physical bit 31.  A false->true
 * transition latches a one-shot event until this separate bridge consumes
 * it.  Falling transitions never erase an unconsumed rise. */
int PE_GPU_DICRRisingEdgePending(void);
int PE_GPU_TakeDICRRisingEdge(void);

/* Explicit hardware completion-flag input for a represented DMA channel.
 * It has no data-transfer or callback behavior.  A normal completion flag
 * is created only when that channel and the DICR master are enabled. */
int PE_GPU_LatchDMACompletionFlag(uint32_t dma_channel);

/* Safe read-only access to the PSX VRAM authority.  The accessor itself
 * does not wrap; wrapping belongs to the GP0 image-transfer operation. */
int PE_GPU_ReadVRAM(uint32_t x, uint32_t y, uint16_t *pixel);

uint32_t PE_GPU_VSyncQuery(void);
void PE_GPU_VBlankStep(void);
/* Debug: current host frame for the env-gated PE_GPU_PRIM_LOG. */
void PE_GPU_PrimLogFrame(int frame);
/* Packet length at words[0] (polylines to their terminator); 0 = incomplete. */
uint32_t PE_GPU_GP0_StreamPacketWords(const uint32_t *words, uint32_t avail);
/* Physical signal inputs from the future raster scheduler. Rising edges
 * advance timer1/latch IRQ0; they never run callbacks. Repeated levels and
 * stale generations are inert. SDK/GPU reset begins in blanking. */
int PE_GPU_SetHBlank(int level,PeIrqGeneration generation);
int PE_GPU_SetVBlank(int level,PeIrqGeneration generation);

void PE_GPU_GetState(PeGpuState *out);

#ifdef __cplusplus
}
#endif

#endif /* PE_GPU_H */
