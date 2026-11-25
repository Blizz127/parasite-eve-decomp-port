/* PutDispEnv (guest 0x800755F0), written clean-room from black-box vectors of
 * the original code (pc_port/tools/pe_gpu_rows_vectors.py `dispenv`, checked
 * by pe_gpu_rows_check.c set D) plus the GPU documentation (psx-spx GP1(05h)
 * display area start, GP1(06h)/(07h) horizontal/vertical display range,
 * GP1(08h) display mode).  No SDK source, matched C or disassembly was read;
 * see docs/evidence/native-gpu-rows-20261125/.
 *
 * DISPENV layout (20 bytes, int16 unless noted): disp {x, y, w, h} at +0,
 * screen {x, y, w, h} at +8, is-interlaced byte +0x10, is-24-bit byte +0x11,
 * pad0 byte +0x12 (used as scratch: it ends holding the video mode byte), +0x13.
 * Guest data: the video mode word 0x800956EC (0 NTSC, 1 PAL); the byte
 * 0x8009574F (reverse flag, GP1(08h) bit 7); a table of (x1, x2) uint16 pairs at
 * 0x80095820 indexed by hres class + 5 * video mode byte; clock divisors per
 * class at 0x80095848; the cached copy of the last environment at 0x800957B8
 * (word 0x800957C8 holds its flag word).
 *
 * Contract (from vectors): the command words are sent through the GPU jump
 * table slot +0x10 (0x80076B20) in this order: GP1(05h) always; GP1(08h) only
 * if the display part (flag word, x, y, w, h) differs from the cache; GP1(06h)
 * and GP1(07h) if the screen part differs OR the environment's scratch byte +0x12 holds 8 (the marker the
 * display branch leaves there; the vectors of set D2 showed it with a pre-set 8 and an unchanged cache); then the 20-byte environment is
 * copied into the cache (BIOS memcpy) and the environment pointer is returned.
 * Display mode word: 0x08000000 | hres bits (class 0..4 -> 0, 1, 0x40, 2, 3;
 * class from the signed width: <=280, <=352, <=400, <=560, wider) | 0x24 if
 * the signed height is above 256 (288 when the video mode byte is non-zero), else 0x20 if interlaced | 0x10 if 24-bit |
 * 0x08 if the video mode byte is 1 | 0x80 if the reverse byte is non-zero.
 * Range words: x1 = tab.x1 + sx * div, x2 = x1 + w * (tab.x2 - tab.x1) / 256
 * (w = 256 when the screen width is 0), x1 clamped to [500, 3250], x2 to
 * [x1 + 4 * div, 3290] (PAL, i.e. video mode byte != 0: [540, 3220] and 3260); y1 = ymin + sy, y2 = y1 + h (240 when 0), ymin 16 (NTSC)
 * or 19, y1 clamped to [ymin, 257 | 303], y2 to [y1 + 2, 258 | 305]. */
#include <stdint.h>
#include "psx_compat.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "game_port.h"
#include "pe_bios_string.h"
#include "pe_plat/gpu_putdispenv.h"
#include "pe_plat/gpu_regs.h"
#include "pe_plat/gpu_host.h"

#define B_DEBUG    0x8009574Eu
#define W_TRACE    0x80095748u
#define W_TABLE    0x80095744u
#define W_VMODE    0x800956ECu
#define B_REVERSE  0x8009574Fu
#define TAB_X      0x80095820u
#define TAB_DIV    0x80095848u
#define CACHE      0x800957B8u
#define W_CACHEFL  0x800957C8u
#define FMT_TRACE  0x80011970u
#define BIOS_PRINTF 0x80071A74u
#define SLOT_CTL   0x10u

