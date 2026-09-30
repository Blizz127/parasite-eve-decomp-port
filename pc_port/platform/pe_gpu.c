/*
 * Phase 6E-B53B — minimal deterministic GPU/DMA2 platform authority.
 *
 * B53A proved the software queue separately.  This file therefore contains
 * no retail ring, worker identity, callback, or queue-pump state.  It models
 * only the hardware subset required by the LoadImage path: 1024x512x16 VRAM,
 * GPUSTAT ready bit 26, the GP0 A0 stream, the required GP1 commands, DMA2
 * block issue/completion, DPCR/DICR channel 2, and a deterministic VBlank
 * counter.  Hardware progress occurs only through explicit API calls.
 */
#include "pe_gpu.h"
#include "pe_audio_driver.h"
#include "pe_timer1.h"

#include <stddef.h>
#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>
#include <string.h>

/* PE_GPU_PRIM_LOG=<path>[:first[:last]]: debug aid, off by default.  Writes
 * one decoded line per executed GP0 draw primitive (and each rejected GP0
 * word) while the host frame passed to PE_GPU_PrimLogFrame lies inside
 * [first,last].  Host diagnostics only; never alters GPU state. */
static FILE *g_prim_log;
static int g_prim_log_first, g_prim_log_last = 0x7FFFFFFF, g_prim_log_frame = -1;
static int g_prim_log_init;

static int PrimLogOn(void)
{
    if (!g_prim_log_init) {
        const char *e = getenv("PE_GPU_PRIM_LOG");
        g_prim_log_init = 1;
        if (e && *e) {
            char path[512];
            const char *colon = strchr(e, ':');
            size_t n = colon ? (size_t)(colon - e) : strlen(e);
            if (n >= sizeof path) n = sizeof path - 1u;
            memcpy(path, e, n); path[n] = 0;
            if (colon) {
                g_prim_log_first = atoi(colon + 1);
                colon = strchr(colon + 1, ':');
                if (colon) g_prim_log_last = atoi(colon + 1);
            }
            g_prim_log = fopen(path, "w");
        }
    }
    return g_prim_log && g_prim_log_frame >= g_prim_log_first &&
           g_prim_log_frame <= g_prim_log_last;
}

/* PE_GPU_XFER_LOG=<path>: debug aid, one line per CPU->VRAM (A0h) and
 * VRAM->VRAM (80h) transfer with the current host frame. */
static void XferLog(const char *kind, uint32_t a, uint32_t b, uint32_t c)
{
    static FILE *f; static int init;
    if (!init) { const char *e = getenv("PE_GPU_XFER_LOG"); init = 1; if (e && *e) f = fopen(e, "w"); }
    if (f) { fprintf(f, "%d %s %08X %08X %08X\n", g_prim_log_frame, kind, a, b, c); fflush(f); }
    if (f && getenv("PE_GPU_WATCH_BT") && strtoul(getenv("PE_GPU_WATCH_BT"), 0, 16) == a) {
        void *bt[24]; int n = backtrace(bt, 24); backtrace_symbols_fd(bt, n, fileno(f));
    }
}

void PE_GPU_PrimLogFrame(int frame)
{
    g_prim_log_frame = frame;
    if (PrimLogOn()) { fprintf(g_prim_log, "F %d\n", frame); fflush(g_prim_log); }
}

typedef struct {
    PeGpuState state;
    uint64_t order_counter;
    uint64_t event_serial;
    uint32_t rectangle_command;
    uint32_t rectangle_position;
    uint32_t rectangle_uv_clut;
    uint32_t fill_command;
    uint32_t fill_position;
    uint32_t mono_rectangle_command;
    uint32_t mono_rectangle_position;
    uint32_t polygon_words[12];
    /* GP0 polyline (48h..5Fh with bit 27) stream state. */
    int polyline, polyline_expect_color;
    uint32_t polyline_command, polyline_color, polyline_next_color;
    uint32_t polyline_xy, polyline_vertices;
    uint32_t polygon_received;
    uint32_t polygon_expected;
    uint16_t vram[PE_GPU_VRAM_PIXELS];
} PeGpuAuthority;

static PeGpuAuthority g_gpu = {
    .state = {
        .status = PE_GPU_STATUS_READY_GP0,
        .dma2_chcr = PE_GPU_DMA2_CHCR_IDLE
    }
};

static int DICRMasterFlag(uint32_t stored)
{
    return (stored & PE_GPU_DICR_FORCE) != 0u ||
           ((stored & PE_GPU_DICR_MASTER) != 0u &&
            (stored & PE_GPU_DICR_FLAGS) != 0u);
}

static void CommitStoredDICR(uint32_t stored)
{
    int old_level = DICRMasterFlag(g_gpu.state.dicr);
    int new_level;

    /* Physical/master flag bit 31 is derived and is never stored. */
    stored &= ~PE_GPU_DICR_MASTER_FLAG;
    new_level = DICRMasterFlag(stored);
    g_gpu.state.dicr = stored;
    if (!old_level && new_level) {
        /* Sticky until the separate CPU-source bridge consumes it.  A
         * later falling transition must not erase an already-created IRQ
         * edge. */
        g_gpu.state.dicr_rising_edge_pending = 1;
    }
}

static void ResetParser(void)
{
    g_gpu.state.status &= ~PE_GPU_STATUS_READY_READ;
    g_gpu.state.gp0_state = PE_GPU_GP0_IDLE;
    g_gpu.state.image_x = 0;
    g_gpu.state.image_y = 0;
    g_gpu.state.image_width = 0;
    g_gpu.state.image_height = 0;
    g_gpu.state.image_current_pixel = 0;
    g_gpu.state.image_remaining_pixels = 0;
    g_gpu.rectangle_command = 0;
    g_gpu.rectangle_position = 0;
    g_gpu.rectangle_uv_clut = 0;
    g_gpu.fill_command = 0;
    g_gpu.fill_position = 0;
    g_gpu.mono_rectangle_command = 0;
    g_gpu.mono_rectangle_position = 0;
    g_gpu.polygon_received = g_gpu.polygon_expected = 0u;
    g_gpu.polyline = 0;
}

static void ResetDisplayRegisters(void)
{
    g_gpu.state.display_start=0;
    g_gpu.state.display_horizontal_range=0xC00200u;
    g_gpu.state.display_vertical_range=0x40010u;
    g_gpu.state.display_mode=0;
    g_gpu.state.status&=~0x7F4000u;
}

static void ResetHardwareState(void)
{
    uint64_t event_serial = g_gpu.event_serial;

    memset(&g_gpu.state, 0, sizeof(g_gpu.state));
    g_gpu.order_counter = 0;
    g_gpu.event_serial = event_serial;
    g_gpu.state.status = PE_GPU_STATUS_READY_GP0;
    g_gpu.state.hblank_level=g_gpu.state.vblank_level=1;
    g_gpu.state.dma2_chcr = PE_GPU_DMA2_CHCR_IDLE;
    ResetDisplayRegisters();
    ResetParser();
}

void PE_GPU_Init(void)
{
    memset(g_gpu.vram, 0, sizeof(g_gpu.vram));
    ResetHardwareState();
}

void PE_GPU_Reset(void)
{
    ResetHardwareState();
}

/* GPUSTAT readback of GP0 E1/E6, including texpage commands embedded
 * in textured polygons. The register shares bits with the draw state. */
static void UpdateDrawingStatus(void)
{
    g_gpu.state.status=(g_gpu.state.status&~0x9FFFu) |
        (g_gpu.state.draw_mode&0x7FFu) |
        ((g_gpu.state.draw_mode&0x800u)<<4u) |
        ((g_gpu.state.mask_setting&3u)<<11u);
}

uint32_t PE_GPU_ReadStatus(void)
{
    return g_gpu.state.status;
}

void PE_GPU_SetReady(int ready)
{
    if (ready) {
        g_gpu.state.status |= PE_GPU_STATUS_READY_GP0;
    } else {
        g_gpu.state.status &= ~PE_GPU_STATUS_READY_GP0;
    }
}

static void FinishImage(void)
{
    g_gpu.state.status &= ~PE_GPU_STATUS_READY_READ;
    g_gpu.state.gp0_state = PE_GPU_GP0_IDLE;
    g_gpu.state.image_remaining_pixels = 0;
}

static long g_watch_x = -2, g_watch_y = -1;
static void WatchInit(void)
{
    const char *e;
    if (g_watch_x != -2) return;
    g_watch_x = -1; e = getenv("PE_GPU_WATCH");
    if (e) { char *end; g_watch_x = strtol(e, &end, 10); if (*end == ',') g_watch_y = strtol(end + 1, 0, 10); }
}

