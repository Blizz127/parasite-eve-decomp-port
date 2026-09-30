/*
 * pe_plat renderer -> in-house pe_gpu backend (step P0 adapter).
 *
 * Encodes meaning-level requests (include/pe_plat/renderer.h) into the
 * pe_gpu GP0/GP1 substrate.  Pure adapter: no state of its own beyond what
 * pe_gpu holds.  Hardware facts used here come from psx-spx (GPU command
 * encoding); no Psy-Q code or tables.
 */
#include "pe_plat/renderer.h"

#include <stddef.h>

#include "pe_gpu.h"

static uint32_t plat_rgb(PePlatColor c)
{
    return (uint32_t)c.r | ((uint32_t)c.g << 8) | ((uint32_t)c.b << 16);
}

static uint32_t plat_xy(int x, int y)
{
    return ((uint32_t)(uint16_t)y << 16) | (uint16_t)x;
}

static int plat_rect_in_vram(const PePlatRect *r)
{
    return r && r->w > 0 && r->h > 0 && r->x >= 0 && r->y >= 0 &&
           r->x + r->w <= PE_PLAT_VRAM_WIDTH &&
           r->y + r->h <= PE_PLAT_VRAM_HEIGHT;
}

/* Write a complete GP0 packet; on a mid-packet rejection the command buffer
 * is reset so the parser never stays half-way through a packet. */
static int plat_gp0_packet(const uint32_t *words, uint32_t count)
{
    uint32_t i;
    for (i = 0; i < count; i++) {
        if (!PE_GPU_WriteGP0(words[i])) {
            if (i) PE_GPU_WriteGP0(0x01000000u);
            return 0;
        }
    }
    return 1;
}

void pe_plat_renderer_init(void)  { PE_GPU_Init(); }
void pe_plat_renderer_reset(void) { PE_GPU_Reset(); }

void pe_plat_renderer_caps(PePlatRendererCaps *out)
{
    if (!out) return;
    out->internal_scale = 1;
    out->widescreen = 0;
    out->backend_name = "pe_gpu";
}

int pe_plat_renderer_load_image(const PePlatRect *rect, const uint16_t *pixels)
{
    uint32_t hdr[3], n, i;
    if (!plat_rect_in_vram(rect) || !pixels || !PE_GPU_CanBeginImageLoad(0))
        return 0;
    hdr[0] = 0xA0000000u;
    hdr[1] = plat_xy(rect->x, rect->y);
    hdr[2] = plat_xy(rect->w, rect->h);
    if (!plat_gp0_packet(hdr, 3)) return 0;
    n = (uint32_t)rect->w * (uint32_t)rect->h;
    for (i = 0; i + 1 < n; i += 2)
        if (!PE_GPU_WriteGP0(pixels[i] | ((uint32_t)pixels[i + 1] << 16)))
            return 0;
    if (i < n && !PE_GPU_WriteGP0(pixels[i])) return 0;
    return 1;
}

int pe_plat_renderer_store_image(const PePlatRect *rect, uint16_t *pixels)
{
    int x, y;
    if (!plat_rect_in_vram(rect) || !pixels) return 0;
    for (y = 0; y < rect->h; y++)
        for (x = 0; x < rect->w; x++)
            if (!PE_GPU_ReadVRAM((uint32_t)(rect->x + x), (uint32_t)(rect->y + y),
                                 &pixels[(size_t)y * (size_t)rect->w + (size_t)x]))
                return 0;
    return 1;
}

int pe_plat_renderer_move_image(const PePlatRect *src, int16_t dst_x, int16_t dst_y)
{
    PeGpuState st;
    PePlatRect dst;
    int ok;
    if (!plat_rect_in_vram(src)) return 0;
    dst.x = dst_x; dst.y = dst_y; dst.w = src->w; dst.h = src->h;
    if (!plat_rect_in_vram(&dst)) return 0;
    /* The substrate copies only with the CPU->GP0 transfer direction set;
     * select it for the copy and restore the caller's direction. */
    PE_GPU_GetState(&st);
    if (st.gp1_dma_direction != 2u) PE_GPU_WriteGP1(0x04000002u);
    ok = PE_GPU_MoveImage(plat_xy(src->x, src->y), plat_xy(dst_x, dst_y),
                          plat_xy(src->w, src->h));
    if (st.gp1_dma_direction != 2u)
        PE_GPU_WriteGP1(0x04000000u | (st.gp1_dma_direction & 3u));
    return ok;
}

/* PS1 backend: the fill is hardware-granular (x rounded down and width
 * rounded up to 16 pixels), exactly like the console. */