static int32_t clamp32(int32_t v, int32_t lo, int32_t hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int send(uint32_t cmd)
{
    uint32_t root = PE_LoadU32(W_TABLE);
    uint32_t target;
    /* Host: a table pointer that is not guest RAM (library jump table never set up) takes the native
     * control write too (retail would fault). */
    if (!pe_gpu_reg_ok(root + SLOT_CTL, 4)) { func_80076B20(cmd); return 1; }
    target = PE_LoadU32(root + SLOT_CTL);
    int ret = 0;
    unsigned epoch;
    /* A zero slot (jump table not set up yet, as in the unit tests) goes to the native control write too. */
    if (target == 0x80076B20u || target == 0u) {
        func_80076B20(cmd);
        return 1;
    }
    epoch = PE_Port_StopEpoch();
    if (!PE_GuestHook_Call("PutDispEnv_ctl", "func_800755F0", target, 1, cmd, 0, 0, 0, &ret) ||
        PE_Port_StopEpoch() != epoch) return 0;
    return 1;
}

static int hres_class(int16_t w)
{
    return w <= 280 ? 0 : w <= 352 ? 1 : w <= 400 ? 2 : w <= 560 ? 3 : 4;
}

uint32_t pe_plat_gpu_put_disp_env(uint32_t env)
{
    int16_t dx, dy, dw, sx, sy, sw, sh;
    uint32_t flags, vm;
    int changed, screen_changed;
    if (PE_LoadU8(B_DEBUG) >= 2u) {
        uint32_t trace = PE_LoadU32(W_TRACE);
        int ret = 0;
        unsigned epoch = PE_Port_StopEpoch();
        if (trace == BIOS_PRINTF) {
            uint32_t arg = env;
            (void)PE_Bios_Printf(FMT_TRACE, &arg, 1);
        } else {
            /* retail's jalr through the hook word: the generic guest-call boundary names this function */
            (void)ret;
            (void)PE_GuestCall("func_800755F0", trace, 2, FMT_TRACE, env, 0, 0);
        }
        if (PE_Port_StopEpoch() != epoch) return 0;
    }
    dy = (int16_t)PE_LoadU16(env + 2u);
    dx = (int16_t)PE_LoadU16(env + 0u);
    if (!send(0x05000000u | (((uint32_t)dy & 0x3FFu) << 10) | ((uint32_t)dx & 0x3FFu))) return 0;

    /* display part */
    flags = PE_LoadU32(env + 0x10u);
    changed = PE_LoadU32(W_CACHEFL) != flags ||
              PE_LoadU16(CACHE + 0u) != PE_LoadU16(env + 0u) || PE_LoadU16(CACHE + 2u) != PE_LoadU16(env + 2u) ||
              PE_LoadU16(CACHE + 4u) != PE_LoadU16(env + 4u) || PE_LoadU16(CACHE + 6u) != PE_LoadU16(env + 6u);
    if (changed) {
        uint32_t mode;
        int16_t w, h;
        vm = PE_LoadU32(W_VMODE);
        PE_StoreU8(env + 0x12u, (uint8_t)vm);
        w = (int16_t)PE_LoadU16(env + 4u);
        h = (int16_t)PE_LoadU16(env + 6u);
        mode = 0x08000000u;
        mode |= (uint32_t[]){0u, 1u, 0x40u, 2u, 3u}[hres_class(w)];
        if (h > (PE_LoadU8(env + 0x12u) == 0u ? 256 : 288)) mode |= 0x24u;
        else if (PE_LoadU8(env + 0x10u) != 0u) mode |= 0x20u;
        if (PE_LoadU8(env + 0x11u) != 0u) mode |= 0x10u;
        if (PE_LoadU8(env + 0x12u) == 1u) mode |= 0x08u;
        if (PE_LoadU8(B_REVERSE) != 0u) mode |= 0x80u;
        if (!send(mode)) return 0;
        PE_StoreU8(env + 0x12u, 8u);
    }

    /* screen part: runs when it differs from the cache or the display branch left its marker (8) */
    screen_changed = PE_LoadU16(CACHE + 8u) != PE_LoadU16(env + 8u) || PE_LoadU16(CACHE + 10u) != PE_LoadU16(env + 10u) ||
                     PE_LoadU16(CACHE + 12u) != PE_LoadU16(env + 12u) || PE_LoadU16(CACHE + 14u) != PE_LoadU16(env + 14u);
    if (screen_changed || PE_LoadU8(env + 0x12u) == 8u) {   /* 8 = the scratch byte's "display part was sent" marker */
        int cls, pal;
        int32_t div, tx1, tx2, x1, x2, y1, y2, ymin, w;
        uint32_t vmb;
        vm = PE_LoadU32(W_VMODE);
        PE_StoreU8(env + 0x12u, (uint8_t)vm);
        sy = (int16_t)PE_LoadU16(env + 10u);
        sh = (int16_t)PE_LoadU16(env + 14u);
        dw = (int16_t)PE_LoadU16(env + 4u);
        sx = (int16_t)PE_LoadU16(env + 8u);
        cls = hres_class(dw);
        div = (int32_t)PE_LoadU8(TAB_DIV + (uint32_t)cls);
        vmb = PE_LoadU8(env + 0x12u);
        tx1 = (int32_t)PE_LoadU16(TAB_X + 4u * ((uint32_t)cls + 5u * vmb));
        tx2 = (int32_t)PE_LoadU16(TAB_X + 4u * ((uint32_t)cls + 5u * vmb) + 2u);
        sw = (int16_t)PE_LoadU16(env + 12u);
        w = sw == 0 ? 256 : sw;
        x1 = tx1 + (int32_t)sx * div;
        x2 = x1 + (w * (tx2 - tx1)) / 256;
        pal = vmb != 0u;
        x1 = clamp32(x1, pal ? 540 : 500, pal ? 3220 : 3250);
        x2 = clamp32(x2, x1 + 4 * div, pal ? 3260 : 3290);
        ymin = pal ? 19 : 16;
        y1 = ymin + sy;
        y2 = y1 + (sh == 0 ? 240 : sh);
        y1 = clamp32(y1, ymin, pal ? 303 : 257);
        y2 = clamp32(y2, y1 + 2, pal ? 305 : 258);
        if (!send(0x06000000u | (((uint32_t)x2 & 0xFFFu) << 12) | ((uint32_t)x1 & 0xFFFu))) return 0;
        if (!send(0x07000000u | (((uint32_t)y2 & 0x3FFu) << 10) | ((uint32_t)y1 & 0x3FFu))) return 0;
    }
    (void)func_80071A34(CACHE, env, 0x14);
    /* Host (not in the vectors): the GP1 05h..08h writes above set the GPU model's display
     * start / range / mode; the host framebuffer presents the environment's display area. */
    pe_plat_gpu_host_present_dispenv(env);
    return env;
}