static void WriteImagePixel(uint16_t pixel)
{
    uint32_t ordinal;
    uint32_t x;
    uint32_t y;

    if (g_gpu.state.gp0_state != PE_GPU_GP0_IMAGE_DATA ||
        g_gpu.state.image_remaining_pixels == 0 ||
        g_gpu.state.image_width == 0) {
        return;
    }

    ordinal = g_gpu.state.image_current_pixel;
    x = (g_gpu.state.image_x + ordinal % g_gpu.state.image_width) &
        (PE_GPU_VRAM_WIDTH - 1u);
    y = (g_gpu.state.image_y + ordinal / g_gpu.state.image_width) &
        (PE_GPU_VRAM_HEIGHT - 1u);
    WatchInit();
    if ((long)x == g_watch_x && (long)y == g_watch_y)
    {
        fprintf(stderr, "[GPU_WATCH] A0 image (%u,%u) %ux%u sets %04X (dma2=%d src=%08X)\n",
                g_gpu.state.image_x, g_gpu.state.image_y, g_gpu.state.image_width,
                g_gpu.state.image_height, pixel, g_gpu.state.dma2_active, (unsigned)g_gpu.state.dma2_source);
        if (getenv("PE_GPU_WATCH_BT")) { void *bt[24]; int n = backtrace(bt, 24); backtrace_symbols_fd(bt, n, 2); }
    }
    g_gpu.vram[y * PE_GPU_VRAM_WIDTH + x] = pixel;
    g_gpu.state.image_current_pixel++;
    g_gpu.state.image_remaining_pixels--;
    if (g_gpu.state.image_remaining_pixels == 0) {
        FinishImage();
    }
}

static void WriteImageWord(uint32_t value)
{
    WriteImagePixel((uint16_t)value);
    if (g_gpu.state.gp0_state == PE_GPU_GP0_IMAGE_DATA) {
        WriteImagePixel((uint16_t)(value >> 16));
    }
}

static uint16_t ReadImagePixel(void)
{
    uint32_t ordinal,x,y;uint16_t pixel;
    if (g_gpu.state.gp0_state!=PE_GPU_GP0_READ_DATA || !g_gpu.state.image_remaining_pixels) return 0u;
    ordinal=g_gpu.state.image_current_pixel;
    x=(g_gpu.state.image_x+ordinal%g_gpu.state.image_width)&(PE_GPU_VRAM_WIDTH-1u);
    y=(g_gpu.state.image_y+ordinal/g_gpu.state.image_width)&(PE_GPU_VRAM_HEIGHT-1u);
    pixel=g_gpu.vram[y*PE_GPU_VRAM_WIDTH+x];
    g_gpu.state.image_current_pixel++;g_gpu.state.image_remaining_pixels--;
    if (!g_gpu.state.image_remaining_pixels) FinishImage();
    return pixel;
}

static uint32_t ReadImageWord(void)
{
    uint32_t low=ReadImagePixel();
    return low|((uint32_t)ReadImagePixel()<<16u);
}

int PE_GPU_ReadGP0(uint32_t *value)
{
    if (!value || g_gpu.state.dma2_active || g_gpu.state.gp0_state!=PE_GPU_GP0_READ_DATA) return 0;
    *value=ReadImageWord();return 1;
}

static int32_t SignExtend11(uint32_t value)
{
    value &= 0x7FFu;
    return (value & 0x400u) != 0u ?
        (int32_t)(value | 0xFFFFF800u) : (int32_t)value;
}

static uint32_t ApplyTextureWindow(uint32_t coordinate, int horizontal)
{
    uint32_t command = g_gpu.state.texture_window;
    uint32_t mask = horizontal ? command & 0x1Fu :
        (command >> 5) & 0x1Fu;
    uint32_t offset = horizontal ? (command >> 10) & 0x1Fu :
        (command >> 15) & 0x1Fu;

    mask <<= 3;
    return ((coordinate & ~mask) | ((offset << 3) & mask)) & 0xFFu;
}

static uint16_t ModulateTextureColor(uint16_t texture, uint32_t command)
{
    uint32_t red = ((uint32_t)(texture & 0x1Fu) *
                    (command & 0xFFu)) >> 7;
    uint32_t green = ((uint32_t)((texture >> 5) & 0x1Fu) *
                      ((command >> 8) & 0xFFu)) >> 7;
    uint32_t blue = ((uint32_t)((texture >> 10) & 0x1Fu) *
                     ((command >> 16) & 0xFFu)) >> 7;

    if (red > 0x1Fu) red = 0x1Fu;
    if (green > 0x1Fu) green = 0x1Fu;
    if (blue > 0x1Fu) blue = 0x1Fu;
    return (uint16_t)(red | (green << 5) | (blue << 10) |
                      (texture & 0x8000u));
}

static uint16_t BlendSemiTransparent(uint16_t back, uint16_t front);
static uint32_t MonoRectangleFixedSize(uint32_t command);

/* GP0 textured rectangles. Raw-texture variants preserve texel RGB; blending
 * requires both command bit 25 and texel bit 15. Rectangles do not dither.
 * https://psx-spx.consoledev.net/graphicsprocessingunitgpu/
 * The E2h..E6h
 * environment is applied only after the corresponding command has actually
 * been observed; this preserves the pre-PutDrawEnv physical-clip behavior. */
static void DrawTexturedRectangle(uint32_t size)
{
    uint32_t draw_mode = g_gpu.state.draw_mode;
    uint32_t width = size & 0x3FFu;
    uint32_t height = (size >> 16) & 0x1FFu;
    int32_t origin_x = SignExtend11(g_gpu.rectangle_position);
    int32_t origin_y = SignExtend11(g_gpu.rectangle_position >> 16);
    uint32_t origin_u = g_gpu.rectangle_uv_clut & 0xFFu;
    uint32_t origin_v = (g_gpu.rectangle_uv_clut >> 8) & 0xFFu;
    uint32_t clut = g_gpu.rectangle_uv_clut >> 16;
    uint32_t clut_x = (clut & 0x3Fu) * 16u;
    uint32_t clut_y = (clut >> 6) & (PE_GPU_VRAM_HEIGHT - 1u);
    uint32_t texture_x = (draw_mode & 0xFu) * 64u;
    uint32_t texture_y = ((draw_mode >> 4) & 1u) * 256u;
    int flip_x = (draw_mode & 0x1000u) != 0u;
    int flip_y = (draw_mode & 0x2000u) != 0u;
    uint32_t row;

    if (g_gpu.state.drawing_offset_count != 0u) {
        origin_x += SignExtend11(g_gpu.state.drawing_offset);
        origin_y += SignExtend11(g_gpu.state.drawing_offset >> 11);
    }
    if (PrimLogOn())
        fprintf(g_prim_log, "S %08X (%d,%d) uvclut=%08X %ux%u mode=%04X\n",
                g_gpu.rectangle_command, origin_x, origin_y, g_gpu.rectangle_uv_clut,
                width, height, (unsigned)(draw_mode & 0xFFFFu));

    for (row = 0u; row < height; row++) {
        int32_t destination_y = origin_y + (int32_t)row;
        uint32_t texture_v = (origin_v +
            (flip_y ? (0u - row) : row)) & 0xFFu;
        uint32_t column;

        if (g_gpu.state.texture_window_count != 0u)
            texture_v = ApplyTextureWindow(texture_v, 0);
        if (destination_y < 0 || destination_y >=
            (int32_t)PE_GPU_VRAM_HEIGHT) {
            continue;
        }
        if (g_gpu.state.drawing_area_top_left_count != 0u &&
            g_gpu.state.drawing_area_bottom_right_count != 0u &&
            ((uint32_t)destination_y <
                 ((g_gpu.state.drawing_area_top_left >> 10) & 0x1FFu) ||
             (uint32_t)destination_y >
                 ((g_gpu.state.drawing_area_bottom_right >> 10) & 0x1FFu)))
            continue;
        for (column = 0u; column < width; column++) {
            int32_t destination_x = origin_x + (int32_t)column;
            uint32_t texture_u;
            uint16_t packed;
            uint32_t palette_index;
            uint16_t texture_color;

            if (destination_x < 0 || destination_x >=
                (int32_t)PE_GPU_VRAM_WIDTH) {
                continue;
            }
            if (g_gpu.state.drawing_area_top_left_count != 0u &&
                g_gpu.state.drawing_area_bottom_right_count != 0u &&
                ((uint32_t)destination_x <
                     (g_gpu.state.drawing_area_top_left & 0x3FFu) ||
                 (uint32_t)destination_x >
                     (g_gpu.state.drawing_area_bottom_right & 0x3FFu)))
                continue;
            texture_u = (origin_u +
                (flip_x ? (0u - column) : column)) & 0xFFu;
            if (g_gpu.state.texture_window_count != 0u)
                texture_u = ApplyTextureWindow(texture_u, 1);
            if (((draw_mode >> 7) & 3u) >= 2u) {
                /* 15bpp direct (depth 3 "reserved" behaves as 15bpp). */
                texture_color = g_gpu.vram[
                    ((texture_y + texture_v) &
                     (PE_GPU_VRAM_HEIGHT - 1u)) * PE_GPU_VRAM_WIDTH +
                    ((texture_x + texture_u) &
                     (PE_GPU_VRAM_WIDTH - 1u))];
                goto have_texel;
            } else if (((draw_mode >> 7) & 3u) == 1u) {
                packed = g_gpu.vram[
                    ((texture_y + texture_v) &
                     (PE_GPU_VRAM_HEIGHT - 1u)) * PE_GPU_VRAM_WIDTH +
                    ((texture_x + texture_u / 2u) &
                     (PE_GPU_VRAM_WIDTH - 1u))];
                palette_index =
                    (packed >> ((texture_u & 1u) * 8u)) & 0xFFu;
            } else {
                packed = g_gpu.vram[
                    ((texture_y + texture_v) &
                     (PE_GPU_VRAM_HEIGHT - 1u)) * PE_GPU_VRAM_WIDTH +
                    ((texture_x + texture_u / 4u) &
                     (PE_GPU_VRAM_WIDTH - 1u))];
                palette_index =
                    (packed >> ((texture_u & 3u) * 4u)) & 0xFu;
            }
            texture_color = g_gpu.vram[
                clut_y * PE_GPU_VRAM_WIDTH +
                ((clut_x + palette_index) &
                 (PE_GPU_VRAM_WIDTH - 1u))];
        have_texel:
            if (texture_color == 0u)
                continue;
            {
                uint16_t *destination = &g_gpu.vram[
                    (uint32_t)destination_y * PE_GPU_VRAM_WIDTH +
                    (uint32_t)destination_x];
                uint16_t result;

                if (g_gpu.state.mask_setting_count != 0u &&
                    (g_gpu.state.mask_setting & 2u) != 0u &&
                    (*destination & 0x8000u) != 0u)
                    continue;
                result = (g_gpu.rectangle_command & 0x01000000u) ?
                    texture_color : ModulateTextureColor(texture_color,
                                                         g_gpu.rectangle_command);
                if ((g_gpu.rectangle_command & 0x02000000u) && (texture_color & 0x8000u))
                    result = BlendSemiTransparent(*destination, result) | (texture_color & 0x8000u);
                if (g_gpu.state.mask_setting_count != 0u &&
                    (g_gpu.state.mask_setting & 1u) != 0u)
                    result |= 0x8000u;
                *destination = result;
            }
        }
    }
}

