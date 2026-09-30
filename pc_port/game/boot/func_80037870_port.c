/*
 * PE-BTL55 — func_80037870 message updater named cut
 * (translated retail sites, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 1064 words 0x80037870..0x80038910, SHA-256
 * 2b0eeaf2ed110c388b805b4cb3759bcf0161f1d35cc434a218d5b8f68d8c9d4d.
 * Sole live jal 3F3C4 @ 0x8003F568 when B0CD8&0x200==0.
 *
 * TXT1 extends the TXT0 state machine with the retail ordinary-glyph
 * DR_MODE/SPRT path (38704..38890), page redraw, newline and page advance:
 *   state 1: copy gp+0x120 (D_8009CE90) to +0x04, scan
 *     (FF|F9) FE <id>, cursor = marker+3
 *   parse until a yielding control
 *   F9 → state 0
 *   FF + state!=2 → parser_stop (388AC promotes 1→2)
 *   FF + state==2 + (D1F4&0x100) + !(flags&0x02000000) → 0
 *   FB 07 nn pauses on +0x0D
 *
 * Custom window frames, substituted numbers and extended glyph pages are
 * still outside this cut. Ordinary field dialogue uses the disc font atlas,
 * bearing/advance table and per-bank packet arena, not a host text overlay.
 * Host refuses to walk a non-KSEG stream (ROM would load
 * address 0). Scan length is capped at 0x10000 —
 * APPROXIMATION; ROM has no bound.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_guest_ram.h"

#define GA_D_800BCEA8 0x800BCEA8u
#define GA_D_8009CE90 0x8009CE90u
#define GA_D_8009D1F4 0x8009D1F4u
#define REC_STRIDE    56u
#define SCAN_CAP      0x10000u
#define GLYPH_BYTES   28u
#define ARENA_BYTES   0x2400u /* func_8006A8D4's per-bank allocation */

typedef struct {
    pe_addr_t packet;
    pe_addr_t end;
    pe_addr_t ot;
    unsigned int control;
    int background;
} PeMessageDraw;

/* func_80077AC4's 24-bit AddPrim stores (682BC.s: 77AC4..77B00). */
static void pe_37870_addprim(pe_addr_t ot, pe_addr_t packet)
{
    PE_StoreU32(packet, (PE_LoadU32(packet) & 0xFF000000u) |
                        (PE_LoadU32(ot) & 0x00FFFFFFu));
    PE_StoreU32(ot, (PE_LoadU32(ot) & 0xFF000000u) | (packet & 0x00FFFFFFu));
}

static void pe_37870_glyph(PeMessageDraw *draw, uint8_t code,
                           uint16_t *x, uint16_t y, int space_adjust)
{
    pe_addr_t p = draw->packet;

    if (space_adjust && code == 0x0Fu && PE_LoadU8(0x8009CE94u) == 1u)
        *x = (uint16_t)(*x - 7u);
    *x = (uint16_t)(*x - PE_LoadU8(0x800916A0u + code * 2u));
    if (p <= draw->end && draw->end - p >= GLYPH_BYTES &&
        PE_RangeIsRam(p, GLYPH_BYTES) && PE_RangeIsRam(draw->ot + 4u, 4u)) {
        func_80077C84(p, 0u, 1u, PE_LoadU16(0x80091650u));
        func_80077C04(p + 8u);
        if (func_80077CB4(p, p + 8u) != 0)
            func_800719E4((uint32_t)-1);
        PE_StoreU16(p + 0x10u, *x);
        PE_StoreU16(p + 0x12u, y);
        PE_StoreU8(p + 0x14u, (uint8_t)((code % 21u) * 12u));
        PE_StoreU8(p + 0x15u, (uint8_t)((code / 21u) * 12u));
        PE_StoreU8(p + 0x0Cu, PE_LoadU8(0x8009CEA8u));
        PE_StoreU8(p + 0x0Du, PE_LoadU8(0x8009CEACu));
        PE_StoreU8(p + 0x0Eu, PE_LoadU8(0x8009CEB0u));
        PE_StoreU16(p + 0x16u, PE_LoadU16(0x80091652u));
        PE_StoreU16(p + 0x18u, 12u);
        PE_StoreU16(p + 0x1Au, 12u);
        pe_37870_addprim(draw->ot + 4u, p);
        draw->packet += GLYPH_BYTES;
    }
    *x = (uint16_t)(*x + PE_LoadU8(0x800916A1u + code * 2u));
}

