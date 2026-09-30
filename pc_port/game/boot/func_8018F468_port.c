/*
 * Title object update/draw leaf func_8018F468 and its sprite compositor
 * func_8018F7F0.
 *
 * Retail overlay: [0x8018F468, 0x8018F7F0), 226 words / 0x388, and
 * [0x8018F7F0, 0x8018F958), 90 words.  Retranslated (fmv lane) from the
 * PE.IMG overlay bytes (LBA 1013 + 0x03D2, load VA 0x8018EFF0):
 *   1. walk D_801D1370: jalr each node's +0xC draw, accumulate the
 *      bounding box (s1,s2)-(s3,s4);
 *   2. dirty rect = UNION of this box and the previous one kept at
 *      DRAWENV +0x78..+0x7E (0x8018F544..F5E4);
 *   3. restore the 24-bit title background for the dirty rect from the
 *      overlay TIM (D_80193254 + *D_80193258, pixels at +0x14, 320 px /
 *      960-byte rows, image origin y=20) into the packed buffer at
 *      DRAWENV +0x8080 (0x8018F608..F6C4);
 *   4. jalr each node's +0x10 updater with (node, dst, skip-words,
 *      row-words) addressing that buffer (0x8018F6DC..F778);
 *   5. publish box (+0x78..+0x7E) and dirty rect (+0x70..+0x76).
 * func_8018F7F0 max-blends the node's sprite (+0x18, pixels at +0x14)
 * scaled by +0x1C/256 over the buffer, skipping all-zero source words.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_guestcode.h"

#define GA_TITLE_LIST          0x801D1370u
#define GA_ENVIRONMENT_CURRENT 0x801D11C4u
#define GA_TITLE_DRAW_FN       0x80192FE8u
#define GA_TITLE_UPDATE_FN     0x8018F7F0u
#define GA_OVERLAY_ANCHOR      0x80193254u
#define GA_OVERLAY_TIM_OFFSET  0x80193258u

void func_8018F7F0(pe_addr_t node, pe_addr_t dst, int32_t skip_words,
                   int32_t row_words);

static int32_t Div4(int32_t v) { return (v < 0 ? v + 3 : v) >> 2; }

void func_8018F7F0(pe_addr_t node, pe_addr_t dst, int32_t skip_words,
                   int32_t row_words)
{
    int32_t words = Div4((int16_t)PE_LoadU16(node + 8u) * 3);
    uint32_t alpha = PE_LoadU32(node + 0x1Cu);
    int32_t rows = (int16_t)PE_LoadU16(node + 0x0Au);
    pe_addr_t src = PE_LoadU32(node + 0x18u) + 20u;
    int32_t row;

    (void)row_words;
    for (row = 0; row < rows; row++) {
        int32_t w;

        for (w = 0; w < words; w++) {
            unsigned b;

            if (PE_LoadU32(src) == 0u) {
                src += 4u;
                dst += 4u;
                continue;
            }
            for (b = 0u; b < 4u; b++) {
                int32_t v = (int32_t)((int32_t)PE_LoadU8(src) *
                                      (int32_t)alpha) >> 8;
                int32_t d = (int32_t)PE_LoadU8(dst);

                if (v < d)
                    v = d;
                PE_StoreU8(dst, (uint8_t)v);
                src++;
                dst++;
            }
        }
        dst += (uint32_t)skip_words << 2;
    }
}

extern int PE_TitleNodeCall(const char *site, pe_addr_t fn, pe_addr_t node,
                            pe_addr_t a1, int32_t a2, int32_t a3);

static void title_call_draw(pe_addr_t node)
{
    pe_addr_t draw = PE_LoadU32(node + 0x0Cu);

    if (draw != 0u)
        (void)PE_TitleNodeCall("func_8018F468_draw", draw, node, 0u, 0, 0);
}

static void title_call_update(pe_addr_t update, pe_addr_t node,
                              pe_addr_t dst, int32_t skip, int32_t row)
{
    (void)PE_TitleNodeCall("func_8018F468_update", update, node, dst, skip,
                           row);
}

void func_8018F468(void)
{
    pe_addr_t node;
    pe_addr_t env;
    int32_t min_x = 0x7FFF;  /* s1 */
    int32_t min_y = 0x7FFF;  /* s2 */
    int32_t max_x = 0;       /* s3 */
    int32_t max_y = 0;       /* s4 */
    int16_t clip_x, clip_y, clip_w, clip_h; /* sp+16..sp+22 */

    for (node = PE_LoadU32(GA_TITLE_LIST); node != 0u;
         node = PE_LoadU32(node)) {
        int32_t x, y;

        title_call_draw(node);
        if (PE_Port_ShouldStop())
            return;
        x = (int16_t)PE_LoadU16(node + 0x04u);
        if (x < min_x)
            min_x = x;
        y = (int16_t)PE_LoadU16(node + 0x06u);
        if (y < min_y)
            min_y = y;
        x += (int16_t)PE_LoadU16(node + 0x08u);
        if (max_x < x)
            max_x = x;
        y += (int16_t)PE_LoadU16(node + 0x0Au);
        if (max_y < y)
            max_y = y;
    }

    env = PE_LoadU32(GA_ENVIRONMENT_CURRENT);
    if ((int16_t)PE_LoadU16(env + 0x7Cu) > 0) {
        int32_t px = (int16_t)PE_LoadU16(env + 0x78u);
        int32_t py = (int16_t)PE_LoadU16(env + 0x7Au);
        int32_t right = px + (int16_t)PE_LoadU16(env + 0x7Cu);
        int32_t bottom = py + (int16_t)PE_LoadU16(env + 0x7Eu);

        clip_x = (int16_t)(px < min_x ? px : min_x);
        clip_y = (int16_t)(py < min_y ? py : min_y);
        clip_w = (int16_t)((max_x < right ? right : max_x) - clip_x);
        clip_h = (int16_t)((max_y < bottom ? bottom : max_y) - clip_y);
    } else {
        clip_x = (int16_t)min_x;
        clip_y = (int16_t)min_y;
        clip_w = (int16_t)(max_x - min_x);
        clip_h = (int16_t)(max_y - min_y);
    }

    if (clip_w > 0 && clip_h > 0) {
        int32_t offset = Div4((((int32_t)clip_y - 20) * 320 + clip_x) * 3);
        pe_addr_t src = PE_LoadU32(GA_OVERLAY_TIM_OFFSET) +
                        GA_OVERLAY_ANCHOR + (uint32_t)(offset << 2) + 20u;
        pe_addr_t dst = PE_LoadU32(GA_ENVIRONMENT_CURRENT) + 0x8080u;
        int32_t words = Div4((int32_t)clip_w * 3);
        uint32_t stride = (uint32_t)(240 - words) << 2;
        int32_t row;

        for (row = 0; row < clip_h; row++) {
            int32_t w;

            for (w = 0; w < words; w++) {
                PE_StoreU32(dst, PE_LoadU32(src));
                src += 4u;
                dst += 4u;
            }
            src += stride;
        }

        for (node = PE_LoadU32(GA_TITLE_LIST); node != 0u;
             node = PE_LoadU32(node)) {
            pe_addr_t update = PE_LoadU32(node + 0x10u);
            int32_t rel;

            if (update == 0u)
                continue;
            rel = (int32_t)clip_w *
                      ((int16_t)PE_LoadU16(node + 0x06u) - clip_y) +
                  ((int16_t)PE_LoadU16(node + 0x04u) - clip_x);
            title_call_update(
                update, node,
                PE_LoadU32(GA_ENVIRONMENT_CURRENT) + 0x8080u +
                    (uint32_t)(Div4(rel * 3) << 2),
                Div4((clip_w - (int16_t)PE_LoadU16(node + 0x08u)) * 3),
                Div4((int32_t)clip_w * 3));
            if (PE_Port_ShouldStop())
                return;
        }
    }

    env = PE_LoadU32(GA_ENVIRONMENT_CURRENT);
    PE_StoreU16(env + 0x7Cu, (uint16_t)(max_x - min_x));
    PE_StoreU16(env + 0x78u, (uint16_t)min_x);
    PE_StoreU16(env + 0x7Au, (uint16_t)min_y);
    PE_StoreU16(env + 0x7Eu, (uint16_t)(max_y - min_y));
    PE_StoreU16(env + 0x70u, (uint16_t)clip_x);
    PE_StoreU16(env + 0x72u, (uint16_t)clip_y);
    PE_StoreU16(env + 0x74u, (uint16_t)clip_w);
    PE_StoreU16(env + 0x76u, (uint16_t)clip_h);
}
