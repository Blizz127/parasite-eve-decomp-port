/* SDK formatter71A84..72308. Explicit caller stack preserves vararg and
 * temporary-storage identity. Callers must supply established guest context;
 * this function does not allocate a fabricated guest scratch frame. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

static uint32_t formatter_boundary(pe_addr_t target,uint32_t a0,uint32_t a1,uint32_t a2)
{
    (void)Bootstrap_ReturnInt4Indirect("formatter unresolved call","PE_FormatterFrame",0,
        target,a0,a1,a2,0u,0,0);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0u;
}

/* saved[0..7] are incoming s0..s7; saved[8] is incoming ra. The return value
 * is valid only if no stop was requested. Register restoration belongs to the
 * caller's eventual machine-context adapter, not this memory/output kernel. */
uint32_t PE_FormatterFrame(pe_addr_t destination,pe_addr_t format,uint32_t arg0,
                          uint32_t arg1,pe_addr_t caller_sp,const uint32_t saved[9])
{
    pe_addr_t frame=caller_sp-0x250u;
    uint32_t emitted=0u,conversion,flags,value,length,source,width,precision;
#define F(offset) PE_LoadU32(frame+(offset))
#define S(offset,value_) PE_StoreU32(frame+(offset),(value_))
#define SIGN() PE_LoadU8(frame+0x211u)
    PE_StoreU32(caller_sp+4u,format);
    PE_StoreU32(caller_sp+8u,arg0);
    PE_StoreU32(caller_sp+12u,arg1);
    S(0x238u,saved[4]);S(0x248u,saved[8]);
    S(0x244u,saved[7]);S(0x240u,saved[6]);S(0x23Cu,saved[5]);
    S(0x234u,saved[3]);S(0x230u,saved[2]);S(0x22Cu,saved[1]);S(0x228u,saved[0]);
    S(0x254u,format);S(0x220u,caller_sp+8u);
    conversion=(uint32_t)(int32_t)(int8_t)PE_LoadU8(format);
    while(conversion) {
        if(conversion!='%')goto literal;
        /* Original copies three words, with all loads preceding the stores. */
        flags=PE_LoadU32(0x80094528u);width=PE_LoadU32(0x8009452Cu);
        precision=PE_LoadU32(0x80094530u);
        S(0x210u,flags);S(0x214u,width);S(0x218u,precision);
        for(;;) {
            format=F(0x254u);S(0x254u,format+1u);
            conversion=(uint32_t)(int32_t)(int8_t)PE_LoadU8(format+1u);
            if(conversion==' ')PE_StoreU8(frame+0x211u,conversion);
            else {
                uint32_t bit=conversion=='-'?1u:conversion=='+'?2u:conversion=='#'?4u:conversion=='0'?8u:0u;
                if(!bit)break;
                S(0x210u,F(0x210u)|bit);
            }
        }
        if(conversion=='*') {
            pe_addr_t arg=F(0x220u);width=PE_LoadU32(arg);S(0x220u,arg+4u);S(0x214u,width);
            if((int32_t)width<0) {flags=F(0x210u);S(0x214u,0u-width);S(0x210u,flags|1u);}
            S(0x254u,format+2u);
            conversion=(uint32_t)(int32_t)(int8_t)PE_LoadU8(format+2u);
        } else while(conversion-'0'<10u) {
            S(0x214u,F(0x214u)*10u+conversion-'0');
            format=F(0x254u);S(0x254u,format+1u);
            conversion=(uint32_t)(int32_t)(int8_t)PE_LoadU8(format+1u);
        }
        if(conversion=='.') {
            format=F(0x254u);S(0x254u,format+1u);
            conversion=(uint32_t)(int32_t)(int8_t)PE_LoadU8(format+1u);
            if(conversion=='*') {
                pe_addr_t arg=F(0x220u);precision=PE_LoadU32(arg);S(0x220u,arg+4u);
                S(0x218u,precision);S(0x254u,format+2u);
                conversion=(uint32_t)(int32_t)(int8_t)PE_LoadU8(format+2u);
            } else while(conversion-'0'<10u) {
                S(0x218u,F(0x218u)*10u+conversion-'0');
                format=F(0x254u);S(0x254u,format+1u);
                conversion=(uint32_t)(int32_t)(int8_t)PE_LoadU8(format+1u);
            }
            if((int32_t)F(0x218u)>=0)S(0x210u,F(0x210u)|0x10u);
        }
        flags=F(0x210u);source=frame+0x210u;
        if(flags&1u)S(0x210u,flags&~8u);
    dispatch:;
        uint32_t target=conversion-0x4Cu<45u?PE_LoadU32(0x80011644u+(conversion-0x4Cu)*4u):0x8007220Cu;
        switch(target) {
        case 0x80071D48u:case 0x80071D54u:case 0x80071D60u:
            S(0x210u,F(0x210u)|(target==0x80071D48u?0x20u:target==0x80071D54u?0x40u:0x80u));
            format=F(0x254u);S(0x254u,format+1u);
            conversion=(uint32_t)(int32_t)(int8_t)PE_LoadU8(format+1u);goto dispatch;
        case 0x8007220Cu:
            if(conversion!='%')goto finish;
            goto literal;
        case 0x8007212Cu: {
            pe_addr_t arg=F(0x220u);length=1u;value=PE_LoadU8(arg);
            PE_StoreU8(--source,value);S(0x220u,arg+4u);goto padded;
        }
        case 0x800721D8u:case 0x8007214Cu: {
            pe_addr_t arg=F(0x220u);flags=F(0x210u);source=PE_LoadU32(arg);S(0x220u,arg+4u);
            if(target==0x800721D8u) {
                if(flags&0x20u)PE_StoreU16(source,emitted);else PE_StoreU32(source,emitted);
                goto next;
            }
            if(!(flags&4u))return formatter_boundary(flags&0x10u?0x80072324u:0x80072314u,
                source,0u,flags&0x10u?F(0x218u):0u);
            length=PE_LoadU8(source++);
            if(flags&0x10u) {precision=F(0x218u);if((int32_t)precision<(int32_t)length)length=precision;}
            goto padded;
        }
        case 0x80071D8Cu:case 0x80071DE4u:case 0x80071F04u:
        case 0x80072004u:case 0x80072018u:case 0x80072024u:break;
        default:return formatter_boundary(target,0u,0u,0u);
        }
        if(target==0x80072004u) {flags=F(0x210u);S(0x218u,8u);S(0x210u,flags|0x50u);}
        {pe_addr_t arg=F(0x220u);value=PE_LoadU32(arg);S(0x220u,arg+4u);}
        flags=F(0x210u);
        if(flags&0x20u)value=target==0x80071D8Cu?(uint32_t)(int32_t)(int16_t)value:value&0xFFFFu;
        uint32_t base=target==0x80071D8Cu || target==0x80071DE4u?10u:target==0x80071F04u?8u:16u;
        if(target==0x80071D8Cu) {
            if((int32_t)value<0) {value=0u-value;PE_StoreU8(frame+0x211u,'-');}
            else if(flags&2u)PE_StoreU8(frame+0x211u,'+');
        } else if(target==0x80071DE4u)PE_StoreU8(frame+0x211u,0u);
        flags=F(0x210u);
        if(!(flags&0x10u)) {
            if(flags&8u) {
                precision=F(0x214u);
                if(base==10u && SIGN())precision--;
                else if(base==16u && (flags&4u))precision-=2u;
                S(0x218u,precision);
            }
            if((int32_t)F(0x218u)<=0)S(0x218u,1u);
        }
        length=0u;
        while(value) {
            uint32_t digit=value%base;value/=base;
            if(base==16u)digit=PE_LoadU8((target==0x80072024u?0x80011630u:0x8001161Cu)+digit);
            else digit+='0';
            PE_StoreU8(--source,digit);length++;
        }
        if(base==8u && (F(0x210u)&4u) && length && PE_LoadU8(source)!='0') {
            PE_StoreU8(--source,'0');length++;
        }
        while((int32_t)length<(int32_t)F(0x218u)) {PE_StoreU8(--source,'0');length++;}
        if(base==10u && SIGN()) {value=SIGN();PE_StoreU8(--source,value);length++;}
        if(base==16u && (F(0x210u)&4u)) {
            PE_StoreU8(--source,conversion);PE_StoreU8(--source,'0');length+=2u;
        }
    padded:
        if((int32_t)length<(int32_t)F(0x214u) && !(F(0x210u)&1u)) {
            do {
                PE_StoreU8(destination+emitted,' ');S(0x214u,F(0x214u)-1u);emitted++;
            } while((int32_t)length<(int32_t)F(0x214u));
        }
        (void)func_80072334(destination+emitted,source,length);
        emitted+=length;
        while((int32_t)length<(int32_t)F(0x214u)) {
            PE_StoreU8(destination+emitted,' ');width=F(0x214u);length++;emitted++;
            if((int32_t)length>=(int32_t)width)break;
        }
        goto next;
    literal:
        PE_StoreU8(destination+emitted,conversion);emitted++;
    next:
        format=F(0x254u);S(0x254u,format+1u);
        conversion=(uint32_t)(int32_t)(int8_t)PE_LoadU8(format+1u);
    }
finish:
    PE_StoreU8(destination+emitted,0u);
#undef SIGN
#undef S
#undef F
    return emitted;
}