static int TexturedRectangleIndexedModeSupported(void)
{
    /* Variable and fixed rectangles use the same E1h texture depth.
     * 4bpp, 8bpp and 15bpp (and reserved depth 3, which the GPU treats
     * as 15bpp) are all implemented.  E1h bit 11 (texture disable) only
     * takes effect when GP1(09h) allows it, which retail never enables. */
    return 1;
}

/* Textured rectangle size class (bits 27-28): 0 variable, 1 1x1,
 * 2 8x8, 3 16x16 (psx-spx "GPU Render Rectangle Commands"). */
static uint32_t TexturedRectangleFixedSize(uint32_t command)
{
    return MonoRectangleFixedSize(command);
}

/* GP0(02h), console VRAM: 16-pixel horizontal alignment/rounding,
 * independent X/Y wrapping, and RGB888 -> RGB555 without mask bits.
 * Drawing area, offset and E6 mask controls do not affect quick fills. */
static void DrawFillRectangle(uint32_t size)
{
    uint32_t command = g_gpu.fill_command;
    uint32_t red = (command & 0xFFu) >> 3;
    uint32_t green = ((command >> 8) & 0xFFu) >> 3;
    uint32_t blue = ((command >> 16) & 0xFFu) >> 3;
    uint16_t pixel =
        (uint16_t)(red | (green << 5) | (blue << 10));
    uint32_t origin_x = g_gpu.fill_position & 0x3F0u;
    uint32_t origin_y = (g_gpu.fill_position >> 16) & 0x1FFu;
    uint32_t width = ((size & 0x3FFu) + 15u) & ~15u;
    uint32_t height = (size >> 16) & 0x1FFu;
    uint32_t row;

    for (row = 0u; row < height; row++) {
        uint32_t destination_y = (origin_y + row) & 0x1FFu;
        uint32_t column;

        for (column = 0u; column < width; column++) {
            uint32_t destination_x = (origin_x + column) & 0x3FFu;

            g_gpu.vram[destination_y * PE_GPU_VRAM_WIDTH +
                       destination_x] = pixel;
        }
    }
}

/* GP0 command-word semi-transparency (bit 25) blends the incoming color
 * against the framebuffer with the GP0(E1h) ABR mode (bits 5-6):
 *   0: B/2 + F/2    1: B + F    2: B - F    3: B + F/4
 * evaluated per 5-bit channel with saturation, as the hardware does.
 * The result keeps the destination's mask bit untouched; the caller
 * decides whether GP0(E6h) forces it. */
static uint16_t BlendSemiTransparent(uint16_t back, uint16_t front)
{
    uint32_t abr = (g_gpu.state.draw_mode >> 5) & 3u;
    uint16_t result = 0u;
    unsigned shift;

    for (shift = 0u; shift < 15u; shift += 5u) {
        int32_t b = (int32_t)((back >> shift) & 0x1Fu);
        int32_t f = (int32_t)((front >> shift) & 0x1Fu);
        int32_t v;

        switch (abr) {
        case 0u:  v = b / 2 + f / 2; break;
        case 1u:  v = b + f;         break;
        case 2u:  v = b - f;         break;
        default:  v = b + f / 4;     break;
        }
        if (v < 0) v = 0;
        if (v > 31) v = 31;
        result |= (uint16_t)((uint32_t)v << shift);
    }
    return result;
}

/* Drawing-area clip for the observed GP0(E3h)/(E4h) registers.  Before both
 * have been seen only the physical VRAM bound applies (the same
 * pre-PutDrawEnv rule the textured rectangle path uses). */
static int PixelInDrawingArea(int32_t x, int32_t y)
{
    if (x < 0 || y < 0 || x >= (int32_t)PE_GPU_VRAM_WIDTH ||
        y >= (int32_t)PE_GPU_VRAM_HEIGHT)
        return 0;
    if (g_gpu.state.drawing_area_top_left_count == 0u ||
        g_gpu.state.drawing_area_bottom_right_count == 0u)
        return 1;
    return (uint32_t)x >= (g_gpu.state.drawing_area_top_left & 0x3FFu) &&
           (uint32_t)x <= (g_gpu.state.drawing_area_bottom_right & 0x3FFu) &&
           (uint32_t)y >=
               ((g_gpu.state.drawing_area_top_left >> 10) & 0x1FFu) &&
           (uint32_t)y <=
               ((g_gpu.state.drawing_area_bottom_right >> 10) & 0x1FFu);
}

/* GP0 24-bit color word (0x00BBGGRR) to the 15-bit VRAM format. */
static uint16_t Color24To15(uint32_t command)
{
    return (uint16_t)(((command & 0xFFu) >> 3) |
                      (((command >> 8) & 0xFFu) >> 3) << 5 |
                      (((command >> 16) & 0xFFu) >> 3) << 10);
}

/* Untextured rectangle opcodes 60h..7Bh (bit 26 clear).  Bits 27-28 pick
 * the size class; bit 25 selects semi-transparency.  Rectangles are not
 * dithered, and the drawing offset (E5h) applies once observed. */
static uint32_t MonoRectangleFixedSize(uint32_t command)
{
    switch ((command >> 27) & 3u) {
    case 1u: return 1u | (1u << 16);
    case 2u: return 8u | (8u << 16);
    case 3u: return 16u | (16u << 16);
    default: return 0u;   /* variable: size word follows */
    }
}

static void DrawMonoRectangle(uint32_t size)
{
    uint32_t command = g_gpu.mono_rectangle_command;
    uint16_t color = Color24To15(command);
    int semi_transparent = (command & 0x02000000u) != 0u;
    uint32_t width = size & 0x3FFu;
    uint32_t height = (size >> 16) & 0x1FFu;
    int32_t origin_x = SignExtend11(g_gpu.mono_rectangle_position);
    int32_t origin_y = SignExtend11(g_gpu.mono_rectangle_position >> 16);
    int check_mask = g_gpu.state.mask_setting_count != 0u &&
                     (g_gpu.state.mask_setting & 2u) != 0u;
    int set_mask = g_gpu.state.mask_setting_count != 0u &&
                   (g_gpu.state.mask_setting & 1u) != 0u;
    uint32_t row;

    if (g_gpu.state.drawing_offset_count != 0u) {
        origin_x += SignExtend11(g_gpu.state.drawing_offset);
        origin_y += SignExtend11(g_gpu.state.drawing_offset >> 11);
    }
    if (PrimLogOn())
        fprintf(g_prim_log, "T %08X (%d,%d) %ux%u mode=%04X\n", command, origin_x, origin_y,
                width, height, (unsigned)(g_gpu.state.draw_mode & 0xFFFFu));

    for (row = 0u; row < height; row++) {
        int32_t y = origin_y + (int32_t)row;
        uint32_t column;

        for (column = 0u; column < width; column++) {
            int32_t x = origin_x + (int32_t)column;
            uint16_t *destination;
            uint16_t result;

            if (!PixelInDrawingArea(x, y))
                continue;
            destination = &g_gpu.vram[(uint32_t)y * PE_GPU_VRAM_WIDTH +
                                      (uint32_t)x];
            if (check_mask && (*destination & 0x8000u) != 0u)
                continue;
            result = semi_transparent ?
                BlendSemiTransparent(*destination, color) : color;
            if (set_mask)
                result |= 0x8000u;
            *destination = result;
        }
    }
    g_gpu.state.mono_rectangle_command = command;
    g_gpu.state.mono_rectangle_position = g_gpu.mono_rectangle_position;
    g_gpu.state.mono_rectangle_size = width | (height << 16);
    g_gpu.state.mono_rectangle_count++;
}

