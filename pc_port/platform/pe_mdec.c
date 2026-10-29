/* MDEC tables, software pixels and deferred DMA for the libpress path. */
#include "pe_mdec.h"

#include "pe_gpu.h"
#include "game_port.h"
#include "pe_bootstrap.h"
#include "pe_irq_delivery.h"

#include <string.h>
#include <stdlib.h>

#define MDEC_DMA_CHCR_INPUT 0x01000201u

static PeMdecState g_mdec;

static uint8_t g_quant[2][64];
static int16_t g_scale[64];
static unsigned g_tables;
static int g_decode_valid,g_input_pending;
static uint32_t g_output_bytes;
static int g_dma1_port;
static uint32_t g_color_dma_pos; /* address phase; manual FIFO reads do not advance it */
static pe_addr_t g_dma0_source; /* guest alias, separate from 24-bit MADR */
static uint32_t g_dma0_port_bytes; /* nonzero: raw parameter-port DMA */
static uint16_t g_input[131070];
static uint32_t g_input_pos,g_input_count,g_decode_command,g_macroblocks;
typedef struct {
    uint8_t pixels[768]; /* DMA row-major storage; raw port reorders on read */
    uint32_t pos,count;
    unsigned depth,order; /* 0 unread, 1 row-major helper, 2 CPU tiles, 3 DMA tiles */
} MdecOutput;
static MdecOutput g_current;
/* Output of a previous command still ahead of the current one in the
 * data-out FIFO (see MdecBeginCommand). */
#define MDEC_STALE_OUTPUT_BYTES (64u*768u)
typedef struct MdecQueuedOutput {
    MdecOutput output;
    struct MdecQueuedOutput *next;
} MdecQueuedOutput;
static MdecQueuedOutput *g_stale_head,*g_stale_tail;
static uint32_t g_stale_bytes;
static int g_drain;
/* CPU command-port input, separate from queued DMA register ownership. */
static uint32_t g_cpu_command,g_cpu_words,g_cpu_received;
static uint8_t g_cpu_table[128];
static int MdecPrepareCommand(uint32_t command);
static int MdecReadOutputByte(unsigned order,uint8_t *value);

static int MdecBoundary(const char *name,uint32_t value)
{
    Bootstrap_ReturnVoid1(name,"MDEC_decode",value);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
}
/* Defined floor division avoids implementation-defined signed right shifts. */
static int32_t MdecShift(int64_t value,unsigned bits)
{
    int64_t divisor=INT64_C(1)<<bits;
    return (int32_t)(value>=0?value/divisor:-((-value+divisor-1)/divisor));
}
static int32_t MdecSigned(unsigned value,unsigned bits)
{
    unsigned mask=(1u<<bits)-1u;value&=mask;
    return value&(1u<<(bits-1u))?(int32_t)value-(int32_t)(1u<<bits):(int32_t)value;
}
static void MdecClearOutputQueue(void)
{
    while(g_stale_head) {
        MdecQueuedOutput *next=g_stale_head->next;
        free(g_stale_head);g_stale_head=next;
    }
    g_stale_tail=NULL;g_stale_bytes=0;
}
static int32_t MdecClamp(int32_t value,int32_t low,int32_t high)
{
    return value<low?low:value>high?high:value;
}
static uint32_t MdecAvailableInput(void)
{
    return g_cpu_words && (g_cpu_command>>29u)==1u
        ?g_cpu_received*2u:g_input_count;
}
/* Inspect only received halfwords, without consuming a partial block. This
 * mirrors the RLE framing (including coefficient 63 without an EOB), not
 * the IDCT. A command may contain many macroblocks: its final parameter is
 * not a prerequisite for output from an earlier complete macroblock. */
