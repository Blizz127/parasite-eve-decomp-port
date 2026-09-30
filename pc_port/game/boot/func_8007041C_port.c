/* Retail weapon-range wireframe, 7041C..70D10 (60C1C.s), and its
 * animated battle caller 347B4 (24240.s). Matrices live in native storage;
 * projected coordinates, depth arrays and packets retain guest authority. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

static pe_addr_t range_edge(pe_addr_t vertices, pe_addr_t palette,
                            pe_addr_t xy, pe_addr_t z, pe_addr_t ot,
                            pe_addr_t packet, unsigned a, unsigned b, int floor)
{
    uint8_t flags=PE_LoadU8(palette+PE_LoadU8(vertices+a*8u+6u)*4u+3u);
    int32_t depth=((int16_t)PE_LoadU16(z+a*2u)+(int16_t)PE_LoadU16(z+b*2u))/8;
    unsigned i;
    PE_StoreU8(packet+3u,4u); PE_StoreU8(packet+7u,(flags&32u)?0x52u:0x50u);
    for (i=0;i<2;i++) {
        PE_StoreU8(packet+4u+i*8u,0u);
        PE_StoreU8(packet+5u+i*8u,floor?255u:128u);
        PE_StoreU8(packet+6u+i*8u,floor?255u:0u);
    }
    PE_StoreU32(packet+8u,PE_LoadU32(xy+a*4u));
    PE_StoreU32(packet+16u,PE_LoadU32(xy+b*4u));
    if (depth<4096) { func_80077AC4(ot+(uint32_t)(depth*4),packet); packet+=20u; }
    if (flags&32u) {
        func_80077C84(packet,0u,1u,(flags&3u)<<5u);
        if (depth<4096) { func_80077AC4(ot+(uint32_t)(depth*4),packet); packet+=8u; }
    }
    return packet;
}

uint32_t func_8007041C(pe_addr_t geometry, pe_addr_t position, int32_t radius,
                      uint32_t angle, pe_addr_t ot, pe_addr_t output)
{
    int16_t matrix[3][3]={{0}}, composed[3][3];
    int32_t translation[3];
    pe_addr_t vertices=geometry+28u, triangles, tail, palette, packet=output;
    pe_addr_t xy=PE_LoadU32(0x8009CDD0u), z=PE_LoadU32(0x8009CDD4u);
    uint32_t trig=PE_LoadU32(0x800966ECu+(angle&4095u)*4u);
    int32_t cosine=(int16_t)(trig>>16u), negsine=-(int32_t)(int16_t)trig;
    unsigned i,j,count=PE_LoadU8(geometry+1u);
    /* 79C74 rotates the diagonal scale around Y, with 32-bit wrapping
     * products and arithmetic shifts. Its input here is unsigned 16-bit. */
    for (i=0;i<3;i++) matrix[i][i]=(int16_t)((radius>>16)/4);
    for (j=0;j<3;j++) {
        int32_t a=matrix[0][j], b=matrix[2][j];
        matrix[0][j]=(int16_t)((int32_t)((uint32_t)(cosine*a)-(uint32_t)(negsine*b))>>12);
        matrix[2][j]=(int16_t)((int32_t)((uint32_t)(negsine*a)+(uint32_t)(cosine*b))>>12);
    }
    PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
    for (j=0;j<3;j++) {
        PE_GTE_SetIR(matrix[0][j],matrix[1][j],matrix[2][j]);
        PE_GTE_MVMVA(0x49E012u);
        for (i=0;i<3;i++) composed[i][j]=(int16_t)g_pe_gte.ir[i];
    }
    PE_GTE_SetV0((int16_t)PE_LoadU16(position+2u),(int16_t)PE_LoadU16(position+6u),
                 (int16_t)PE_LoadU16(position+10u));
    PE_GTE_MVMVA(0x480012u);
    for (i=0;i<3;i++) translation[i]=g_pe_gte.ir[i];
    for (i=0;i<3;i++) {
        g_pe_gte.tr[i]=translation[i];
        for (j=0;j<3;j++) g_pe_gte.rt[i][j]=composed[i][j];
    }
    for (i=0;i<count;i++) {
        uint32_t screen,depth;
        PE_GTE_SetV0((int16_t)PE_LoadU16(vertices+i*8u),
                     (int16_t)PE_LoadU16(vertices+i*8u+2u),(int16_t)PE_LoadU16(vertices+i*8u+4u));
        PE_GTE_RTPS_coordinates(&screen,&depth);
        PE_StoreU32(xy+i*4u,screen); PE_StoreU16(z+i*2u,(uint16_t)depth);
    }
    triangles=vertices+count*8u+(PE_LoadU8(geometry+8u)+PE_LoadU8(geometry+9u))*4u;
    count=PE_LoadU8(geometry+10u);
    tail=triangles+(count+PE_LoadU8(geometry+11u))*4u;
    for (i=12;i<16;i++) tail+=PE_LoadU8(geometry+i)*8u;
    tail+=PE_LoadU8(geometry+16u)*4u+(PE_LoadU8(geometry+17u)+PE_LoadU8(geometry+18u))*2u;
    for (i=0;i<PE_LoadU8(geometry+19u);i++) PE_StoreU8(tail+i*2u,PE_LoadU8(tail+i*2u)&127u);
    palette=geometry+PE_LoadU16(geometry+6u)*4u;
    for (i=0;i<count;i++) {
        unsigned indices[3]; int32_t sx[3],sy[3],vy[3]; int64_t area;
        for (j=0;j<3;j++) {
            indices[j]=PE_LoadU8(triangles+i*4u+j);
            sx[j]=(int16_t)PE_LoadU16(xy+indices[j]*4u);
            sy[j]=(int16_t)PE_LoadU16(xy+indices[j]*4u+2u);
            vy[j]=(int16_t)PE_LoadU16(vertices+indices[j]*8u+2u);
        }
        area=(int64_t)sx[0]*(sy[1]-sy[2])+(int64_t)sx[1]*(sy[2]-sy[0])+(int64_t)sx[2]*(sy[0]-sy[1]);
        if (area<=0) {
            /* Retail's back-facing path only retains a ground edge that
             * starts at vertex 0. The 1/2 candidate leaves s0=-1. */
            if (vy[0]==0 && (vy[1]==0 || vy[2]==0))
                packet=range_edge(vertices,palette,xy,z,ot,packet,indices[0],indices[vy[1]==0?1:2],1);
            else { PE_StoreU32(packet,0u); packet+=4u; }
        } else {
            static const unsigned pairs[3][2]={{0,1},{0,2},{1,2}};
            for (j=0;j<3;j++) {
                unsigned a=indices[pairs[j][0]], b=indices[pairs[j][1]];
                if (a>b) { unsigned swap=a; a=b; b=swap; }
                packet=range_edge(vertices,palette,xy,z,ot,packet,a,b,
                    PE_LoadU16(vertices+a*8u+2u)==0u && PE_LoadU16(vertices+b*8u+2u)==0u);
            }
        }
    }
    return packet-output;
}