/* POLY1: integer affine polygon rasterization. Packet formats and draw
 * attributes follow psx-spx GPU Render Polygon Commands. Edge coverage is a
 * native top-left rule; attribute interpolation rounds to nearest (below). */
typedef struct {
    int32_t x, y, u, v;
    int32_t color[3];
} PePolygonVertex;

static int64_t PolygonEdge(const PePolygonVertex *a, const PePolygonVertex *b, int32_t x, int32_t y)
{
    return (int64_t)(b->x-a->x)*(y-a->y) - (int64_t)(b->y-a->y)*(x-a->x);
}

static int PolygonTopLeft(const PePolygonVertex *a, const PePolygonVertex *b)
{
    return b->y < a->y || (b->y == a->y && b->x > a->x);
}

static uint16_t PolygonTexel(uint32_t u, uint32_t v, uint32_t clut)
{
    uint32_t mode = g_gpu.state.draw_mode;
    uint32_t depth = (mode >> 7) & 3u;
    uint32_t tx = (mode & 15u) * 64u, ty = ((mode >> 4) & 1u) * 256u;
    uint16_t packed;
    unsigned int index;
    u = ApplyTextureWindow(u & 255u, 1);
    v = ApplyTextureWindow(v & 255u, 0);
    ty = (ty + v) & 511u;
    if (depth >= 2u) return g_gpu.vram[ty * 1024u + ((tx + u) & 1023u)];
    packed = g_gpu.vram[ty * 1024u + ((tx + (u >> (depth ? 1u : 2u))) & 1023u)];
    index = depth ? (packed >> ((u & 1u) * 8u)) & 255u : (packed >> ((u & 3u) * 4u)) & 15u;
    return g_gpu.vram[((clut >> 6) & 511u) * 1024u + (((clut & 63u) * 16u + index) & 1023u)];
}

static void DrawPolygonTriangle(PePolygonVertex a, PePolygonVertex b, PePolygonVertex c,
                                uint32_t command, uint32_t clut)
{
    static const int8_t dither[4][4] = {{-4,0,-3,1},{2,-2,3,-1},{-3,1,-4,0},{3,-1,2,-2}};
    int64_t area = PolygonEdge(&a,&b,c.x,c.y), half;
    int32_t minx, maxx, miny, maxy, x, y;
    int textured = (command & 0x04000000u) != 0u;
    int raw = textured && (command & 0x01000000u);
    int dithering = (g_gpu.state.draw_mode & 0x200u) && !raw &&
                    (textured || (command & 0x10000000u));
    if (!area) return;
    if (area < 0) { PePolygonVertex temp = b; b = c; c = temp; area = -area; }
    /* Interpolated UV and RGB round to nearest (half-unit bias on the planar
     * value), not truncate: the hardware-researched rasterizers add
     * 1 << (frac-1) to the attribute base (Mednafen/DuckStation convention),
     * and the emulator oracle's first room agrees (gfx2 lane: 95.87% ->
     * 98.90% identical pixels vs oracle VSync 9713, with the model packets
     * and GTE colour tables already bit-identical).  Inside the triangle every
     * edge weight is >= 0, so the numerators are non-negative. */
    half = area / 2;
    minx = a.x < b.x ? a.x : b.x; if (c.x < minx) minx = c.x;
    maxx = a.x > b.x ? a.x : b.x; if (c.x > maxx) maxx = c.x;
    miny = a.y < b.y ? a.y : b.y; if (c.y < miny) miny = c.y;
    maxy = a.y > b.y ? a.y : b.y; if (c.y > maxy) maxy = c.y;
    if (maxx-minx >= 1024 || maxy-miny >= 512) return;
    if (minx < 0) minx = 0;
    if (maxx > 1023) maxx = 1023;
    if (miny < 0) miny = 0;
    if (maxy > 511) maxy = 511;
    for (y = miny; y <= maxy; y++) {
        for (x = minx; x <= maxx; x++) {
            int64_t w0 = PolygonEdge(&b,&c,x,y), w1 = PolygonEdge(&c,&a,x,y), w2 = PolygonEdge(&a,&b,x,y);
            uint16_t texture = 0u, color = 0u, *pixel;
            unsigned int channel;
            if (w0 < 0 || (w0 == 0 && !PolygonTopLeft(&b,&c)) ||
                w1 < 0 || (w1 == 0 && !PolygonTopLeft(&c,&a)) ||
                w2 < 0 || (w2 == 0 && !PolygonTopLeft(&a,&b)) || !PixelInDrawingArea(x,y)) continue;
            pixel = &g_gpu.vram[(uint32_t)y * 1024u + (uint32_t)x];
            if ((g_gpu.state.mask_setting & 2u) && (*pixel & 0x8000u)) continue;
            if (textured) {
                uint32_t u = (uint32_t)((w0*a.u + w1*b.u + w2*c.u + half) / area);
                uint32_t v = (uint32_t)((w0*a.v + w1*b.v + w2*c.v + half) / area);
                texture = PolygonTexel(u,v,clut);
                if (texture == 0u) continue;
            }
            if (raw) color = texture;
            else {
                for (channel = 0u; channel < 3u; channel++) {
                    int value = (int)((w0*a.color[channel] + w1*b.color[channel] + w2*c.color[channel] + half) / area);
                    if (textured) value = (int)(((texture >> (channel*5u)) & 31u) * (unsigned int)value) >> 4;
                    if (dithering) value += dither[y & 3][x & 3];
                    if (value < 0) value = 0;
                    if (value > 255) value = 255;
                    color |= (uint16_t)((unsigned int)(value >> 3) << (channel*5u));
                }
                color |= texture & 0x8000u;
            }
            if ((command & 0x02000000u) && (!textured || (texture & 0x8000u)))
                color = BlendSemiTransparent(*pixel,color) | (texture & 0x8000u);
            if (g_gpu.state.mask_setting & 1u) color |= 0x8000u;
            *pixel = color;
            g_gpu.state.polygon_pixel_count++;
        }
    }
}

static void DrawPolygon(void)
{
    PePolygonVertex vertices[4];
    uint32_t command = g_gpu.polygon_words[0], clut = 0u;
    uint32_t color = command;
    unsigned int i, ch, cursor = 1u, count = (command & 0x08000000u) ? 4u : 3u;
    for (i = 0; i < count; i++) {
        uint32_t xy, uv;
        if (i && (command & 0x10000000u)) color = g_gpu.polygon_words[cursor++];
        xy = g_gpu.polygon_words[cursor++];
        vertices[i].x = SignExtend11(xy) + SignExtend11(g_gpu.state.drawing_offset);
        vertices[i].y = SignExtend11(xy >> 16) + SignExtend11(g_gpu.state.drawing_offset >> 11);
        vertices[i].u = vertices[i].v = 0;
        for (ch=0; ch<3u; ch++) vertices[i].color[ch] = (color >> (ch*8u)) & 255u;
        if (command & 0x04000000u) {
            uv = g_gpu.polygon_words[cursor++];
            vertices[i].u = uv & 255u; vertices[i].v = (uv >> 8) & 255u;
            if (i == 0u) clut = uv >> 16;
            if (i == 1u) {
                g_gpu.state.draw_mode = (g_gpu.state.draw_mode & ~0x9FFu) | ((uv >> 16) & 0x9FFu);
                UpdateDrawingStatus();
            }
        }
    }
    if (PrimLogOn()) {
        fprintf(g_prim_log, "P %08X mode=%04X clut=%04X", command,
                (unsigned)(g_gpu.state.draw_mode & 0xFFFFu), (unsigned)clut);
        for (i = 0; i < count; i++)
            fprintf(g_prim_log, " (%d,%d uv%d,%d c%02X%02X%02X)", vertices[i].x, vertices[i].y,
                    vertices[i].u, vertices[i].v, (unsigned)vertices[i].color[0],
                    (unsigned)vertices[i].color[1], (unsigned)vertices[i].color[2]);
        fputc('\n', g_prim_log);
    }
    DrawPolygonTriangle(vertices[0],vertices[1],vertices[2],command,clut);
    if (count == 4u) DrawPolygonTriangle(vertices[1],vertices[2],vertices[3],command,clut);
    g_gpu.state.polygon_count++;
}