static int MdecScanMacroblock(uint32_t *position,uint32_t available)
{
    uint32_t pos=*position;
    unsigned count=((g_decode_command>>27u)&3u)<2u?1u:6u;
    for(unsigned block=0;block<count;block++) {
        while(pos<available && g_input[pos]==0xFE00u) pos++;
        if(pos==available) return 0;
        pos++; /* DC */
        unsigned coefficient=0;
        while(coefficient<63u) {
            if(pos==available) return 0;
            coefficient+=(g_input[pos++]>>10u)+1u;
        }
    }
    *position=pos;return 1;
}
static void MdecLoadTables(uint32_t command,const uint8_t *payload,uint32_t bytes)
{
    unsigned kind=command>>29u;
    if(kind==2u && bytes>=64u) {
        memcpy(g_quant[0],payload,64u);g_tables|=1u;
        if((command&1u) && bytes>=128u) {
            memcpy(g_quant[1],payload+64u,64u);g_tables|=2u;
        }
    } else if(kind==3u && bytes>=128u) {
        for(unsigned i=0;i<64u;i++)
            g_scale[i]=(int16_t)((unsigned)payload[i*2u]|((unsigned)payload[i*2u+1u]<<8u));
        g_tables|=4u;
    }
}
static int MdecAcceptWord(uint32_t value)
{
    if(!g_cpu_words) {
        unsigned kind=value>>29u;
        /* Classes 0 and 4..7 have no operation or payload. Retain the
         * command latch (including its status-format/count fields), but
         * do not disturb older decode input, output or loaded tables. */
        if(kind<1u || kind>3u) {
            g_cpu_command=value;g_mdec.command_last_write=value;return 1;
        }
        if(kind==1u && !MdecPrepareCommand(value)) return 0;
        g_cpu_command=value;g_mdec.command_last_write=value;
        g_cpu_words=kind==1u?(value&0xFFFFu):(kind==2u && !(value&1u)?16u:32u);
        g_cpu_received=0;
        if(kind==1u && g_cpu_words) g_decode_valid=0;
    } else {
        unsigned kind=g_cpu_command>>29u;
        if(kind==1u) {
            g_input[g_cpu_received*2u]=(uint16_t)value;
            g_input[g_cpu_received*2u+1u]=(uint16_t)(value>>16u);
        } else {
            unsigned offset=g_cpu_received*4u;
            for(unsigned i=0;i<4u;i++) g_cpu_table[offset+i]=(uint8_t)(value>>(i*8u));
        }
        if(++g_cpu_received==g_cpu_words) {
            if(kind==1u) g_decode_valid=1;
            else MdecLoadTables(g_cpu_command,g_cpu_table,g_cpu_words*4u);
            g_cpu_words=g_cpu_received=0;
        }
    }
    return 1;
}
int PE_MDEC_WriteCommandData(uint32_t value)
{
    if(g_mdec.dma0_active) return MdecBoundary("MDEC_CPU_DMA_input_overlap",value);
    return MdecAcceptWord(value);
}
uint32_t PE_MDEC_ReadDMA0(uint32_t offset)
{
    if(offset==0u) return g_mdec.dma0_madr;
    if(offset==4u) return g_mdec.dma0_bcr;
    if(offset==8u) return g_mdec.dma0_chcr;
    return (uint32_t)MdecBoundary("MDEC_DMA0_register",offset);
}
void PE_MDEC_WriteDMA0(uint32_t offset,uint32_t value)
{
    if(g_mdec.dma0_active) {
        if(offset==8u && !(value&0x01000000u)) {
            g_mdec.dma0_chcr=value;g_mdec.dma0_active=0;
            g_dma0_port_bytes=0;g_input_pending=0;return;
        }
        if(offset==8u && value==g_mdec.dma0_chcr) return;
        (void)MdecBoundary("MDEC_DMA0_busy_register",offset);return;
    }
    if(offset==0u) {g_mdec.dma0_madr=value&0xFFFFFFu;return;}
    if(offset==4u) {g_mdec.dma0_bcr=value;return;}
    if(offset!=8u) {(void)MdecBoundary("MDEC_DMA0_register",offset);return;}
    if(!(value&0x01000000u)) {g_mdec.dma0_chcr=value;return;}
    if((value&~0x10000002u)!=MDEC_DMA_CHCR_INPUT) {
        (void)MdecBoundary("MDEC_DMA0_control",value);return;
    }
    uint32_t bs=g_mdec.dma0_bcr&0xFFFFu,blocks=g_mdec.dma0_bcr>>16u;
    if(!bs || bs>32u) {(void)MdecBoundary("MDEC_DMA0_block_size",bs);return;}
    uint32_t bytes=bs*(blocks?blocks:65536u)*4u;
    uint32_t ram_offset=g_mdec.dma0_madr&0xFFFFFCu;
    pe_addr_t source=PE_RAM_BASE+ram_offset;
    int decrement=(value&2u)!=0;
    if((decrement && (ram_offset>=PE_RAM_SIZE || bytes-4u>ram_offset)) ||
       (!decrement && !PE_RangeIsRam(source,bytes))) {
        (void)MdecBoundary("MDEC_DMA0_input_range",g_mdec.dma0_madr);return;
    }
    g_dma0_source=source;g_dma0_port_bytes=bytes;g_input_pending=0;
    g_mdec.dma0_chcr=value;g_mdec.dma0_active=1;
}
uint32_t PE_MDEC_ReadDMA1(uint32_t offset)
{
    if(offset==0u) return g_mdec.dma1_madr;
    if(offset==4u) return g_mdec.dma1_bcr;
    if(offset==8u) return g_mdec.dma1_chcr;
    return (uint32_t)MdecBoundary("MDEC_DMA1_register",offset);
}
void PE_MDEC_WriteDMA1(uint32_t offset,uint32_t value)
{
    if(g_mdec.dma1_chcr&0x01000000u) {
        if(offset==8u && !(value&0x01000000u)) {
            g_mdec.dma1_chcr=value;g_output_bytes=0;g_dma1_port=0;return;
        }
        if(offset==8u && value==g_mdec.dma1_chcr) return;
        (void)MdecBoundary("MDEC_DMA1_busy_register",offset);return;
    }
    if(offset==0u) {g_mdec.dma1_madr=value&0xFFFFFFu;return;}
    if(offset==4u) {g_mdec.dma1_bcr=value;return;}
    if(offset!=8u) {(void)MdecBoundary("MDEC_DMA1_register",offset);return;}
    if(!(value&0x01000000u)) {g_mdec.dma1_chcr=value;return;}
    if((value&~0x10000002u)!=0x01000200u) {(void)MdecBoundary("MDEC_DMA1_control",value);return;}
    uint32_t bs=g_mdec.dma1_bcr&0xFFFFu,blocks=g_mdec.dma1_bcr>>16u;
    if(!bs || bs>32u) {(void)MdecBoundary("MDEC_DMA1_block_size",bs);return;}
    uint32_t bytes=bs*(blocks?blocks:65536u)*4u;
    uint32_t ram_offset=g_mdec.dma1_madr&0xFFFFFCu;
    pe_addr_t destination=PE_RAM_BASE+ram_offset;
    if(((value&2u) && (ram_offset>=PE_RAM_SIZE || bytes-4u>ram_offset)) ||
       (!(value&2u) && !PE_RangeIsRam(destination,bytes))) {
        (void)MdecBoundary("MDEC_DMA1_output_range",g_mdec.dma1_madr);return;
    }
    g_output_bytes=bytes;g_dma1_port=1;
    g_mdec.dma1_chcr=value;g_mdec.output_count++;
}
static void MdecIDCT(int32_t block[64])
{
    int32_t temporary[64];
    for(unsigned pass=0;pass<2u;pass++) {
        for(unsigned y=0;y<8u;y++) for(unsigned x=0;x<8u;x++) {
            int64_t sum=0;
            for(unsigned z=0;z<8u;z++) sum+=(int64_t)block[y+z*8u]*MdecShift(g_scale[x+z*8u],3u);
            temporary[x+y*8u]=MdecShift(sum+4095,13u);
        }
        memcpy(block,temporary,sizeof(temporary));
    }
    /* The IDCT stage exposes signed eight-bit samples after its nine-bit
     * accumulator wrap. Chroma must be clamped before RGB conversion too. */
    for(unsigned i=0;i<64u;i++)
        block[i]=MdecClamp(MdecSigned((unsigned)block[i],9u),-128,127);
}
static int MdecBlock(int32_t block[64],const uint8_t quant[64])
{
    /* Zigzag traversal generated by diagonals, with the first AC at (1,0). */
    unsigned scan[64],index=0;
    for(unsigned diagonal=0;diagonal<15u;diagonal++) {
        for(unsigned n=0;n<8u;n++) {
            unsigned y=(diagonal&1u)?n:7u-n;
            if(diagonal>=y && diagonal-y<8u) scan[index++]=y*8u+diagonal-y;
        }
    }
    memset(block,0,64u*sizeof(*block));
    while(g_input_pos<g_input_count && g_input[g_input_pos]==0xFE00u) g_input_pos++;
    if(g_input_pos==g_input_count) return g_drain?0:MdecBoundary("MDEC_missing_block",g_input_pos);
    uint16_t code=g_input[g_input_pos++];unsigned scale=code>>10u,k=0;
    for(;;) {
        int32_t value=MdecSigned(code,10u);
        if(!scale) value*=2;
        else if(!k) value*=quant[0];
        else value=MdecShift((int64_t)value*quant[k]*scale+4,3u);
        block[scale?scan[k]:k]=MdecClamp(value,-1024,1023);
        /* A fully specified block needs no EOB. The next halfword may
         * already be the next block's DC, so leave it for that decoder. */
        if(k==63u) break;
        if(g_input_pos==g_input_count) return g_drain?0:MdecBoundary("MDEC_unterminated_block",g_input_pos);
        code=g_input[g_input_pos++];k+=(code>>10u)+1u;
        if(k>=64u) break;
    }
    MdecIDCT(block);return 1;
}
static uint8_t MdecComponent(int32_t value)
{
    /* RGB sums wrap in the signed nine-bit intermediate before saturation;
     * clamping an unwrapped sum reverses extreme chroma/luma output. */
    int32_t clamped=MdecClamp(MdecSigned((unsigned)value,9u),-128,127);
    return (uint8_t)clamped^((g_decode_command&0x04000000u)?0u:0x80u);
}
static int MdecMacroblock(void)
{
    unsigned depth=(g_decode_command>>27u)&3u;
    int32_t blocks[6][64];
    unsigned count=depth<2u?1u:6u;
    for(unsigned i=0;i<count;i++) if(!MdecBlock(blocks[i],g_quant[count==6u && i<2u])) return 0;
    g_current.pos=0;g_current.count=0;g_current.depth=depth;g_current.order=0;
    if(depth<2u) {
        for(unsigned i=0;i<64u;i++) {
            uint8_t value=MdecComponent(blocks[0][i]);
            if(depth) g_current.pixels[g_current.count++]=value;
            else if(!(i&1u)) g_current.pixels[g_current.count]=value>>4u;
            else g_current.pixels[g_current.count++]|=(uint8_t)(value&0xF0u);
        }
    } else {
        for(unsigned y=0;y<16u;y++) for(unsigned x=0;x<16u;x++) {
            unsigned uv=(y/2u)*8u+x/2u,yi=2u+(y/8u)*2u+x/8u;
            int32_t lum=blocks[yi][(y&7u)*8u+(x&7u)],cr=blocks[0][uv],cb=blocks[1][uv];
            uint8_t red=MdecComponent(lum+MdecShift(359*cr+128,8u));
            /* Green's two products lose different low fractional bits
             * before addition. Use floor division for negative products. */
            int32_t green_cb=MdecShift(-88*cb,5u)*32;
            int32_t green_cr=MdecShift(-183*cr,3u)*8;
            uint8_t green=MdecComponent(lum+MdecShift(green_cb+green_cr+128,8u));
            uint8_t blue=MdecComponent(lum+MdecShift(454*cb+128,8u));
            if(depth==2u) {g_current.pixels[g_current.count++]=red;g_current.pixels[g_current.count++]=green;g_current.pixels[g_current.count++]=blue;}
            else {
                unsigned r=(red+4u)>>3u,g=(green+4u)>>3u,b=(blue+4u)>>3u;
                uint16_t pixel=(uint16_t)((r>31u?31u:r)|((g>31u?31u:g)<<5u)|((b>31u?31u:b)<<10u)|((g_decode_command&0x02000000u)?0x8000u:0u));
                g_current.pixels[g_current.count++]=(uint8_t)pixel;g_current.pixels[g_current.count++]=(uint8_t)(pixel>>8u);
            }
        }
    }
    g_macroblocks++;return 1;
}
static int MdecPrepareCommand(uint32_t command)
{
    uint32_t words=command&0xFFFFu;
    if(command>>29u!=1u) return MdecBoundary("MDEC_decode_command",command);
    unsigned required_tables=((command>>27u)&3u)<2u?5u:7u;
    if((g_tables&required_tables)!=required_tables) return MdecBoundary("MDEC_missing_tables",g_tables);
    /* A new decode command while the previous one still holds input
     * (psx-spx MDEC: a command owns exactly its N parameter words; FE00h
     * at a block start or after EOB is ignored, anywhere else it is EOB).
     * The hardware decodes those words in order, so their macroblocks are
     * queued in the data-out FIFO ahead of the new command's output and
     * the next DMA1 reads them first.  Model that: finish the old
     * command's complete macroblocks into the output queue (after any output not
     * yet read) and serve it before the new command's pixels.  A trailing
     * partial macroblock yields no output (only complete macroblocks
     * reach the FIFO); psx-spx does not document it further.  The
     * command is only parsed here, lazily at DMA0 service, so an
     * already-queued DMA1 request belongs to the new command.  Not
     * reached by the Day-1 movies (FMV001-FMV004: 0 occurrences). */
    if(g_input_pos<g_input_count || g_current.pos<g_current.count) {
        for(;;) {
            uint32_t n=g_current.count-g_current.pos;
            if(n) {
                if(n>MDEC_STALE_OUTPUT_BYTES-g_stale_bytes)
                    return MdecBoundary("MDEC_stale_output_overflow",g_stale_bytes);
                MdecQueuedOutput *saved=malloc(sizeof(*saved));
                if(!saved) return MdecBoundary("MDEC_output_allocation",n);
                saved->output=g_current;saved->next=NULL;
                if(g_stale_tail) g_stale_tail->next=saved;
                else g_stale_head=saved;
                g_stale_tail=saved;g_stale_bytes+=n;
                g_current.pos=g_current.count;
                g_mdec.stale_output_bytes+=n;
            }
            while(g_input_pos<g_input_count && g_input[g_input_pos]==0xFE00u) g_input_pos++;
            if(g_input_pos==g_input_count) break;
            g_drain=1;
            int ok=MdecMacroblock();
            g_drain=0;
            if(!ok) {g_current.pos=g_current.count=0;break;}
        }
        g_input_pos=g_input_count;
        g_mdec.stale_input_discards++;
    }
    g_decode_command=command;g_input_count=words*2u;g_input_pos=0;g_current.pos=g_current.count=0;g_macroblocks=0;g_decode_valid=1;
    g_color_dma_pos=0;
    return 1;
}
static int MdecBeginCommand(uint32_t command,pe_addr_t source)
{
    uint32_t bytes=(command&0xFFFFu)*4u;
    if(!PE_RangeIsRam(PE_RamCanonical(source,bytes),bytes)) return MdecBoundary("MDEC_input_range",source);
    if(!MdecPrepareCommand(command)) return 0;
    for(unsigned i=0;i<g_input_count;i++) g_input[i]=PE_LoadU16(source+i*2u);
    return 1;
}
int PE_MDEC_BeginDecode(pe_addr_t command_block)
{
    if(g_cpu_words) return MdecBoundary("MDEC_CPU_input_pending",g_cpu_words-g_cpu_received);
    if(!PE_RangeIsRam(PE_RamCanonical(command_block,4u),4u)) return MdecBoundary("MDEC_command_range",command_block);
    return MdecBeginCommand(PE_LoadU32(command_block),command_block+4u);
}
int PE_MDEC_HasDecode(void)
{
    return g_decode_valid || g_input_pending || (g_cpu_words && (g_cpu_command>>29u)==1u);
}
static int MdecDMA1OutputShape(uint32_t bytes)
{
    /* The existing pixel helper supplies complete row-major color blocks.
     * Until DMA1's partial-block address counter is modeled, admit complete
     * color macroblocks only, before any destination bytes are written. */
    MdecQueuedOutput *queued=g_stale_head;
    for(;;) {
        const MdecOutput *out=queued?&queued->output:&g_current;
        uint32_t available=out->count-out->pos;
        if(bytes && available) {
            if(out->depth>=2u && (out->pos || out->order==2u || bytes<available))
                return MdecBoundary("MDEC_DMA1_color_block_layout",bytes);
            bytes-=bytes<available?bytes:available;
        }
        if(!queued || !bytes) break;
        queued=queued->next;
    }
    unsigned depth=(g_decode_command>>27u)&3u;
    if(bytes && depth>=2u && bytes%(depth==2u?768u:512u))
        return MdecBoundary("MDEC_DMA1_color_block_layout",bytes);
    return 1;
}
/* Monochrome output has no DMA1 color-address swizzle. Keep the existing
 * whole-macroblock color/mixed-format service until that counter is modeled. */