/* Original37E6C/38338/38484/385C4: icons and extended glyph pages.
 * All four paths use a fixed12-pixel advance. */
static void pe_37870_extended(PeMessageDraw *draw,unsigned prefix,unsigned sub,uint16_t *x,uint16_t y)
{
    unsigned code,u,v;pe_addr_t font,p=draw->packet;
    if(prefix==0u) {u=sub*12u;v=0u;font=0x80091644u;}
    else if(prefix==0xFBu && sub<4u) {
        u=64u+sub*12u;v=PE_LoadU16(0x8009166Au);font=0x80091664u;
    } else {
        code=sub+(prefix==0xFCu?0x34u:prefix==0xFDu?0x134u:0xEDu);
        u=(code%21u)*12u;v=(code/21u)*12u;
        font=prefix==0xFBu?0x80091644u:0x80091654u;
        if(prefix==0xFBu && v>=241u){v-=252u;font+=16u;}
    }
    if(p<=draw->end && draw->end-p>=GLYPH_BYTES &&
       PE_RangeIsRam(p,GLYPH_BYTES) && PE_RangeIsRam(draw->ot+4u,4u)) {
        func_80077C84(p,0u,1u,PE_LoadU16(font+12u));func_80077C04(p+8u);
        if(func_80077CB4(p,p+8u)!=0)func_800719E4((uint32_t)-1);
        PE_StoreU16(p+16u,*x);PE_StoreU16(p+18u,y);
        PE_StoreU8(p+20u,(uint8_t)u);PE_StoreU8(p+21u,(uint8_t)v);
        PE_StoreU8(p+12u,PE_LoadU8(0x8009CEA8u));PE_StoreU8(p+13u,PE_LoadU8(0x8009CEACu));PE_StoreU8(p+14u,PE_LoadU8(0x8009CEB0u));
        PE_StoreU16(p+22u,PE_LoadU16(font+14u));PE_StoreU16(p+24u,12u);PE_StoreU16(p+26u,12u);
        pe_37870_addprim(draw->ot+4u,p);draw->packet+=GLYPH_BYTES;
    }
    *x=(uint16_t)(*x+12u);
}

static void pe_37870_background(PeMessageDraw *draw, unsigned int bank)
{
    if (!draw->background && PE_RangeIsRam(draw->ot + 8u, 4u)) {
        pe_37870_addprim(draw->ot + 8u, 0x8009ECA8u + bank * 24u);
        draw->background = 1;
    }
}

static int pe_37870_kseg(pe_addr_t p)
{
    return p >= 0x80000000u && p < 0x80200000u;
}

static pe_addr_t pe_37870_find_marker(pe_addr_t stream, int id)
{
    unsigned int n;
    uint8_t b0;
    uint8_t b1;
    uint8_t b2;

    if (!pe_37870_kseg(stream))
        return 0;
    for (n = 0; n < SCAN_CAP; n++) {
        if (!pe_37870_kseg(stream + 2u))
            return 0;
        b0 = PE_LoadU8(stream);
        if (b0 == 0xFFu || b0 == 0xF9u) {
            b1 = PE_LoadU8(stream + 1u);
            b2 = PE_LoadU8(stream + 2u);
            if (b1 == 0xFEu && (int)b2 == id)
                return stream + 3u;
        }
        stream++;
    }
    return 0;
}