/* Two-vertex lines include both endpoints, dither even with a flat color,
 * and use the same draw-area, mask and ABR registers as polygons.
 * Command layout: https://psx-spx.consoledev.net/graphicsprocessingunitgpu/
 * Fixed-point coverage conventions (32 fractional coordinate bits, 12
 * color bits, directional tie bias) cross-checked against DuckStation's
 * gpu_sw_rasterizer.inl. This native DDA uses no host graphics API. */
static void DrawLine(void)
{
    static const int8_t dither[4][4]={{-4,0,-3,1},{2,-2,3,-1},{-3,1,-4,0},{3,-1,2,-2}};
    const int64_t unit=INT64_C(4294967296);
    uint32_t command=g_gpu.polygon_words[0];
    uint32_t colors[2]={command,command};
    uint32_t xy[2]={g_gpu.polygon_words[1],g_gpu.polygon_words[2]};
    int32_t ends[2][2], delta[2], span, axis, step, ch;
    int64_t origin[2], increment[2];
    int32_t rgb[3], rgb_step[3];
    if (command&0x10000000u) {
        colors[1]=g_gpu.polygon_words[2]; xy[1]=g_gpu.polygon_words[3];
    }
    for (axis=0;axis<2;axis++) {
        int32_t offset=SignExtend11(g_gpu.state.drawing_offset>>(axis*11));
        ends[0][axis]=SignExtend11(xy[0]>>(axis*16))+offset;
        ends[1][axis]=SignExtend11(xy[1]>>(axis*16))+offset;
        delta[axis]=ends[1][axis]-ends[0][axis];
        if (delta[axis]<0) delta[axis]=-delta[axis];
    }
    g_gpu.state.line_count++;
    if (PrimLogOn())
        fprintf(g_prim_log, "L %08X (%d,%d) (%d,%d) mode=%04X\n", command, ends[0][0], ends[0][1],
                ends[1][0], ends[1][1], (unsigned)(g_gpu.state.draw_mode & 0xFFFFu));
    if (delta[0]>=1024 || delta[1]>=512) return;
    span=delta[0]>delta[1]?delta[0]:delta[1];
    if (span && ends[0][0]>=ends[1][0]) {
        uint32_t color=colors[0]; colors[0]=colors[1]; colors[1]=color;
        for (axis=0;axis<2;axis++) {
            int32_t coord=ends[0][axis]; ends[0][axis]=ends[1][axis]; ends[1][axis]=coord;
        }
    }
    for (axis=0;axis<2;axis++) {
        int32_t distance=ends[1][axis]-ends[0][axis];
        int64_t numerator=(int64_t)distance*unit;
        if (span && distance) numerator+=distance>0?span-1:1-span;
        increment[axis]=span?numerator/span:0;
        origin[axis]=(int64_t)ends[0][axis]*unit+unit/2;
        if (axis==0 || increment[axis]<0) origin[axis]-=1024;
    }
    for (ch=0;ch<3;ch++) {
        int32_t first=(colors[0]>>(ch*8))&255u, last=(colors[1]>>(ch*8))&255u;
        rgb[ch]=first*4096+2048; rgb_step[ch]=span?(last-first)*4096/span:0;
    }
    for (step=0;step<=span;step++) {
        int32_t x=SignExtend11((uint32_t)(origin[0]>>32));
        int32_t y=SignExtend11((uint32_t)(origin[1]>>32));
        if (PixelInDrawingArea(x,y)) {
            uint16_t *pixel=&g_gpu.vram[(uint32_t)y*1024u+(uint32_t)x], color=0;
            if (!(g_gpu.state.mask_setting&2u) || !(*pixel&0x8000u)) {
                for (ch=0;ch<3;ch++) {
                    int32_t value=rgb[ch]>>12;
                    if (g_gpu.state.draw_mode&0x200u) value+=dither[y&3][x&3];
                    if (value<0) value=0;
                    if (value>255) value=255;
                    color|=(uint16_t)((uint32_t)(value>>3)<<(ch*5));
                }
                if (command&0x02000000u) color=BlendSemiTransparent(*pixel,color);
                if (g_gpu.state.mask_setting&1u) color|=0x8000u;
                *pixel=color; g_gpu.state.line_pixel_count++;
            }
        }
        for (axis=0;axis<2;axis++) origin[axis]+=increment[axis];
        for (ch=0;ch<3;ch++) rgb[ch]+=rgb_step[ch];
    }
}

uint32_t PE_GPU_GP0_PacketWords(uint32_t command)
{
    uint32_t opcode = command >> 24;

    if (command == 0u)
        return 1u;
    if (opcode >= 0xE1u && opcode <= 0xE6u)
        return 1u;
    if (opcode == 0x02u)
        return 3u;
    if ((opcode & 0xE0u) == 0x20u) {
        uint32_t vertices = (opcode & 8u) ? 4u : 3u;
        return 1u + vertices + ((opcode & 4u) ? vertices : 0u) + ((opcode & 16u) ? vertices - 1u : 0u);
    }
    if ((opcode & 0xE8u) == 0x40u)
        return (opcode & 0x10u) ? 4u : 3u;
    if ((opcode & 0xE0u) == 0x60u) {
        if ((opcode & 0x04u) == 0u)   /* untextured: cmd, xy, [size] */
            return MonoRectangleFixedSize(command) != 0u ? 2u : 3u;
        /* Structural length only.  Whether the texture depth/page is
         * executable is decided at execution against the draw mode in
         * force *then* (E1h may precede it in the same packet). */
        /* textured: cmd, xy, uv/clut, [size] for the variable class */
        return MonoRectangleFixedSize(command) != 0u ? 3u : 4u;
    }
    return 0u;
}

/* Length of the GP0 packet starting at words[0] given `avail` words, or 0
 * when it is not a complete executable packet.  Fixed-length commands use
 * PE_GPU_GP0_PacketWords; polylines run to their 5xxx5xxx terminator. */
uint32_t PE_GPU_GP0_StreamPacketWords(const uint32_t *words, uint32_t avail)
{
    uint32_t command, i, vertices = 0u, gouraud, expect_color = 0u;

    if (!avail) return 0u;
    command = words[0];
    if ((command & 0xE8000000u) != 0x48000000u) {
        uint32_t n = PE_GPU_GP0_PacketWords(command);
        return n <= avail ? n : 0u;
    }
    gouraud = (command & 0x10000000u) != 0u;
    for (i = 1u; i < avail; i++) {
        if (vertices >= 2u && (words[i] & 0xF000F000u) == 0x50005000u)
            return i + 1u;
        if (expect_color) { expect_color = 0u; continue; }
        vertices++;
        expect_color = gouraud;
    }
    return 0u;
}

static int GP0WriteInner(uint32_t value);

/* PE_GPU_WATCH=x,y: debug aid, report each GP0 word that changes that VRAM pixel. */
int PE_GPU_WriteGP0(uint32_t value)
{
    static int init; static long wx = -1, wy = -1; static uint32_t last_cmd;
    uint16_t before = 0; int r;
    if (!init) { const char *e = getenv("PE_GPU_WATCH"); init = 1;
        if (e) { char *end; wx = strtol(e, &end, 10); if (*end == ',') wy = strtol(end + 1, 0, 10); } }
    if (wy < 0) return GP0WriteInner(value);
    if (g_gpu.state.gp0_state == PE_GPU_GP0_IDLE) last_cmd = value;
    before = g_gpu.vram[wy * PE_GPU_VRAM_WIDTH + wx];
    r = GP0WriteInner(value);
    if (g_gpu.vram[wy * PE_GPU_VRAM_WIDTH + wx] != before)
        fprintf(stderr, "[GPU_WATCH] (%ld,%ld) %04X->%04X cmd=%08X word=%08X mode=%04X off=%08X\n", wx, wy, before,
                g_gpu.vram[wy * PE_GPU_VRAM_WIDTH + wx], last_cmd, value,
                (unsigned)(g_gpu.state.draw_mode & 0xFFFFu), g_gpu.state.drawing_offset);
    return r;
}