static int MdecDMA1Monochrome(uint32_t bytes)
{
    for(MdecQueuedOutput *q=g_stale_head;q;q=q->next) {
        uint32_t available=q->output.count-q->output.pos;
        if(available && q->output.depth>=2u) return 0;
        if(bytes<=available) return 1;
        bytes-=available;
    }
    uint32_t available=g_current.count-g_current.pos;
    if(available && g_current.depth>=2u) return 0;
    if(bytes<=available) return 1;
    return ((g_decode_command>>27u)&3u)<2u;
}
/* Convert a byte in four consecutive 8x8 color tiles to its position
 * in the 16x16 bitmap. Both color row widths are multiples of four, so
 * one raw data word always maps to four consecutive destination bytes. */
static unsigned MdecColorByteIndex(unsigned position,unsigned depth)
{
    unsigned bpp=depth==2u?3u:2u;
    unsigned pixel=position/bpp,tile=pixel/64u,within=pixel%64u;
    unsigned x=(tile%2u)*8u+within%8u,y=(tile/2u)*8u+within/8u;
    return (y*16u+x)*bpp+position%bpp;
}
static int MdecDMA1Color(void)
{
    unsigned depth=(g_decode_command>>27u)&3u;
    /* Mixed/queued formats retain the older atomic service contract.
     * A pure color stream has one stable layout for each request block. */
    return !g_stale_head && depth>=2u &&
        (g_current.pos==g_current.count || g_current.depth==depth);
}
static int MdecDMA1ColorBlock(pe_addr_t destination,uint32_t bytes)
{
    unsigned depth=(g_decode_command>>27u)&3u;
    unsigned size=depth==2u?768u:512u;
    int decrement=(g_mdec.dma1_chcr&2u)!=0u;
    unsigned start=g_color_dma_pos;
    pe_addr_t addresses[32]; /* admitted raw request block is at most 32 words */
    if(g_current.pos<g_current.count && g_current.order==1u)
        return MdecBoundary("MDEC_mixed_output_order",g_current.pos);
    /* The sequential DMA address advances normally; the device supplies
     * the tile-to-row displacement. Admit all displaced addresses before
     * consuming even the first output word, including at a RAM edge. */
    for(unsigned i=0;i<bytes;i+=4u) {
        unsigned position=(start+i)%size;
        int32_t displacement=(int32_t)MdecColorByteIndex(position,depth)-(int32_t)position;
        int64_t step=decrement?-(int64_t)i:(int64_t)i;
        int64_t address=(int64_t)destination+step+displacement;
        if(address<PE_RAM_BASE || address>(int64_t)PE_RAM_BASE+PE_RAM_SIZE-4u)
            return MdecBoundary("MDEC_DMA1_color_output_range",(uint32_t)address);
        addresses[i/4u]=(pe_addr_t)address;
    }
    g_mdec.dma1_chcr&=~0x10000000u; /* first admitted block begins now */
    for(unsigned i=0;i<bytes;i+=4u) {
        uint32_t word=0;
        for(unsigned j=0;j<4u;j++) {
            uint8_t byte;
            if(!MdecReadOutputByte(3u,&byte)) return 0;
            word|=(uint32_t)byte<<(8u*j);
        }
        PE_StoreU32(addresses[i/4u],word);
        g_color_dma_pos=(g_color_dma_pos+4u)%size;
    }
    return 1;
}
static int MdecOutputAvailable(uint32_t bytes)
{
    /* DMA waits on empty output; do not consume a prefix or write RAM until
     * this atomic service transfer can be satisfied. Count buffered output
     * first, then complete RLE macroblocks in received input only. */
    for(MdecQueuedOutput *q=g_stale_head;q;q=q->next) {
        uint32_t available=q->output.count-q->output.pos;
        if(bytes<=available) return 1;
        bytes-=available;
    }
    uint32_t available=g_current.count-g_current.pos;
    if(bytes<=available) return 1;
    bytes-=available;
    uint32_t pos=g_input_pos;
    const uint32_t sizes[]={32u,64u,768u,512u};
    uint32_t size=sizes[(g_decode_command>>27u)&3u];
    while(MdecScanMacroblock(&pos,MdecAvailableInput())) {
        if(bytes<=size) return 1;
        bytes-=size;
    }
    return 0;
}
/* A refused public read must not consume a buffered prefix or partly
 * overwrite its destination. Inspect queued/current consumer order and
 * complete received macroblocks without running the decoder first. */
