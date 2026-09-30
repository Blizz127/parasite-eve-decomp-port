/*
 * pe_plat renderer interface (docs/ARCHITECTURE-PORT.md, step P0).
 *
 * Defined by meaning, not by PS1 registers: callers hand over VRAM
 * rectangles, pixel buffers, drawing/display environments and primitives
 * described by vertices, colours and texture coordinates.  A backend decides
 * how to realise them.  The PS1-accurate backend (pe_gpu, platform/
 * plat_renderer.c) encodes them into its GP0/GP1 substrate; a modern backend
 * may upscale, replace textures or widen the view without game changes.
 *
 * Pixels are PS1 15-bit colour (bit 15 = mask/semi-transparency flag) in a
 * 1024x512 halfword VRAM space.  Texture locations (`clut`, `tpage`) are
 * meaning-level VRAM addresses: `clut` = palette position (x/16 | y<<6),
 * `tpage` = texture page base, colour depth and blend mode
 * (x/64 | (y/256)<<4 | blend<<5 | depth<<7).
 *
 * Game code must include only pe_plat headers (never pe_gpu.h).
 */
#ifndef PE_PLAT_RENDERER_H
#define PE_PLAT_RENDERER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_PLAT_VRAM_WIDTH  1024
#define PE_PLAT_VRAM_HEIGHT  512

typedef struct { int16_t x, y, w, h; } PePlatRect;
typedef struct { int16_t x, y; } PePlatVertex;
typedef struct { uint8_t r, g, b; } PePlatColor;
typedef struct { uint8_t u, v; } PePlatUV;

/* Drawing environment: where primitives land and how they are offset. */
typedef struct {
    PePlatRect clip;        /* drawing area (inclusive of x..x+w-1) */
    int16_t offset_x;       /* added to every vertex */
    int16_t offset_y;
} PePlatDrawEnv;

/* Display environment: which VRAM rectangle is shown. */
typedef struct {
    int16_t x, y;           /* top-left of the displayed VRAM area */
} PePlatDispEnv;

typedef enum {
    PE_PLAT_PRIM_TRIANGLE = 0,  /* v[0..2] */
    PE_PLAT_PRIM_QUAD,          /* v[0..3], PS1 order: (0,1,2) + (1,2,3) */
    PE_PLAT_PRIM_LINE,          /* v[0..1] */
    PE_PLAT_PRIM_TILE,          /* v[0] + w/h, solid colour c[0] */
    PE_PLAT_PRIM_SPRITE         /* v[0] + w/h, uv[0], clut, tpage */
} PePlatPrimKind;

#define PE_PLAT_PRIM_GOURAUD          0x01u  /* per-vertex colour c[i] */
#define PE_PLAT_PRIM_TEXTURED         0x02u  /* uv[i], clut, tpage */
#define PE_PLAT_PRIM_SEMI_TRANSPARENT 0x04u  /* blend mode from tpage */
#define PE_PLAT_PRIM_RAW_TEXTURE      0x08u  /* texture not modulated */

typedef struct {
    PePlatPrimKind kind;
    uint32_t flags;             /* PE_PLAT_PRIM_* */
    PePlatVertex v[4];
    PePlatColor c[4];           /* c[0] only unless GOURAUD */
    PePlatUV uv[4];
    uint16_t clut;
    uint16_t tpage;
    uint16_t w, h;              /* TILE / SPRITE size */
} PePlatPrimitive;

typedef struct {
    int internal_scale;         /* 1 = native 1024x512 VRAM */
    int widescreen;             /* 0 = 4:3 */
    const char *backend_name;
} PePlatRendererCaps;

/* Lifecycle.  init clears VRAM; reset keeps VRAM (soft GPU reset). */
void pe_plat_renderer_init(void);
void pe_plat_renderer_reset(void);
void pe_plat_renderer_caps(PePlatRendererCaps *out);

/* VRAM transfers.  Rectangles must lie inside VRAM (no wrap); return 1 on
 * success, 0 when rejected (bad rectangle or backend busy). */
int pe_plat_renderer_load_image(const PePlatRect *rect, const uint16_t *pixels);
int pe_plat_renderer_store_image(const PePlatRect *rect, uint16_t *pixels);
int pe_plat_renderer_move_image(const PePlatRect *src, int16_t dst_x, int16_t dst_y);
int pe_plat_renderer_clear_image(const PePlatRect *rect, PePlatColor color);

/* Environments. */
int pe_plat_renderer_put_draw_env(const PePlatDrawEnv *env);
int pe_plat_renderer_put_disp_env(const PePlatDispEnv *env);

/* Draw one primitive.  Returns 1 when the backend accepted it. */
int pe_plat_renderer_draw(const PePlatPrimitive *prim);

/* All work so far is complete (the in-house backend is synchronous). */
int pe_plat_renderer_draw_sync(void);

/* Diagnostics / tests: read one VRAM pixel.  1 on success. */
int pe_plat_renderer_read_pixel(int x, int y, uint16_t *pixel);

#ifdef __cplusplus
}
#endif

#endif /* PE_PLAT_RENDERER_H */