static int GP0WriteInner(uint32_t value)
{
    uint64_t pixels;

    if ((g_gpu.state.status & PE_GPU_STATUS_READY_GP0) == 0) return 0;

    switch (g_gpu.state.gp0_state) {
    case PE_GPU_GP0_POLYGON_DATA:
    case PE_GPU_GP0_LINE_DATA:
        if (g_gpu.polyline) {
            /* psx-spx: the terminator is any word with
             * (w & F000F000h) == 50005000h, recognised once two
             * vertices have been received. */
            if (g_gpu.polyline_vertices >= 2u &&
                (value & 0xF000F000u) == 0x50005000u) {
                g_gpu.polyline = 0;
                g_gpu.state.gp0_state = PE_GPU_GP0_IDLE;
                return 1;
            }
            if (g_gpu.polyline_expect_color) {
                g_gpu.polyline_next_color = value & 0xFFFFFFu;
                g_gpu.polyline_expect_color = 0;
                return 1;
            }
            if (g_gpu.polyline_vertices != 0u) {
                uint32_t gouraud = g_gpu.polyline_command & 0x10000000u;
                g_gpu.polygon_words[0] = (g_gpu.polyline_command & 0xFF000000u & ~0x08000000u) |
                                         g_gpu.polyline_color;
                g_gpu.polygon_words[1] = g_gpu.polyline_xy;
                if (gouraud) {
                    g_gpu.polygon_words[2] = g_gpu.polyline_next_color;
                    g_gpu.polygon_words[3] = value;
                } else g_gpu.polygon_words[2] = value;
                DrawLine();
                if (gouraud) g_gpu.polyline_color = g_gpu.polyline_next_color;
            }
            g_gpu.polyline_xy = value;
            g_gpu.polyline_vertices++;
            g_gpu.polyline_expect_color = (g_gpu.polyline_command & 0x10000000u) != 0u;
            return 1;
        }
        g_gpu.polygon_words[g_gpu.polygon_received++] = value;
        if (g_gpu.polygon_received == g_gpu.polygon_expected) {
            if (g_gpu.state.gp0_state==PE_GPU_GP0_LINE_DATA) DrawLine();
            else DrawPolygon();
            g_gpu.state.gp0_state = PE_GPU_GP0_IDLE;
        }
        return 1;
    case PE_GPU_GP0_IDLE:
        if ((value & 0xE8000000u) == 0x48000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.polyline = 1;
            g_gpu.polyline_command = value;
            g_gpu.polyline_color = value & 0xFFFFFFu;
            g_gpu.polyline_vertices = 0u;
            g_gpu.polyline_expect_color = 0;
            g_gpu.state.gp0_state = PE_GPU_GP0_LINE_DATA;
            return 1;
        }
        if ((value & 0xE0000000u) == 0x20000000u ||
            (value & 0xE8000000u) == 0x40000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.polygon_words[0] = value;
            g_gpu.polygon_received = 1u;
            g_gpu.polygon_expected = PE_GPU_GP0_PacketWords(value);
            g_gpu.state.gp0_state = (value & 0x40000000u) ?
                PE_GPU_GP0_LINE_DATA : PE_GPU_GP0_POLYGON_DATA;
            return 1;
        }
        if (value == 0u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.nop_count++;
            return 1;
        }
        if (value == 0x01000000u) {
            if (g_gpu.state.dma2_active) return 0;
            ResetParser();
            return 1;
        }
        if (value == 0xA0000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.gp0_state = PE_GPU_GP0_EXPECT_POSITION;
            return 1;
        }
        if ((value&0xE0000000u)==0xC0000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.gp0_state=PE_GPU_GP0_READ_EXPECT_POSITION;
            return 1;
        }
        if ((value & 0xFF000000u) == 0xE1000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.draw_mode = value;
            UpdateDrawingStatus();
            g_gpu.state.draw_mode_count++;
            return 1;
        }
        if ((value & 0xFF000000u) == 0xE2000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.texture_window = value;
            g_gpu.state.texture_window_count++;
            return 1;
        }
        if ((value & 0xFF000000u) == 0xE3000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.drawing_area_top_left = value;
            g_gpu.state.drawing_area_top_left_count++;
            return 1;
        }
        if ((value & 0xFF000000u) == 0xE4000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.drawing_area_bottom_right = value;
            g_gpu.state.drawing_area_bottom_right_count++;
            return 1;
        }
        if ((value & 0xFF000000u) == 0xE5000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.drawing_offset = value;
            g_gpu.state.drawing_offset_count++;
            return 1;
        }
        if ((value & 0xFF000000u) == 0xE6000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.mask_setting = value;
            UpdateDrawingStatus();
            g_gpu.state.mask_setting_count++;
            return 1;
        }
        if ((value & 0xE4000000u) == 0x64000000u) {
            /* textured rectangle, any size class (64h..7Fh, bit 26 set) */
            if (g_gpu.state.dma2_active ||
                !TexturedRectangleIndexedModeSupported()) {
                return 0;
            }
            g_gpu.rectangle_command = value;
            g_gpu.state.gp0_state = PE_GPU_GP0_RECT_EXPECT_POSITION;
            return 1;
        }
        if ((value & 0xFF000000u) == 0x02000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.fill_command = value;
            g_gpu.state.gp0_state = PE_GPU_GP0_FILL_EXPECT_POSITION;
            return 1;
        }
        if ((value & 0xE4000000u) == 0x60000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.mono_rectangle_command = value;
            g_gpu.state.gp0_state = PE_GPU_GP0_MONO_RECT_EXPECT_POSITION;
            return 1;
        }
        if (PrimLogOn()) fprintf(g_prim_log, "X %08X rejected\n", value);
        return 0;

    case PE_GPU_GP0_READ_EXPECT_POSITION:
        g_gpu.state.image_x=value&(PE_GPU_VRAM_WIDTH-1u);
        g_gpu.state.image_y=(value>>16u)&(PE_GPU_VRAM_HEIGHT-1u);
        g_gpu.state.gp0_state=PE_GPU_GP0_READ_EXPECT_SIZE;
        return 1;
    case PE_GPU_GP0_READ_EXPECT_SIZE:
        g_gpu.state.image_width=(((value&65535u)-1u)&(PE_GPU_VRAM_WIDTH-1u))+1u;
        g_gpu.state.image_height=(((value>>16u)-1u)&(PE_GPU_VRAM_HEIGHT-1u))+1u;
        g_gpu.state.image_current_pixel=0u;
        g_gpu.state.image_remaining_pixels=g_gpu.state.image_width*g_gpu.state.image_height;
        g_gpu.state.gp0_state=PE_GPU_GP0_READ_DATA;
        g_gpu.state.status|=PE_GPU_STATUS_READY_READ;
        return 1;
    case PE_GPU_GP0_READ_DATA:return 0;
    case PE_GPU_GP0_EXPECT_POSITION:
        g_gpu.state.image_x = value & 0xFFFFu;
        g_gpu.state.image_y = value >> 16;
        g_gpu.state.gp0_state = PE_GPU_GP0_EXPECT_SIZE;
        return 1;

    case PE_GPU_GP0_EXPECT_SIZE:
        g_gpu.state.image_width = value & 0xFFFFu;
        g_gpu.state.image_height = value >> 16;
        XferLog("A0", g_gpu.state.image_x | (g_gpu.state.image_y << 16), value, 0u);
        if (g_gpu.state.image_width == 0 ||
            g_gpu.state.image_width > PE_GPU_VRAM_WIDTH ||
            g_gpu.state.image_height == 0 ||
            g_gpu.state.image_height > PE_GPU_VRAM_HEIGHT) {
            ResetParser();
            return 0;
        }
        pixels = (uint64_t)g_gpu.state.image_width *
                 (uint64_t)g_gpu.state.image_height;
        if (pixels > UINT32_MAX) {
            ResetParser();
            return 0;
        }
        g_gpu.state.image_current_pixel = 0;
        g_gpu.state.image_remaining_pixels = (uint32_t)pixels;
        g_gpu.state.gp0_state = PE_GPU_GP0_IMAGE_DATA;
        return 1;

    case PE_GPU_GP0_IMAGE_DATA:
        if (g_gpu.state.dma2_active) return 0;
        WriteImageWord(value);
        return 1;

    case PE_GPU_GP0_RECT_EXPECT_POSITION:
        if (g_gpu.state.dma2_active) return 0;
        g_gpu.rectangle_position = value;
        g_gpu.state.gp0_state = PE_GPU_GP0_RECT_EXPECT_UV_CLUT;
        return 1;

    case PE_GPU_GP0_RECT_EXPECT_UV_CLUT: {
        uint32_t opcode;

        if (g_gpu.state.dma2_active) return 0;
        g_gpu.rectangle_uv_clut = value;
        opcode = TexturedRectangleFixedSize(g_gpu.rectangle_command);
        if (opcode != 0u) {
            DrawTexturedRectangle(opcode);
            g_gpu.state.rectangle_command = g_gpu.rectangle_command;
            g_gpu.state.rectangle_position = g_gpu.rectangle_position;
            g_gpu.state.rectangle_uv_clut = g_gpu.rectangle_uv_clut;
            g_gpu.state.rectangle_size = opcode;
            g_gpu.state.rectangle_count++;
            ResetParser();
            return 1;
        }
        g_gpu.state.gp0_state = PE_GPU_GP0_RECT_EXPECT_SIZE;
        return 1;
    }

    case PE_GPU_GP0_RECT_EXPECT_SIZE:
        if (g_gpu.state.dma2_active) return 0;
        DrawTexturedRectangle(value);
        g_gpu.state.rectangle_command = g_gpu.rectangle_command;
        g_gpu.state.rectangle_position = g_gpu.rectangle_position;
        g_gpu.state.rectangle_uv_clut = g_gpu.rectangle_uv_clut;
        g_gpu.state.rectangle_size = value;
        g_gpu.state.rectangle_count++;
        ResetParser();
        return 1;

    case PE_GPU_GP0_FILL_EXPECT_POSITION:
        if (g_gpu.state.dma2_active) return 0;
        g_gpu.fill_position = value;
        g_gpu.state.gp0_state = PE_GPU_GP0_FILL_EXPECT_SIZE;
        return 1;

    case PE_GPU_GP0_FILL_EXPECT_SIZE:
        if (g_gpu.state.dma2_active) return 0;
        DrawFillRectangle(value);
        g_gpu.state.fill_command = g_gpu.fill_command;
        g_gpu.state.fill_position = g_gpu.fill_position;
        g_gpu.state.fill_size = value;
        g_gpu.state.fill_count++;
        ResetParser();
        return 1;

    case PE_GPU_GP0_MONO_RECT_EXPECT_POSITION: {
        uint32_t fixed;

        if (g_gpu.state.dma2_active) return 0;
        g_gpu.mono_rectangle_position = value;
        fixed = MonoRectangleFixedSize(g_gpu.mono_rectangle_command);
        if (fixed == 0u) {
            g_gpu.state.gp0_state = PE_GPU_GP0_MONO_RECT_EXPECT_SIZE;
            return 1;
        }
        DrawMonoRectangle(fixed);
        ResetParser();
        return 1;
    }

    case PE_GPU_GP0_MONO_RECT_EXPECT_SIZE:
        if (g_gpu.state.dma2_active) return 0;
        DrawMonoRectangle(value);
        ResetParser();
        return 1;
    }

    return 0;
}