static int MdecAdmitOutputRead(unsigned order,uint32_t bytes)
{
    uint32_t remaining=bytes;
    MdecQueuedOutput *queued=g_stale_head;
    for(;;) {
        const MdecOutput *out=queued?&queued->output:&g_current;
        uint32_t available=out->count-out->pos;
        if(remaining && available) {
            if(out->depth>=2u && out->order && out->order!=order &&
               (out->order==1u || order==1u))
                return MdecBoundary("MDEC_mixed_output_order",out->pos);
            remaining-=remaining<available?remaining:available;
        }
        if(!queued || !remaining) break;
        queued=queued->next;
    }
    if(remaining && !MdecOutputAvailable(bytes))
        return MdecBoundary("MDEC_output_short",bytes);
    return 1;
}
int PE_MDEC_Service(void)
{
    int completed=0,input_progress=0,output_progress=0;
    if(PE_Port_ShouldStop()) return 0;
    if(g_mdec.dma0_active &&
       ((g_mdec.control_last_write&0x40000000u) || (g_mdec.dma0_chcr&0x10000000u)) &&
       (PE_GPU_ReadDPCR()&8u)) {
        int raw=g_dma0_port_bytes!=0;
        int decrement=raw && (g_mdec.dma0_chcr&2u);
        uint32_t transferred=raw?(g_mdec.dma0_bcr&0xFFFFu)*4u:(g_mdec.dma0_bcr>>16u)*128u;
        if(raw) {
            /* Request DMA grants one input block per opportunity. The
             * receiver/output side may progress before later blocks arrive. */
            g_mdec.dma0_chcr&=~0x10000000u; /* force-start applies to this block only */
            for(uint32_t offset=0;offset<transferred;offset+=4u) {
                pe_addr_t address=decrement?g_dma0_source-offset:g_dma0_source+offset;
                if(!MdecAcceptWord(PE_LoadU32(address))) return 0;
            }
            g_dma0_source+=decrement?0u-transferred:transferred;
            g_dma0_port_bytes-=transferred;
            uint32_t blocks=g_mdec.dma0_bcr>>16u;
            if(!blocks) blocks=65536u;
            g_mdec.dma0_bcr=((blocks-1u)<<16u)|(g_mdec.dma0_bcr&0xFFFFu);
            input_progress=1;
            if(PE_GPU_ReadDICR()&1u) (void)PE_GPU_LatchDMACompletionFlag(0u);
        } else if(g_input_pending) {
            uint32_t available=(g_mdec.dma0_bcr>>16u)*32u;
            if((g_mdec.command_last_write&0xFFFFu)>available) return MdecBoundary("MDEC_input_short",available);
            if(!MdecBeginCommand(g_mdec.command_last_write,g_dma0_source)) return 0;
        } else {
            /* Table writes, like compressed input, belong to the enabled
             * DMA transfer, not the call which merely queues it. */
            uint32_t bytes=(g_mdec.dma0_bcr>>16u)*128u;
            const uint8_t *payload=PE_TranslateConst(g_dma0_source,bytes);
            MdecLoadTables(g_mdec.command_last_write,payload,bytes);
        }
        g_mdec.dma0_madr=(g_mdec.dma0_madr+(decrement?0u-transferred:transferred))&0x00FFFFFCu;
        if(!raw || !g_dma0_port_bytes) {
            g_dma0_port_bytes=0;g_input_pending=0;g_mdec.dma0_active=0;g_mdec.completed_upload_count++;
            g_mdec.dma0_bcr&=0xFFFFu;g_mdec.dma0_chcr&=~0x01000000u;
            (void)PE_GPU_LatchDMACompletionFlag(0u);completed=1;
        }
    }
    if((g_mdec.dma1_chcr&0x01000000u) && !g_input_pending &&
       ((g_mdec.control_last_write&0x20000000u) || (g_mdec.dma1_chcr&0x10000000u)) &&
       (PE_GPU_ReadDPCR()&0x80u)) {
        pe_addr_t destination=PE_RAM_BASE+(g_mdec.dma1_madr&0x00FFFFFCu);
        int color_block=g_dma1_port && MdecDMA1Color();
        if(g_dma1_port && !color_block && !MdecDMA1OutputShape(g_output_bytes)) return 0;
        int blockwise_output=g_dma1_port && (color_block || MdecDMA1Monochrome(g_output_bytes));
        int decrement=g_dma1_port && (g_mdec.dma1_chcr&2u);
        int forced=g_dma1_port && (g_mdec.dma1_chcr&0x10000000u);
        /* Queued/mixed color still uses whole-macroblock helpers, which
         * cannot stand in for a forced or reverse request block. */
        if(forced && !blockwise_output)
            return MdecBoundary("MDEC_DMA1_force_color_layout",g_output_bytes);
        if(decrement && !blockwise_output)
            return MdecBoundary("MDEC_DMA1_decrement_color_layout",g_output_bytes);
        uint32_t transferred=blockwise_output?(g_mdec.dma1_bcr&0xFFFFu)*4u:g_output_bytes;
        if(!MdecOutputAvailable(transferred)) {
            /* Empty forced reads have no established value in this model.
             * Refuse before consuming a prefix or writing any destination. */
            if(forced) return MdecBoundary("MDEC_DMA1_force_output_empty",transferred);
            if(completed || input_progress) (void)PE_IRQ_BridgeDICRRisingEdge(PE_IRQ_Generation());
            return completed;
        }
        /* Clear only when an admitted first block starts, not while DPCR
         * is disabled. Later blocks require the live output request gate. */
        if(color_block) {
            if(!MdecDMA1ColorBlock(destination,transferred)) return 0;
        } else if(decrement) {
            g_mdec.dma1_chcr&=~0x10000000u;
            /* Mono has identical CPU/DMA FIFO order. Step the destination
             * by words, preserving the little-endian byte order within each. */
            for(uint32_t offset=0;offset<transferred;offset+=4u) {
                uint32_t word;
                if(!PE_MDEC_ReadData(&word)) return 0;
                PE_StoreU32(destination-offset,word);
            }
        } else {
            g_mdec.dma1_chcr&=~0x10000000u;
            if(!PE_MDEC_ReadPixels(destination,transferred)) return 0;
        }
        g_mdec.dma1_madr=(g_mdec.dma1_madr+(decrement?0u-transferred:transferred))&0xFFFFFCu;
        g_output_bytes-=transferred;
        if(blockwise_output) {
            uint32_t blocks=g_mdec.dma1_bcr>>16u;
            if(!blocks) blocks=65536u;
            g_mdec.dma1_bcr=((blocks-1u)<<16u)|(g_mdec.dma1_bcr&0xFFFFu);
            output_progress=1;
            if(PE_GPU_ReadDICR()&2u) (void)PE_GPU_LatchDMACompletionFlag(1u);
        }
        if(!g_output_bytes) {
            g_mdec.dma1_bcr&=0xFFFFu;g_mdec.dma1_chcr&=~0x01000000u;
            g_mdec.completed_output_count++;g_dma1_port=0;
            (void)PE_GPU_LatchDMACompletionFlag(1u);completed=1;
        }
    }
    if(completed || input_progress || output_progress) (void)PE_IRQ_BridgeDICRRisingEdge(PE_IRQ_Generation());
    return completed;
}
int PE_MDEC_SubmitOutput(pe_addr_t destination,uint32_t words)
{
    uint32_t blocks=words>>5u;
    if(!blocks || blocks>0xFFFFu)
        return MdecBoundary("MDEC_output_block_count",words);
    uint32_t bytes=blocks*128u;
    pe_addr_t aligned=destination&~3u;
    if(!PE_RangeIsRam(PE_RamCanonical(aligned,bytes),bytes))
        return MdecBoundary("MDEC_output_table_range",destination);
    if(g_mdec.dma1_chcr&0x01000000u) {
        (void)PE_MDEC_Service();
        if(PE_Port_ShouldStop()) return 0;
        if(g_mdec.dma1_chcr&0x01000000u) return MdecBoundary("MDEC_output_wait",g_mdec.dma1_chcr);
    }
    PE_GPU_WriteDPCR(PE_GPU_ReadDPCR()|0x88u);
    g_dma1_port=0;
    g_mdec.dma1_chcr=0;
    g_mdec.dma1_madr=destination&0xFFFFFFu;
    g_mdec.dma1_bcr=(blocks<<16u)|0x20u;
    g_output_bytes=bytes;
    g_mdec.dma1_chcr=0x01000200u;g_mdec.output_count++;
    return 1;
}
static int MdecReadOutputByte(unsigned order,uint8_t *value)
{
    MdecOutput *out;
    int queued=g_stale_head!=NULL;
    if(queued) out=&g_stale_head->output;
    else {
        if(g_current.pos==g_current.count) {
            if(g_cpu_words && (g_cpu_command>>29u)==1u) {
                uint32_t pos=g_input_pos;
                if(!MdecScanMacroblock(&pos,MdecAvailableInput()))
                    return MdecBoundary("MDEC_CPU_decode_incomplete",g_cpu_words-g_cpu_received);
            }
            if(!MdecMacroblock()) return 0;
        }
        out=&g_current;
    }
    /* CPU and raw DMA consume the same tile FIFO. Only raw DMA advances
     * its separate address phase. The row-major helper is still distinct. */
    if(out->depth>=2u && out->order && out->order!=order &&
       (out->order==1u || order==1u))
        return MdecBoundary("MDEC_mixed_output_order",out->pos);
    out->order=order;
    unsigned index=out->pos;
    if(order>=2u && out->depth>=2u)
        index=MdecColorByteIndex(index,out->depth);
    *value=out->pixels[index];out->pos++;
    if(queued) {
        g_stale_bytes--;
        if(out->pos==out->count) {
            MdecQueuedOutput *done=g_stale_head;
            g_stale_head=done->next;
            if(!g_stale_head) g_stale_tail=NULL;
            free(done);
        }
    }
    /* Ignore command-end padding after the last complete output block. */
    if(!queued && out->pos==out->count)
        while(g_input_pos<MdecAvailableInput() && g_input[g_input_pos]==0xFE00u) g_input_pos++;
    return 1;
}
int PE_MDEC_ReadData(uint32_t *value)
{
    if(!MdecAdmitOutputRead(2u,4u)) return 0;
    uint32_t word=0;
    for(unsigned i=0;i<4u;i++) {
        uint8_t byte;
        if(!MdecReadOutputByte(2u,&byte)) return 0;
        word|=(uint32_t)byte<<(i*8u);
    }
    *value=word;return 1;
}
int PE_MDEC_ReadPixels(pe_addr_t destination,uint32_t bytes)
{
    if(!PE_RangeIsRam(PE_RamCanonical(destination,bytes),bytes)) return MdecBoundary("MDEC_output_range",destination);
    if(!MdecAdmitOutputRead(1u,bytes)) return 0;
    for(unsigned i=0;i<bytes;i++) {
        uint8_t byte;
        if(!MdecReadOutputByte(1u,&byte)) return 0;
        PE_StoreU8(destination+i,byte);
    }
    return 1;
}
uint32_t PE_MDEC_DecodedMacroblocks(void) { return g_macroblocks; }