void func_800347B4(void)
{
    uint32_t count=PE_LoadU32(0x8009D290u), angle, radius;
    pe_addr_t record=PE_LoadU32(0x8009D278u), aya=PE_LoadU32(0x8009D254u);
    uint32_t range=(PE_LoadU32(record+0x4Cu)&0x30u)==0x10u?200u:
        (uint32_t)(int32_t)(int16_t)PE_LoadU16(PE_LoadU32(record+0x68u)+2u);
    if (count<10u) { radius=((range<<16u)*count)/10u; angle=count<<6u; }
    else { radius=range<<16u; angle=(count<<4u)+10u; }
    if (!count) func_8006DE80(0x457,0,(int16_t)PE_LoadU16(aya+0x2Au),
        (int16_t)PE_LoadU16(aya+0x2Eu),(int16_t)PE_LoadU16(aya+0x32u));
    func_800661A4();
    {
        uint32_t bank=(uint32_t)PE_LoadU32(0x8009CDDCu), used=PE_LoadU32(0x8009CDD8u);
        used+=func_8007041C(PE_LoadU32(0x800B0DF8u),aya+0x28u,(int32_t)radius,angle&0xFFFEu,
            PE_LoadU32(0x800B0E38u+bank*4u)+8u,PE_LoadU32(0x800B0E58u+bank*4u)+used);
        PE_StoreU32(0x8009CDD8u,used);
    }
    func_800661CC(); PE_StoreU32(0x8009D290u,PE_LoadU32(0x8009D290u)+1u);
}