int PE_GPU_WriteGP1(uint32_t value)
{
    unsigned op=value>>24;
    if(op>=5 && op<=8) {
        if(g_gpu.state.display_command_count<4)
            g_gpu.state.display_commands[g_gpu.state.display_command_count]=value;
        g_gpu.state.display_command_count++;
        if(op==5)g_gpu.state.display_start=value&0x7FFFFu;
        if(op==6)g_gpu.state.display_horizontal_range=value&0xFFFFFFu;
        if(op==7)g_gpu.state.display_vertical_range=value&0xFFFFFu;
        if(op==8) {
            g_gpu.state.display_mode=value&255u;
            g_gpu.state.status=(g_gpu.state.status&~0x7F4000u) |
                ((value&0x3Fu)<<17) | ((value&0x40u)<<10) | ((value&0x80u)<<7);
        }
        return 1;
    }
    switch (value) {
    case 0x00000000u: /* reset GPU: represented subset */
        if (g_gpu.state.dma2_active) return 0;
        ResetDisplayRegisters();
        ResetParser();
        g_gpu.state.gp1_dma_direction = 0;
        g_gpu.state.status |= PE_GPU_STATUS_READY_GP0;
        return 1;
    case 0x01000000u: /* reset command buffer only */
        if (g_gpu.state.dma2_active) return 0;
        ResetParser();
        g_gpu.state.status |= PE_GPU_STATUS_READY_GP0;
        return 1;
    case 0x02000000u: /* acknowledge GPU IRQ; no GPU IRQ state represented */
        return 1;
    case 0x04000000u: /* DMA direction off */
        g_gpu.state.gp1_dma_direction = 0;
        return 1;
    case 0x04000002u: /* DMA CPU -> GP0 */
        g_gpu.state.gp1_dma_direction = 2;
        return 1;
    case 0x04000003u: /* DMA GPUREAD -> CPU */
        g_gpu.state.gp1_dma_direction = 3;
        return 1;
    default:
        return 0;
    }
}

/* GP0(80h) copies each row left-to-right unless the destination begins to
 * the right of the source, in which case the row is copied right-to-left.
 * That horizontal overlap direction is hardware-tested by DuckStation's
 * software backend.  Rows advance top-to-bottom.  Split both axes at VRAM
 * wrap boundaries so every inner span has ordinary array bounds. */
static void CopyVramSpan(uint32_t source_x, uint32_t source_y,
                         uint32_t destination_x, uint32_t destination_y,
                         uint32_t width, uint32_t height)
{
    uint32_t row;

    if (source_x < destination_x) {
        for (row = 0u; row < height; row++) {
            uint32_t column = width;
            const uint16_t *source =
                &g_gpu.vram[(source_y + row) * PE_GPU_VRAM_WIDTH + source_x];
            uint16_t *destination =
                &g_gpu.vram[(destination_y + row) * PE_GPU_VRAM_WIDTH +
                            destination_x];

            while (column != 0u) {
                column--;
                destination[column] = source[column];
            }
        }
    } else {
        for (row = 0u; row < height; row++) {
            uint32_t column;
            const uint16_t *source =
                &g_gpu.vram[(source_y + row) * PE_GPU_VRAM_WIDTH + source_x];
            uint16_t *destination =
                &g_gpu.vram[(destination_y + row) * PE_GPU_VRAM_WIDTH +
                            destination_x];

            for (column = 0u; column < width; column++) {
                destination[column] = source[column];
            }
        }
    }
}

int PE_GPU_MoveImage(uint32_t source, uint32_t destination, uint32_t size)
{
    XferLog("80", source, destination, size);
    WatchInit();
    if (g_watch_y >= 0 && g_watch_x >= (long)(destination & 0x3FFu) &&
        g_watch_x < (long)(destination & 0x3FFu) + (long)(size & 0xFFFFu) &&
        g_watch_y >= (long)((destination >> 16) & 0x1FFu) &&
        g_watch_y < (long)((destination >> 16) & 0x1FFu) + (long)(size >> 16))
        fprintf(stderr, "[GPU_WATCH] 80 move src=%08X dst=%08X size=%08X\n", source, destination, size);
    uint32_t source_x;
    uint32_t source_y;
    uint32_t destination_x;
    uint32_t destination_y;
    uint32_t width;
    uint32_t height;
    uint32_t rows_remaining;
    uint32_t current_source_y;
    uint32_t current_destination_y;

    if ((g_gpu.state.status & PE_GPU_STATUS_READY_GP0) == 0u ||
        g_gpu.state.gp0_state != PE_GPU_GP0_IDLE ||
        g_gpu.state.dma2_active ||
        g_gpu.state.gp1_dma_direction != 2u) {
        return 0;
    }

    source_x = source & (PE_GPU_VRAM_WIDTH - 1u);
    source_y = (source >> 16) & (PE_GPU_VRAM_HEIGHT - 1u);
    destination_x = destination & (PE_GPU_VRAM_WIDTH - 1u);
    destination_y = (destination >> 16) & (PE_GPU_VRAM_HEIGHT - 1u);
    width = (((size & 0xFFFFu) - 1u) &
             (PE_GPU_VRAM_WIDTH - 1u)) + 1u;
    height = ((((size >> 16) & 0xFFFFu) - 1u) &
              (PE_GPU_VRAM_HEIGHT - 1u)) + 1u;

    rows_remaining = height;
    current_source_y = source_y;
    current_destination_y = destination_y;
    while (rows_remaining != 0u) {
        uint32_t source_rows = PE_GPU_VRAM_HEIGHT - current_source_y;
        uint32_t destination_rows =
            PE_GPU_VRAM_HEIGHT - current_destination_y;
        uint32_t rows = rows_remaining;
        uint32_t columns_remaining = width;
        uint32_t current_source_x = source_x;
        uint32_t current_destination_x = destination_x;

        if (rows > source_rows) rows = source_rows;
        if (rows > destination_rows) rows = destination_rows;

        while (columns_remaining != 0u) {
            uint32_t source_columns =
                PE_GPU_VRAM_WIDTH - current_source_x;
            uint32_t destination_columns =
                PE_GPU_VRAM_WIDTH - current_destination_x;
            uint32_t columns = columns_remaining;

            if (columns > source_columns) columns = source_columns;
            if (columns > destination_columns) columns = destination_columns;
            CopyVramSpan(current_source_x, current_source_y,
                         current_destination_x, current_destination_y,
                         columns, rows);
            current_source_x =
                (current_source_x + columns) & (PE_GPU_VRAM_WIDTH - 1u);
            current_destination_x =
                (current_destination_x + columns) &
                (PE_GPU_VRAM_WIDTH - 1u);
            columns_remaining -= columns;
        }

        current_source_y =
            (current_source_y + rows) & (PE_GPU_VRAM_HEIGHT - 1u);
        current_destination_y =
            (current_destination_y + rows) &
            (PE_GPU_VRAM_HEIGHT - 1u);
        rows_remaining -= rows;
    }

    g_gpu.state.move_count++;
    g_gpu.state.move_source = source;
    g_gpu.state.move_destination = destination;
    g_gpu.state.move_size = size;
    return 1;
}

int PE_GPU_CanBeginImageLoad(int needs_dma)
{
    if (g_gpu.state.gp0_state != PE_GPU_GP0_IDLE ||
        g_gpu.state.dma2_active) {
        return 0;
    }
    if (needs_dma &&
        (g_gpu.state.dpcr & PE_GPU_DMA2_DPCR_ENABLE) == 0u) {
        return 0;
    }
    return 1;
}

static uint32_t g_dma2_snapshot[PE_GPU_VRAM_PIXELS / 2u + 16u];

