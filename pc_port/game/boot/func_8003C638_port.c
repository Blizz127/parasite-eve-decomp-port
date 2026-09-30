/* Retail model fade and color restoration, 2A19C.s / 2CE38.s.
 * Native translation; original instruction execution is the test authority. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

void func_8003CCB0(pe_addr_t dest, int enabled)
{
    static const unsigned sizes[4]={52,40,36,28}, special[4]={11,16,21,26};
    pe_addr_t obj=PE_LoadU32(dest), geometry, packets;
    uint32_t bank=PE_LoadU32(0x8009CDDCu);
    if (!obj || !PE_LoadU16(dest+0xBAu)) return;
    geometry=PE_LoadU32(dest+0x10u); packets=PE_LoadU32(dest+0x54u);
    for (unsigned type=0;type<4;type++)
        for (unsigned i=0;i<PE_LoadU16(obj+8u+type*2u);i++,geometry+=12u,packets+=sizes[type]*2u) {
            pe_addr_t command=packets+bank*sizes[type]+7u;
            uint8_t value=PE_LoadU8(command);
            int on=PE_LoadU8(geometry+3u)==special[type] || (int16_t)enabled!=0;
            PE_StoreU8(command,on?(value|2u):(value&0xFDu));
        }
}

void func_8003CEF8(pe_addr_t dest, int mode)
{
    static const unsigned sizes[2]={52,40};
    pe_addr_t obj=PE_LoadU32(dest), packets;
    uint32_t bank=PE_LoadU32(0x8009CDDCu);
    unsigned old=0;
    if (!obj || !PE_LoadU16(dest+0xBAu)) return;
    packets=PE_LoadU32(dest+0x54u);
    if (PE_LoadU16(obj+8u)) old=(PE_LoadU16(packets+bank*52u+26u)&127u)>>5;
    else if (PE_LoadU16(obj+10u)) old=(PE_LoadU16(packets+bank*40u+26u)&127u)>>5;
    if ((int16_t)mode==(int)old) return;
    int delta=(int16_t)PE_LoadU16(0x800921D8u+old*8u+(uint32_t)(int32_t)(int16_t)mode*2u);
    for (unsigned type=0;type<2;type++)
        for (unsigned i=0;i<PE_LoadU16(obj+8u+type*2u);i++,packets+=sizes[type]*2u) {
            pe_addr_t page=packets+bank*sizes[type]+26u;
            PE_StoreU16(page,(uint16_t)(PE_LoadU16(page)+delta));
        }
}

static void fade_lighting(pe_addr_t dest)
{
    uint32_t color=PE_LoadU8(dest+0x94u)|(PE_LoadU8(dest+0x95u)<<8u)|(PE_LoadU8(dest+0x96u)<<16u);
    PE_StoreU32(0x8009CDA0u,color);
    func_8003B97C_lighting_cut(dest,0x800BEA40u);
    if (!(PE_LoadU16(dest+0x9Cu)&8u))
        func_8003BCE0(dest,0,(int16_t)PE_LoadU16(0x8009CDDCu));
    PE_StoreU32(0x8009CDA0u,0x808080u);
}

int func_8003C638(pe_addr_t dest)
{
    static const unsigned steps[3]={0x8E,0x8F,0x93};
    if (!PE_LoadU32(dest) || !PE_LoadU16(dest+0xBAu)) return 1;
    int phase=(int8_t)PE_LoadU8(dest+0x8Cu);
    if (!phase) {
        PE_StoreU8(dest+0x8Cu,255u);
        func_8003CEF8(dest,0); func_8006698C(dest); func_8003CCB0(dest,0);
        PE_StoreU16(dest+0x9Cu,PE_LoadU16(dest+0x9Cu)|0x820u);
        return 1;
    }
    if (phase<0) {
        uint8_t saved[3];
        PE_StoreU8(dest+0x9Eu,1u);
        PE_StoreU16(dest+0x9Cu,PE_LoadU16(dest+0x9Cu)&0xFDFFu);
        func_8003CEF8(dest,1); func_8003CCB0(dest,1);
        for (unsigned i=0;i<3;i++) { saved[i]=PE_LoadU8(dest+0x90u+i); PE_StoreU8(dest+0x94u+i,0); }
        func_8003CAEC(dest,0,0,0);
        for (unsigned i=0;i<3;i++) PE_StoreU8(dest+0x90u+i,saved[i]);
        PE_StoreU8(dest+0x8Cu,PE_LoadU8(dest+0x8Du));
    } else if (phase==(int8_t)PE_LoadU8(dest+0x8Du)-1) {
        func_8003CEF8(dest,1); func_8003CCB0(dest,1);
    }
    fade_lighting(dest);
    for (unsigned i=0;i<3;i++)
        PE_StoreU8(dest+0x94u+i,(uint8_t)(PE_LoadU8(dest+0x94u+i)+PE_LoadU8(dest+steps[i])));
    PE_StoreU8(dest+0x8Cu,(uint8_t)(PE_LoadU8(dest+0x8Cu)-1u));
    return 0;
}

int func_8003C818(pe_addr_t dest)
{
    static const unsigned steps[3]={0x8E,0x8F,0x93};
    if (!dest || !PE_LoadU32(dest) || !PE_LoadU16(dest+0xBAu)) return 1;
    int phase=(int8_t)PE_LoadU8(dest+0x8Cu);
    if (!phase) {
        PE_StoreU8(dest+0x8Cu,255u); PE_StoreU32(0x8009CDA0u,0x808080u);
        return 1;
    }
    if (phase==1) PE_StoreU8(dest+0x9Eu,0);
    else if (phase<0) {
        PE_StoreU8(dest+0x8Cu,PE_LoadU8(dest+0x8Du));
        func_8003CCB0(dest,1); func_8003CEF8(dest,1);
        int divisor=(int8_t)PE_LoadU8(dest+0x8Du);
        if (!divisor) { Bootstrap_ReturnVoid("func_8003C818_divide_zero", "func_8003C818"); return 0; }
        for (unsigned i=0;i<3;i++) {
            unsigned color=PE_LoadU8(dest+0x90u+i);
            if (color>128u) color=128u;
            PE_StoreU8(dest+0x90u+i,(uint8_t)color);
            PE_StoreU8(dest+steps[i],(uint8_t)((int)color/divisor));
            PE_StoreU8(dest+0x94u+i,(uint8_t)color);
        }
    } else if (phase==(int8_t)PE_LoadU8(dest+0x8Du)-1) {
        func_8003CCB0(dest,1); func_8003CEF8(dest,1);
    } else {
        for (unsigned i=0;i<3;i++) {
            uint8_t color=(uint8_t)(PE_LoadU8(dest+0x94u+i)-PE_LoadU8(dest+steps[i]));
            PE_StoreU8(dest+0x94u+i,color>128u?0:color);
        }
    }
    fade_lighting(dest);
    PE_StoreU8(dest+0x8Cu,(uint8_t)(PE_LoadU8(dest+0x8Cu)-1u));
    return 0;
}

void func_8003B708(pe_addr_t dest, int bank)
{
    uint32_t saved[8];
    if (dest!=0x800B0CECu) {
        for (unsigned i=0;i<8;i++) saved[i]=PE_LoadU32(dest+0x34u+i*4u);
        for (unsigned i=0;i<8;i++) PE_StoreU32(dest+0x34u+i*4u,PE_LoadU32(0x80091A38u+i*4u));
        func_80039B74(dest,PE_LoadU32(dest+0xB0u),0,1);
        func_8003A088_mode0_walk_cut(dest);
    } else {
        pe_addr_t matrix=PE_LoadU32(0x800B0D70u);
        for (unsigned i=0;i<8;i++) PE_StoreU32(matrix+i*4u,PE_LoadU32(0x80091A38u+i*4u));
    }
    func_8006698C(dest); func_8003B97C_lighting_cut(dest,0x800BEA40u);
    func_8003BCE0(dest,1,(int16_t)bank);
    if (dest!=0x800B0CECu) {
        for (unsigned i=0;i<8;i++) PE_StoreU32(dest+0x34u+i*4u,saved[i]);
        func_80039B74(dest,PE_LoadU32(dest-4u),(int16_t)PE_LoadU16(dest-0x19Eu),1);
        func_8003A088_mode0_walk_cut(dest);
    }
}

void func_8003C0B4(pe_addr_t dest, int level, uint32_t red, uint32_t green, uint32_t blue)
{
    pe_addr_t obj=PE_LoadU32(dest);
    if (!obj || !PE_LoadU16(dest+0xBAu)) return;
    level=(int16_t)level;
    for (unsigned bone=0;bone<PE_LoadU8(obj+2u);bone++) {
        pe_addr_t rec=PE_LoadU32(dest+4u)+bone*12u;
        if (PE_LoadU8(rec+4u)!=1u) continue;
        pe_addr_t bounds=PE_LoadU32(dest+0x18u)+bone*16u;
        PE_GTE_LoadRT(PE_LoadU32(dest+0x84u)+bone*32u);
        PE_GTE_SetV0((int16_t)PE_LoadU16(bounds),(int16_t)PE_LoadU16(bounds+2u),(int16_t)PE_LoadU16(bounds+4u));
        PE_GTE_MVMVA(0x0480012u);
        int y=(int16_t)g_pe_gte.ir[1],radius=(int16_t)PE_LoadU16(bounds+6u);
        if (!(level<y) && !(level<y-radius) && !(level<y+radius)) continue;
        unsigned start=PE_LoadU16(rec);
        for (unsigned i=0;i<PE_LoadU16(rec+2u);i++) {
            pe_addr_t v=PE_LoadU32(dest+8u)+(start+i)*8u;
            PE_GTE_SetV0((int16_t)PE_LoadU16(v),(int16_t)PE_LoadU16(v+2u),(int16_t)PE_LoadU16(v+4u));
            PE_GTE_MVMVA(0x0480012u);
            if (level<(int16_t)g_pe_gte.ir[1]) {
                pe_addr_t color=0x800B1638u+(start+i)*4u;
                PE_StoreU8(color,(uint8_t)red); PE_StoreU8(color+1u,(uint8_t)green); PE_StoreU8(color+2u,(uint8_t)blue);
            }
        }
    }
    func_8003BCE0(dest,0,(int16_t)PE_LoadU16(0x8009CDDCu));
}