static void pe_37870_record(pe_addr_t rec, PeMessageDraw *draw, unsigned int bank)
{
    uint8_t state;
    pe_addr_t cursor;
    pe_addr_t digits=rec+0x1Au;
    pe_addr_t stream;
    int id;
    uint32_t pad;
    uint32_t flags;
    unsigned int guard;
    uint8_t byte;
    uint8_t sub;
    uint8_t nn;
    uint8_t pause;
    int stop;
    uint16_t x = 20u, y = 174u, left = 20u;
    uint8_t mode;

    state = PE_LoadU8(rec);
    if (state == 0u)
        return;
    if (state == 1u) {
        PE_StoreU8(rec + 0x0Cu, 0u);
        PE_StoreU8(rec + 0x0Du, 0u);
        flags = PE_LoadU32(rec + 0x0Cu) & 0xFFF0FFFFu;
        PE_StoreU32(rec + 0x0Cu, flags);
        stream = PE_LoadU32(GA_D_8009CE90);
        PE_StoreU32(rec + 4u, stream);
        id = (int)(int16_t)PE_LoadU16(rec + 0x10u);
        cursor = pe_37870_find_marker(stream, id);
        if (cursor == 0u)
            return;
        PE_StoreU32(rec + 4u, cursor);
    }
    cursor = PE_LoadU32(rec + 4u);
    if (!pe_37870_kseg(cursor))
        return;
    PE_StoreU8(0x8009CEA8u, 128u);
    PE_StoreU8(0x8009CEACu, 128u);
    PE_StoreU8(0x8009CEB0u, 128u);
    mode = PE_LoadU8(rec + 8u);
    if (mode == 3u) {
        PE_StoreU8(0x8009CEA8u, PE_LoadU8(0x8009CEC4u));
        PE_StoreU8(0x8009CEACu, PE_LoadU8(0x8009CEC8u));
        PE_StoreU8(0x8009CEB0u, PE_LoadU8(0x8009CECCu));
    } else if (mode != 0u) {
        x = left = PE_LoadU16(rec + 0x12u);
        y = PE_LoadU16(rec + 0x14u);
        if (!(PE_LoadU32(rec + 0x0Cu) & 0x02000000u))
            y = (uint16_t)(y + 6u);
    } else if (PE_LoadU8(rec + 9u) != 0u) {
        pe_37870_background(draw, bank);
    }
    pad = PE_LoadU32(GA_D_8009D1F4);
    stop = 0;
    for (guard = 0; guard < SCAN_CAP && !stop; guard++) {
        if (!pe_37870_kseg(cursor))
            break;
        byte = PE_LoadU8(cursor);
        if (byte == 0xF9u) {
            PE_StoreU8(rec, 0u);
            stop = 1;
            break;
        }
        if (byte == 0xFFu) {
            state = PE_LoadU8(rec);
            if (state == 2u && (pad & 0x100u) != 0u &&
                (PE_LoadU32(rec + 0x0Cu) & 0x02000000u) == 0u)
                PE_StoreU8(rec, 0u);
            stop = 1;
            break;
        }
        if (byte == 0xF7u) {
            cursor++;
            x = left;
            y = (uint16_t)(y + 12u);
            continue;
        }
        if (byte == 0xF8u) {
            if (PE_LoadU8(rec) == 2u && (pad & 0x100u) != 0u) {
                cursor++; /* s3 already points to this page's F8. */
                PE_StoreU32(rec + 4u, cursor);
            }
            stop = 1;
            break;
        }
        if (byte == 0xFBu) {
            if (!pe_37870_kseg(cursor + 1u))
                break;
            sub = PE_LoadU8(cursor + 1u);
            if(sub<4u || sub>=10u) {pe_37870_extended(draw,byte,sub,&x,y);cursor+=2u;continue;}
            if(sub==8u) {
                for(int count=(int8_t)PE_LoadU8(digits+5u);count>0;count--)
                    pe_37870_extended(draw,0u,PE_LoadU8(digits+(unsigned)count-1u),&x,y);
                digits+=6u;cursor+=2u;continue;
            }
            if (sub == 6u) {
                cursor++; /* Original s3 points at the subcode here. */
                if(pad&0x100u) {
                    while(draw->control==((PE_LoadU32(rec+12u)>>16u)&15u)) {
                        cursor++;flags=PE_LoadU32(rec+12u);
                        PE_StoreU32(rec+12u,(flags&0xFFF0FFFFu)|(((draw->control+1u)&15u)<<16u));
                        stop=1;
                    }
                    if(PE_LoadU8(cursor)==6u)cursor++;
                } else if(draw->control==((PE_LoadU16(rec+14u))&15u))stop=1;
                else cursor++;
                draw->control=(draw->control+1u)&255u;
                continue;
            }
            if (sub == 7u) {
                cursor+=2u;
                nn=PE_LoadU8(cursor);pause=PE_LoadU8(rec+13u);
                PE_StoreU8(rec+12u,nn);
                if(pause<nn) {
                    if(draw->control==(PE_LoadU16(rec+14u)&15u)) {
                        PE_StoreU8(rec+13u,(uint8_t)(pause+1u));stop=1;
                    } else cursor++;
                } else {
                    while(draw->control==(PE_LoadU16(rec+14u)&15u)) {
                        PE_StoreU8(rec+13u,0u);PE_StoreU8(rec+12u,PE_LoadU8(cursor));
                        flags=PE_LoadU32(rec+12u);cursor++;
                        PE_StoreU32(rec+12u,(flags&0xFFF0FFFFu)|((((flags>>16u)+1u)&15u)<<16u));
                    }
                    if(PE_LoadU8(rec+13u))cursor++;
                }
                draw->control=(draw->control+1u)&255u;
                continue;
            }
            if (sub == 4u || sub == 5u) {
                PE_StoreU8(0x8009CEA8u, sub == 4u ? 255u : 128u);
                PE_StoreU8(0x8009CEACu, sub == 4u ? 0u : 128u);
                PE_StoreU8(0x8009CEB0u, sub == 4u ? 0u : 128u);
            }
            if (sub == 9u) {
                uint32_t fl;
                uint8_t count;
                int8_t cur;

                if (!pe_37870_kseg(cursor + 2u))
                    break;
                fl = PE_LoadU32(rec + 0x0Cu) | 0x00200000u;
                count = (uint8_t)(PE_LoadU8(cursor + 2u) & 7u);
                fl = (fl & 0xFE3FFFFFu) | ((uint32_t)count << 22);
                PE_StoreU32(rec + 0x0Cu, fl);
                cur = (int8_t)PE_LoadU8(0x8009CEA0u);
                if ((pad & 0x20u) != 0u) {
                    cur = (int8_t)(cur + 1);
                    if (cur >= (int8_t)count)
                        cur = (int8_t)(count - 1);
                    PE_StoreU8(0x8009CEA0u, (uint8_t)cur);
                }
                if ((pad & 0x08u) != 0u) {
                    cur = (int8_t)PE_LoadU8(0x8009CEA0u);
                    cur = (int8_t)(cur - 1);
                    if (cur < 0)
                        cur = 0;
                    PE_StoreU8(0x8009CEA0u, (uint8_t)cur);
                }
                if ((pad & 0x100u) != 0u)
                    PE_StoreU8(0x8009CEA4u, PE_LoadU8(0x8009CEA0u));
                /* Original382C8..38338 reuses the initialized cursor packet. */
                pe_addr_t choice=0x8009EC70u+bank*28u;
                PE_StoreU16(choice+16u,x);
                PE_StoreU16(choice+18u,(uint16_t)(y+(int8_t)PE_LoadU8(0x8009CEA0u)*12));
                pe_37870_addprim(PE_LoadU32(0x800B0E38u+bank*4u)+4u,choice);
                cursor += 3u;
                continue;
            }
            cursor += 2u;
            continue;
        }
        if (byte == 0xFCu || byte == 0xFDu) {
            if(!pe_37870_kseg(cursor+1u))break;
            pe_37870_extended(draw,byte,PE_LoadU8(cursor+1u),&x,y);
            cursor += 2u;
            continue;
        }
        if (byte == 0xFAu) {
            int n, count = (int8_t)PE_LoadU8(0x8009169Du);
            for (n = 0; n < count; n++)
                pe_37870_glyph(draw, PE_LoadU8(0x80091694u + (unsigned int)n),
                               &x, y, 0);
            cursor++;
            continue;
        }
        pe_37870_glyph(draw, byte, &x, y, 1);
        cursor++;
    }
    /* Retail redraws from the page anchor every frame. Only opening a
     * message or confirming F8 changes rec+4; FF retains the whole page. */
    if (stop && PE_LoadU8(rec) == 1u)
        PE_StoreU8(rec, 2u);
}

void func_80037870(void)
{
    unsigned int i;
    unsigned int bank = PE_LoadU32(0x8009CDDCu);
    PeMessageDraw draw;

    if (bank > 1u)
        return;
    draw.packet = PE_LoadU32(0x800B0E44u + bank * 4u);
    draw.end = PE_RangeIsRam(draw.packet, ARENA_BYTES) ?
        draw.packet + ARENA_BYTES : draw.packet;
    draw.ot = PE_LoadU32(0x800B0E38u + bank * 4u);
    draw.control = 0u;
    draw.background = 0;
    if (PE_LoadU32(0x8009CED4u) != 0u)
        pe_37870_background(&draw, bank);
    for (i = 0; i < 4u; i++)
        pe_37870_record(GA_D_800BCEA8 + i * REC_STRIDE, &draw, bank);
}