int PE_GPU_DMA2Issue(pe_addr_t madr, uint32_t bcr, uint32_t chcr)
{
    uint32_t blocks;
    uint32_t words;
    uint64_t bytes;
    uint32_t required_words;
    int readback=chcr==PE_GPU_DMA2_CHCR_STORE;

    if (g_gpu.state.dma2_active ||
        g_gpu.state.gp0_state != (readback?PE_GPU_GP0_READ_DATA:PE_GPU_GP0_IMAGE_DATA) ||
        g_gpu.state.gp1_dma_direction != (readback?3u:2u) ||
        (g_gpu.state.dpcr & PE_GPU_DMA2_DPCR_ENABLE) == 0 ||
        (!readback && chcr != PE_GPU_DMA2_CHCR_LOAD) ||
        (madr & 3u) != 0 ||
        (bcr & 0xFFFFu) != 0x10u) {
        return 0;
    }

    blocks = bcr >> 16;
    if (blocks == 0) return 0;
    words = blocks * 16u;
    bytes = (uint64_t)words * 4u;
    required_words = (g_gpu.state.image_remaining_pixels + 1u) / 2u;
    if (words != required_words ||
        bytes > SIZE_MAX ||
        !PE_RangeIsRam(madr, (size_t)bytes)) {
        return 0;
    }

    /* psx-spx "DMA": the CPU is halted while a (non-chopped) DMA runs, so
     * a CPU->VRAM block transfer reads its source at issue time.  The
     * native completion is serviced later (event/IRQ timing), so snapshot
     * the source words now; retail code reuses LoadImage buffers right
     * after the call (e.g. func_8006AD40's shadow TIM at 0x8012A8B8). */
    if (!readback) {
        uint32_t k;
        for (k = 0; k < words; k++) g_dma2_snapshot[k] = PE_LoadU32(madr + k * 4u);
    }
    g_gpu.state.dma2_madr = madr;
    g_gpu.state.dma2_bcr = bcr;
    g_gpu.state.dma2_chcr = chcr;
    g_gpu.state.dma2_active = 1;
    g_gpu.state.dma2_source = madr;
    g_gpu.state.dma2_word_count = words;
    g_gpu.event_serial++;
    if (g_gpu.event_serial == 0) g_gpu.event_serial++;
    g_gpu.state.dma2_event_token = g_gpu.event_serial;
    g_gpu.state.dma_data_order = 0;
    g_gpu.state.dma_completion_order = 0;
    return 1;
}

pe_addr_t PE_GPU_ReadDMA2MADR(void)
{
    return g_gpu.state.dma2_madr;
}

uint32_t PE_GPU_ReadDMA2BCR(void)
{
    return g_gpu.state.dma2_bcr;
}

uint32_t PE_GPU_ReadDMA2CHCR(void)
{
    return g_gpu.state.dma2_chcr;
}

int PE_GPU_DMA2Pending(void)
{
    return g_gpu.state.dma2_active;
}

int PE_GPU_DMA2CompletionPending(void)
{
    return (g_gpu.state.dicr & PE_GPU_DMA2_DICR_FLAG) != 0;
}

int PE_GPU_DMA2InterruptAsserted(void)
{
    return DICRMasterFlag(g_gpu.state.dicr);
}

uint64_t PE_GPU_DMA2EventToken(void)
{
    return g_gpu.state.dma2_event_token;
}

int PE_GPU_ServiceDMA2Completion(uint64_t event_token)
{
    uint32_t i;
    pe_addr_t source;

    if (!g_gpu.state.dma2_active || event_token == 0 ||
        event_token != g_gpu.state.dma2_event_token) {
        return 0;
    }

    source = g_gpu.state.dma2_source;
    for (i = 0; i < g_gpu.state.dma2_word_count; i++) {
        if (g_gpu.state.dma2_chcr==PE_GPU_DMA2_CHCR_STORE)
            PE_StoreU32(source+i*4u,ReadImageWord());
        else WriteImageWord(g_dma2_snapshot[i]);
    }
    if (g_gpu.state.image_remaining_pixels != 0) {
        return 0;
    }

    g_gpu.state.dma_data_order = ++g_gpu.order_counter;
    g_gpu.state.dma2_active = 0;
    g_gpu.state.dma2_chcr &= ~PE_GPU_DMA2_CHCR_BUSY;
    if ((g_gpu.state.dicr &
         (PE_GPU_DICR_MASTER | PE_GPU_DMA2_DICR_ENABLE)) ==
        (PE_GPU_DICR_MASTER | PE_GPU_DMA2_DICR_ENABLE)) {
        CommitStoredDICR(g_gpu.state.dicr | PE_GPU_DMA2_DICR_FLAG);
    }
    g_gpu.state.dma_completion_order = ++g_gpu.order_counter;
    g_gpu.state.dma_event_count++;
    return 1;
}

uint32_t PE_GPU_ReadDPCR(void)
{
    return g_gpu.state.dpcr;
}

void PE_GPU_WriteDPCR(uint32_t value)
{
    g_gpu.state.dpcr = value;
}

void PE_GPU_EnableDMA2(void)
{
    g_gpu.state.dpcr |= PE_GPU_DMA2_DPCR_ENABLE;
}

uint32_t PE_GPU_ReadDICR(void)
{
    return g_gpu.state.dicr |
           (DICRMasterFlag(g_gpu.state.dicr) ?
            PE_GPU_DICR_MASTER_FLAG : 0u);
}

uint32_t PE_GPU_ReadStoredDICR(void)
{
    return g_gpu.state.dicr;
}

void PE_GPU_WriteDICR(uint32_t value)
{
    uint32_t flags = g_gpu.state.dicr & PE_GPU_DICR_FLAGS;
    flags &= ~(value & PE_GPU_DICR_FLAGS);
    CommitStoredDICR((value & 0x00FFFFFFu) | flags);
}

void PE_GPU_SetDMA2InterruptEnabled(int enabled)
{
    uint32_t stored = g_gpu.state.dicr;

    if (enabled) {
        stored |= PE_GPU_DICR_MASTER | PE_GPU_DMA2_DICR_ENABLE;
    } else {
        stored &= ~PE_GPU_DMA2_DICR_ENABLE;
        stored |= PE_GPU_DICR_MASTER;
    }
    CommitStoredDICR(stored);
}

void PE_GPU_AcknowledgeDMA2Interrupt(void)
{
    PE_GPU_WriteDICR((g_gpu.state.dicr & 0x00FFFFFFu) |
                     PE_GPU_DMA2_DICR_FLAG);
}

int PE_GPU_DICRRisingEdgePending(void)
{
    return g_gpu.state.dicr_rising_edge_pending;
}

int PE_GPU_TakeDICRRisingEdge(void)
{
    int pending = g_gpu.state.dicr_rising_edge_pending;
    g_gpu.state.dicr_rising_edge_pending = 0;
    return pending;
}

int PE_GPU_LatchDMACompletionFlag(uint32_t dma_channel)
{
    uint32_t enable_bit;
    uint32_t flag_bit;

    if (dma_channel >= 7u) {
        return 0;
    }
    enable_bit = 1u << (16u + dma_channel);
    flag_bit = 1u << (24u + dma_channel);
    if ((g_gpu.state.dicr & (PE_GPU_DICR_MASTER | enable_bit)) ==
        (PE_GPU_DICR_MASTER | enable_bit)) {
        CommitStoredDICR(g_gpu.state.dicr | flag_bit);
    }
    return 1;
}

int PE_GPU_ReadVRAM(uint32_t x, uint32_t y, uint16_t *pixel)
{
    if (!pixel || x >= PE_GPU_VRAM_WIDTH || y >= PE_GPU_VRAM_HEIGHT) {
        return 0;
    }
    *pixel = g_gpu.vram[y * PE_GPU_VRAM_WIDTH + x];
    return 1;
}

uint32_t PE_GPU_VSyncQuery(void)
{
    return g_gpu.state.vsync_count;
}

int PE_GPU_SetHBlank(int level,PeIrqGeneration generation)
{
    if(generation!=PE_IRQ_Generation())return 0;
    uint8_t next=level!=0;
    if(next && !g_gpu.state.hblank_level) {
        g_gpu.state.hblank_edge_count++;
        (void)PE_Timer1_HBlankEdge(generation);
    }
    g_gpu.state.hblank_level=next;
    return 1;
}
int PE_GPU_SetVBlank(int level,PeIrqGeneration generation)
{
    if(generation!=PE_IRQ_Generation())return 0;
    uint8_t next=level!=0;
    if(next && !g_gpu.state.vblank_level) {
        g_gpu.state.vblank_edge_count++;
        (void)PE_Timer1_VBlankEdge(generation);
        (void)PE_IRQ_AssertSourcesForGeneration(1u,generation);
    }
    g_gpu.state.vblank_level=next;
    return 1;
}

void PE_GPU_VBlankStep(void)
{
    g_gpu.state.vsync_count++;
    PE_AudioDriver_VBlank();   /* audio lane: 240 Hz driver + SPU output */
}

void PE_GPU_GetState(PeGpuState *out)
{
    if (out) {
        *out = g_gpu.state;
        out->dicr = PE_GPU_ReadDICR();
    }
}