static uint64_t Fnv1a64(const uint8_t *data, uint32_t size)
{
    uint64_t value = UINT64_C(0xCBF29CE484222325);
    uint32_t i;

    for (i = 0u; i < size; i++) {
        value ^= data[i];
        value *= UINT64_C(0x100000001B3);
    }
    return value;
}

void PE_MDEC_Init(void)
{
    g_color_dma_pos=0;
    g_dma1_port=0;
    g_dma0_port_bytes=0;
    g_cpu_command=g_cpu_words=g_cpu_received=0;
    g_dma0_source=0;
    memset(&g_mdec, 0, sizeof(g_mdec));
    memset(g_quant,0,sizeof(g_quant));memset(g_scale,0,sizeof(g_scale));g_tables=0;
    g_input_pos=g_input_count=g_current.pos=g_current.count=g_macroblocks=0;
    MdecClearOutputQueue();
    g_decode_valid=g_input_pending=0;g_output_bytes=0;
    g_decode_command=0;g_drain=0;
}

void PE_MDEC_BeginReset(void)
{
    g_dma1_port=0;
    g_dma0_port_bytes=0;
    uint32_t reset_count = g_mdec.reset_count + 1u;

    memset(&g_mdec, 0, sizeof(g_mdec));
    g_mdec.reset_count = reset_count;
    g_dma0_source=0;
    g_input_pos=g_input_count=g_current.pos=g_current.count=g_macroblocks=0;
    MdecClearOutputQueue();
    g_decode_valid=g_input_pending=0;g_output_bytes=0;
    PE_MDEC_WriteControl(0x80000000u);
}