int pe_plat_renderer_clear_image(const PePlatRect *rect, PePlatColor color)
{
    uint32_t w[3];
    if (!plat_rect_in_vram(rect)) return 0;
    w[0] = 0x02000000u | plat_rgb(color);
    w[1] = plat_xy(rect->x, rect->y);
    w[2] = plat_xy(rect->w, rect->h);
    return plat_gp0_packet(w, 3);
}

int pe_plat_renderer_put_draw_env(const PePlatDrawEnv *env)
{
    uint32_t w[3];
    int x1, y1;
    if (!env || env->clip.w <= 0 || env->clip.h <= 0) return 0;
    x1 = env->clip.x + env->clip.w - 1;
    y1 = env->clip.y + env->clip.h - 1;
    w[0] = 0xE3000000u | ((uint32_t)(env->clip.y & 0x3FF) << 10) | (uint32_t)(env->clip.x & 0x3FF);
    w[1] = 0xE4000000u | ((uint32_t)(y1 & 0x3FF) << 10) | (uint32_t)(x1 & 0x3FF);
    w[2] = 0xE5000000u | ((uint32_t)(env->offset_y & 0x7FF) << 11) | (uint32_t)(env->offset_x & 0x7FF);
    return plat_gp0_packet(w, 3);
}

int pe_plat_renderer_put_disp_env(const PePlatDispEnv *env)
{
    if (!env || env->x < 0 || env->y < 0 ||
        env->x >= PE_PLAT_VRAM_WIDTH || env->y >= PE_PLAT_VRAM_HEIGHT)
        return 0;
    return PE_GPU_WriteGP1(0x05000000u | ((uint32_t)env->y << 10) | (uint32_t)env->x);
}

int pe_plat_renderer_draw(const PePlatPrimitive *p)
{
    uint32_t w[16], n = 0, cmd, i, verts;
    int gouraud, textured;
    if (!p) return 0;
    gouraud = (p->flags & PE_PLAT_PRIM_GOURAUD) != 0;
    textured = (p->flags & PE_PLAT_PRIM_TEXTURED) != 0;

    switch (p->kind) {
    case PE_PLAT_PRIM_TRIANGLE:
    case PE_PLAT_PRIM_QUAD:
        verts = p->kind == PE_PLAT_PRIM_QUAD ? 4u : 3u;
        cmd = 0x20u | (verts == 4u ? 0x08u : 0u) | (gouraud ? 0x10u : 0u) |
              (textured ? 0x04u : 0u);
        break;
    case PE_PLAT_PRIM_LINE:
        if (textured) return 0;
        verts = 2u;
        cmd = 0x40u | (gouraud ? 0x10u : 0u);
        break;
    case PE_PLAT_PRIM_TILE:
        verts = 1u;
        cmd = 0x60u;
        break;
    case PE_PLAT_PRIM_SPRITE:
        verts = 1u;
        cmd = 0x64u;
        textured = 1;
        break;
    default:
        return 0;
    }
    if (p->flags & PE_PLAT_PRIM_SEMI_TRANSPARENT) cmd |= 0x02u;
    if (textured && (p->flags & PE_PLAT_PRIM_RAW_TEXTURE)) cmd |= 0x01u;

    if (p->kind == PE_PLAT_PRIM_SPRITE) {
        /* Sprites take their texture page from the draw mode. */
        uint32_t mode = 0xE1000000u | (p->tpage & 0x9FFu);
        if (!PE_GPU_WriteGP0(mode)) return 0;
    }

    w[n++] = (cmd << 24) | plat_rgb(p->c[0]);
    for (i = 0; i < verts; i++) {
        if (i && gouraud) w[n++] = plat_rgb(p->c[i]);
        w[n++] = plat_xy(p->v[i].x, p->v[i].y);
        if (textured) {
            uint32_t hi = i == 0 ? p->clut : (i == 1 && p->kind != PE_PLAT_PRIM_SPRITE ? p->tpage : 0u);
            w[n++] = ((uint32_t)hi << 16) | ((uint32_t)p->uv[i].v << 8) | p->uv[i].u;
        }
    }
    if (p->kind == PE_PLAT_PRIM_TILE || p->kind == PE_PLAT_PRIM_SPRITE)
        w[n++] = plat_xy(p->w, p->h);

    /* Admission: the backend must know this packet at exactly this length. */
    if (PE_GPU_GP0_PacketWords(w[0]) != n) return 0;
    return plat_gp0_packet(w, n);
}

int pe_plat_renderer_draw_sync(void) { return 0; }

int pe_plat_renderer_read_pixel(int x, int y, uint16_t *pixel)
{
    if (x < 0 || y < 0 || x >= PE_PLAT_VRAM_WIDTH || y >= PE_PLAT_VRAM_HEIGHT)
        return 0;
    return PE_GPU_ReadVRAM((uint32_t)x, (uint32_t)y, pixel);
}