void PE_MDEC_WriteControl(uint32_t value)
{
    g_mdec.control_last_write = value;
    g_mdec.control_write_count++;
    if (value & 0x80000000u) {
        g_color_dma_pos=0;
        g_cpu_command=g_cpu_words=g_cpu_received=0;
        /* MDEC1 reset aborts the command and both internal FIFOs. DMA
         * channel registers belong to the separate DMA controller, and
         * loaded quant/scale tables retain their existing reset semantics. */
        g_input_pos=g_input_count=g_current.pos=g_current.count=g_macroblocks=0;
        MdecClearOutputQueue();
        g_decode_valid=g_input_pending=0;g_decode_command=0;g_drain=0;
    }
}

void PE_MDEC_ClearDmaChannels(void)
{
    g_dma1_port=0;
    g_dma0_port_bytes=0;
    g_dma0_source = 0u;
    g_mdec.dma0_madr = 0u;
    g_mdec.dma0_bcr = 0u;
    g_mdec.dma0_chcr = 0u;
    g_mdec.dma1_chcr = 0u;
    g_mdec.dma1_madr=g_mdec.dma1_bcr=0;g_input_pending=0;g_output_bytes=0;
    g_mdec.dma0_active = 0;
}

int PE_MDEC_SubmitInputTable(pe_addr_t command_block, uint32_t size_words)
{
    uint32_t blocks = size_words >> 5;
    uint32_t words = blocks << 5;
    uint32_t bytes = words << 2;
    const uint8_t *payload;
    PeMdecUpload *upload;

    /* This helper submits 32-word request blocks. A zero BA encodes 65536
     * blocks in hardware, not an empty successful upload; out-of-width BA
     * would truncate when written. Validate before servicing an older DMA. */
    if(!blocks || blocks>0xFFFFu) {
        MdecBoundary("MDEC_input_block_count",size_words);return -1;
    }
    if(!PE_RangeIsRam(PE_RamCanonical(command_block,(size_t)bytes+4u),(size_t)bytes+4u)) {
        MdecBoundary("MDEC_input_table_range",command_block);return -1;
    }

    if(g_cpu_words) {
        MdecBoundary("MDEC_CPU_input_pending",g_cpu_words-g_cpu_received);return -1;
    }

    if (g_mdec.dma0_active) {
        (void)PE_MDEC_Service();
        if(PE_Port_ShouldStop()) return -1;
        if(g_mdec.dma0_active) {MdecBoundary("MDEC_input_wait",g_mdec.dma0_chcr);return -1;}
    }
    g_mdec.input_wait_count++;
    g_dma0_port_bytes=0;

    payload = PE_TranslateConst((pe_addr_t)(command_block + 4u), bytes);
    PE_GPU_WriteDPCR(PE_GPU_ReadDPCR() | 0x88u);
    g_mdec.command_last_write = PE_LoadU32(command_block);
    g_dma0_source = (pe_addr_t)(command_block + 4u);
    g_mdec.dma0_madr = g_dma0_source & 0x00FFFFFFu;
    g_mdec.dma0_bcr = (blocks << 16) | 0x20u;
    g_mdec.dma0_chcr = MDEC_DMA_CHCR_INPUT;
    g_mdec.dma0_active = 1;

    upload = &g_mdec.uploads[g_mdec.upload_count % PE_MDEC_UPLOAD_HISTORY];
    upload->command = g_mdec.command_last_write;
    upload->source = g_dma0_source;
    upload->word_count = words;
    upload->bcr = g_mdec.dma0_bcr;
    upload->payload_fnv1a64 = Fnv1a64(payload, bytes);
    /* Hash is submission telemetry; device input is read at DMA service. */
    if((g_mdec.command_last_write>>29u)==1u) g_input_pending=1;
    g_mdec.upload_count++;
    return 0;
}

void PE_MDEC_GetState(PeMdecState *out)
{
    if (out != NULL)
        *out = g_mdec;
}
