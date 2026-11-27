/* Retail effect sprite packets, color curves and their SDK math.
 * Authority: complete BF0F0.s / 684EC.s instruction graphs. Transient
 * stack vectors/matrices are host arrays; persistent data stays guest RAM. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"

static int32_t effect_mul(int32_t a, int32_t b)
{ return (int32_t)((uint32_t)a * (uint32_t)b); }

/* func_800783E4: now pe_plat_gte_cmd2.c (clean-room, docs/evidence/native-gte-cmd-20261031) */

static void effect_color_blend(pe_addr_t a, pe_addr_t b, int32_t wa, int32_t wb, uint8_t out[3])
{
    unsigned i;
    PE_GTE_SetIR(PE_LoadU8(a),PE_LoadU8(a+1u),PE_LoadU8(a+2u));
    g_pe_gte.ir0=(int16_t)wa; PE_GTE_GPF(0,0);
    PE_GTE_SetIR(PE_LoadU8(b),PE_LoadU8(b+1u),PE_LoadU8(b+2u));
    g_pe_gte.ir0=(int16_t)wb; PE_GTE_GPL(0,0);
    for (i=0;i<3;i++) out[i]=(uint8_t)(g_pe_gte.mac[i]>>12);
}

/* func_80078554: now pe_plat_gte_cmd2.c (clean-room, docs/evidence/native-gte-cmd-20261031) */

static void effect_scale_matrix(int16_t matrix[9], const int32_t scale[3])
{
    unsigned i;
    for (i=0;i<9;i++) matrix[i]=(int16_t)(effect_mul(matrix[i],scale[i%3u])>>12);
}

/* func_80078CC4: now pe_plat_gte_regs.c (clean-room, docs/evidence/native-gte-setters-20261030) */

static void effect_multiply_rotation(int16_t matrix[9])
{
    int16_t result[9]; unsigned col,row;
    for (col=0;col<3;col++) {
        PE_GTE_SetV0(matrix[col],matrix[3u+col],matrix[6u+col]);
        PE_GTE_MVMVA(0x86000u);
        for (row=0;row<3;row++) result[row*3u+col]=(int16_t)g_pe_gte.ir[row];
    }
    for (row=0;row<9;row++) matrix[row]=result[row];
}

/* func_800786E4: now pe_plat_gte_cmd.c (clean-room, docs/evidence/native-gte-cmd-20261031) */

void PE_EffectColorCF3AC(pe_addr_t curve, uint8_t out[3], int32_t time)
{
    int32_t count=(int32_t)PE_LoadU32(curve+4u),total=(int32_t)PE_LoadU32(curve),i;
    pe_addr_t p=curve+8u;
    uint32_t duration;
    int32_t alpha;
    if (!total) {
        count=0;
        while ((duration=PE_LoadU8(p+3u))!=0u) {
            PE_StoreU16(p+6u,(uint16_t)total);
            total=(int32_t)((uint32_t)total+duration);count++;
            PE_StoreU16(p+4u,(uint16_t)duration);p+=8u;
        }
        PE_StoreU32(curve,(uint32_t)total);PE_StoreU32(curve+4u,(uint32_t)count);
    }
    if (time>total) time=total;
    p=curve+(uint32_t)count*8u;
    for (i=0;i<count;i++,p-=8u) if (time>=PE_LoadU16(p+6u)) break;
    duration=PE_LoadU16(p+4u);
    if (!duration) {
        /* The original executes BREAK 7 for this invalid curve. */
        Bootstrap_ReturnVoid("func_800CF3AC","zero color-curve duration (retail BREAK 7)");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }
    alpha=(int32_t)(((uint32_t)time-PE_LoadU16(p+6u))<<12u)/(int32_t)duration;
    effect_color_blend(p,p+8u,(int32_t)(4096u-(uint32_t)alpha),alpha,out);
}

void func_800CF3AC(pe_addr_t curve, pe_addr_t out, int32_t time)
{
    uint8_t color[3]={0};unsigned i;
    PE_EffectColorCF3AC(curve,color,time);
    for (i=0;i<3;i++) PE_StoreU8(out+i,color[i]);
}

void PE_EffectSpriteValuesCEE20(const int16_t position[3], const int16_t angles[4], int32_t sx,
    int32_t sy, int32_t texture, uint32_t clut, int32_t abr, int32_t brightness,
    const uint8_t color[3])
{
    int16_t default_angles[4],rotation[9];
    int32_t translation[3],scale[3],u,v;
    uint32_t packet_words[10]={0},bank=PE_LoadU32(0x8009CDDCu),offset=PE_LoadU32(0x8009CDD8u);
    uint32_t type=PE_LoadU16(0x800F3372u),count=PE_LoadU16(0x800E1210u+type*2u);
    uint32_t bias=PE_LoadU16(0x800F3374u),width=PE_LoadU16(0x800F3376u),height=PE_LoadU16(0x800F3378u);
    uint32_t page=PE_LoadU16(0x800F3370u),rgb=0,xy[3],z[3];
    pe_addr_t packet=PE_LoadU32(0x800B0E58u+bank*4u)+offset,vertices;
    unsigned i,j;
    if (!angles) {
        for (i=0;i<4;i++) default_angles[i]=(int16_t)PE_LoadU16(0x800C2268u+i*2u);
        angles=default_angles;
    }
    packet_words[0]=0x09000000u;
    if (abr!=255) page|=func_80077A64(0u,(uint32_t)abr,0u,0u);
    for (i=0;i<3;i++) rgb|=(uint32_t)(uint8_t)(color?effect_mul(brightness,color[i])/128:brightness)<<(i*8u);
    packet_words[1]=rgb|(abr==255?0x2C000000u:0x2E000000u);
    u=((uint32_t)texture&15u)*16u;v=texture/16*16;
    if (PE_LoadU16(0x800F336Eu) && ((uint32_t)texture&15u)>=8u) {u-=128;v+=32;}
    if (PE_LoadU16(0x800F336Cu)==4u && PE_LoadU32(0x800F3428u)) v+=96;
    packet_words[3]=(uint8_t)u|((uint32_t)(uint8_t)v<<8u)|((clut&65535u)<<16u);
    packet_words[5]=(uint8_t)(u+width-1u)|((uint32_t)(uint8_t)v<<8u)|((page&65535u)<<16u);
    packet_words[7]=(uint8_t)u|((uint32_t)(uint8_t)(v+height-1u)<<8u);
    packet_words[9]=(uint8_t)(u+width-1u)|((uint32_t)(uint8_t)(v+height-1u)<<8u);
    scale[0]=effect_mul(sx,(int32_t)(width>>4u));
    scale[1]=effect_mul(sy,(int32_t)(height>>4u));scale[2]=4096;
    PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
    PE_GTE_SetV0(position[0],position[1],position[2]);
    PE_GTE_MVMVA(0x80000u);
    for (i=0;i<3;i++) translation[i]=g_pe_gte.mac[i];
    PE_StoreU32(0x8009CDD8u,offset+count*40u);
    PE_RotMatrix79754_values(angles,rotation);effect_scale_matrix(rotation,scale);
    if (angles[3]==1) effect_multiply_rotation(rotation);
    for (i=0;i<9;i++) g_pe_gte.rt[i/3u][i%3u]=rotation[i];
    for (i=0;i<3;i++) g_pe_gte.tr[i]=translation[i];
    vertices=PE_LoadU32(0x800E13BCu+type*4u);
    for (i=0;i<count;i++,packet+=40u,vertices+=32u) {
        uint32_t depth,fourth_xy,fourth_z;
        PE_GTE_SetV0((int16_t)PE_LoadU16(vertices),(int16_t)PE_LoadU16(vertices+2u),(int16_t)PE_LoadU16(vertices+4u));
        PE_GTE_SetV1((int16_t)PE_LoadU16(vertices+8u),(int16_t)PE_LoadU16(vertices+10u),(int16_t)PE_LoadU16(vertices+12u));
        PE_GTE_SetV2((int16_t)PE_LoadU16(vertices+16u),(int16_t)PE_LoadU16(vertices+18u),(int16_t)PE_LoadU16(vertices+20u));
        PE_GTE_RTPT_coordinates(xy,z);
        for (j=0;j<10;j++) PE_StoreU32(packet+j*4u,packet_words[j]);
        for (j=0;j<3;j++) PE_StoreU32(packet+8u+j*8u,xy[j]);
        if (!g_pe_gte.mac0) break;
        depth=(z[2]>>2u)-bias;
        PE_GTE_SetV0((int16_t)PE_LoadU16(vertices+24u),(int16_t)PE_LoadU16(vertices+26u),(int16_t)PE_LoadU16(vertices+28u));
        PE_GTE_RTPS_coordinates(&fourth_xy,&fourth_z);
        if (depth<4096u) {
            pe_addr_t ot=PE_LoadU32(0x800B0E38u+PE_LoadU32(0x8009CDDCu)*4u)+depth*4u;
            PE_StoreU32(packet+32u,fourth_xy);
            PE_StoreU32(packet,(PE_LoadU32(packet)&0xFF000000u)|(PE_LoadU32(ot)&0xFFFFFFu));
            PE_StoreU32(ot,(PE_LoadU32(ot)&0xFF000000u)|(packet&0xFFFFFFu));
        }
    }
}

void PE_EffectSpriteCEE20(pe_addr_t position, const int16_t angles[4], int32_t sx,
    int32_t sy, int32_t texture, uint32_t clut, int32_t abr, int32_t brightness,
    const uint8_t color[3])
{
    int16_t vector[3];unsigned i;
    for (i=0;i<3;i++) vector[i]=(int16_t)PE_LoadU16(position+i*2u);
    PE_EffectSpriteValuesCEE20(vector,angles,sx,sy,texture,clut,abr,brightness,color);
}

void func_800CEE20(pe_addr_t position, pe_addr_t angles, int32_t sx, int32_t sy,
    int32_t texture, uint32_t clut, int32_t abr, int32_t brightness, pe_addr_t color)
{
    int16_t a[4];uint8_t c[3];unsigned i;
    if (angles) for (i=0;i<4;i++) a[i]=(int16_t)PE_LoadU16(angles+i*2u);
    if (color) for (i=0;i<3;i++) c[i]=PE_LoadU8(color+i);
    PE_EffectSpriteCEE20(position,angles?a:0,sx,sy,texture,clut,abr,brightness,color?c:0);
}

/* Original D004C: Gouraud triangle fan with alternating signed radii.
 * Scale precedes optional camera rotation; D0728 uses the opposite order. */
void PE_EffectFanD004C(const int16_t position[3],int32_t radius0,int32_t radius1,int32_t segments,
    const int16_t angles[4],int32_t sx,int32_t sy,const uint8_t center_color[3],
    const uint8_t edge_color[3],int32_t brightness,int32_t abr)
{
    int16_t defaults[4],rotation[9],radii[2]={(int16_t)radius0,(int16_t)radius1};
    int32_t scale[3]={sx,sy,(int32_t)PE_LoadU32(0x800C2298u)},translation[3];
    uint32_t packet_template[7]={0x06000000u,0x30000000u,0,0,0,0,0};
    unsigned i;
    if(segments<4)return;
    uint32_t bias=PE_LoadU16(0x800F3374u);
    if(!angles){for(i=0;i<4;i++)defaults[i]=(int16_t)PE_LoadU16(0x800C2260u+i*2u);angles=defaults;}
    for(i=0;i<3;i++) {
        uint32_t center=center_color?(uint8_t)(effect_mul(center_color[i],brightness)/128):0;
        uint32_t edge=edge_color?(uint8_t)(effect_mul(edge_color[i],brightness)/128):0;
        packet_template[1]|=center<<(i*8u);packet_template[3]|=edge<<(i*8u);packet_template[5]|=edge<<(i*8u);
    }
    PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));PE_GTE_SetV0(position[0],position[1],position[2]);
    PE_GTE_MVMVA(0x80000u);for(i=0;i<3;i++)translation[i]=g_pe_gte.mac[i];
    PE_RotMatrix79754_values(angles,rotation);effect_scale_matrix(rotation,scale);
    if(angles[3])effect_multiply_rotation(rotation);
    for(i=0;i<9;i++)g_pe_gte.rt[i/3u][i%3u]=rotation[i];
    for(i=0;i<3;i++)g_pe_gte.tr[i]=translation[i];
    uint32_t bank=PE_LoadU32(0x8009CDDCu),offset=PE_LoadU32(0x8009CDD8u);
    pe_addr_t packet=PE_LoadU32(0x800B0E58u+bank*4u)+offset;
    PE_StoreU32(0x8009CDD8u,offset+(uint32_t)segments*28u);
    for(int32_t segment=0;segment<segments;segment++,packet+=28u) {
        int32_t start=(int32_t)((uint32_t)segment<<12u)/segments;
        int32_t end=(int32_t)(((uint32_t)segment+1u)<<12u)/segments;
        int16_t x1=(int16_t)(effect_mul(func_80077DC4(start),radii[segment&1])/4096);
        int16_t y1=(int16_t)(effect_mul(func_80077CF4(start),radii[segment&1])/4096);
        int16_t x2=(int16_t)(effect_mul(func_80077DC4(end),radii[(segment+1)&1])/4096);
        int16_t y2=(int16_t)(effect_mul(func_80077CF4(end),radii[(segment+1)&1])/4096);
        uint32_t xy[3],z[3];
        PE_GTE_SetV0(0,0,0);PE_GTE_SetV1(x1,y1,0);PE_GTE_SetV2(x2,y2,0);
        PE_GTE_RTPT_coordinates(xy,z);
        for(i=0;i<7;i++)PE_StoreU32(packet+i*4u,packet_template[i]);
        for(i=0;i<3;i++)PE_StoreU32(packet+8u+i*8u,xy[i]);
        PE_GTE_AVSZ3(z);uint32_t depth=(z[2]>>2u)-bias;
        if(depth>=4096u)return;
        bank=PE_LoadU32(0x8009CDDCu);
        pe_addr_t ot=PE_LoadU32(0x800B0E38u+bank*4u)+depth*4u;
        if(abr!=255) {
            offset=PE_LoadU32(0x8009CDD8u);
            pe_addr_t mode=PE_LoadU32(0x800B0E58u+bank*4u)+offset;
            PE_StoreU32(0x8009CDD8u,offset+8u);
            func_80077C84(mode,0,1,func_80077A64(0,(uint32_t)abr,0,0)&65535u);
            PE_StoreU8(packet+7u,(uint8_t)(PE_LoadU8(packet+7u)|2u));
            func_80077AC4(ot,packet);func_80077AC4(ot,mode);
        }else func_80077AC4(ot,packet);
    }
}

void func_800D004C(pe_addr_t position,int32_t radius0,int32_t radius1,int32_t segments,
    pe_addr_t angles,int32_t sx,int32_t sy,pe_addr_t center_color,pe_addr_t edge_color,
    int32_t brightness,int32_t abr)
{
    int16_t p[3],a[4];uint8_t c0[3],c1[3];unsigned i;
    if(segments<4)return;
    for(i=0;i<3;i++)p[i]=(int16_t)PE_LoadU16(position+i*2u);
    if(angles)for(i=0;i<4;i++)a[i]=(int16_t)PE_LoadU16(angles+i*2u);
    if(center_color)for(i=0;i<3;i++)c0[i]=PE_LoadU8(center_color+i);
    if(edge_color)for(i=0;i<3;i++)c1[i]=PE_LoadU8(edge_color+i);
    PE_EffectFanD004C(p,radius0,radius1,segments,angles?a:0,sx,sy,center_color?c0:0,edge_color?c1:0,brightness,abr);
}

/* Full D0728: shaded annulus segments and per-segment draw-mode packets. */
void PE_EffectRingD0728(const int16_t position[3],int32_t inner,int32_t outer,int32_t segments,
    const int16_t angles[4],int32_t sx,int32_t sy,const uint8_t inner_color[3],
    const uint8_t outer_color[3],int32_t brightness,int32_t abr)
{
    int16_t defaults[4],rotation[9];
    int32_t scale[3]={sx,sy,(int32_t)PE_LoadU32(0x800C2298u)},translation[3],segment;
    uint32_t template[9]={0x08000000u,0x38000000u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t bank,bias=PE_LoadU16(0x800F3374u),offset;
    pe_addr_t packet;
    unsigned i;
    if (segments<4) return;
    if (!angles) {
        for (i=0;i<4;i++) defaults[i]=(int16_t)PE_LoadU16(0x800C2260u+i*2u);
        angles=defaults;
    }
    for (i=0;i<3;i++) {
        uint32_t inside=inner_color?(uint8_t)(effect_mul(inner_color[i],brightness)/128):0u;
        uint32_t outside=outer_color?(uint8_t)(effect_mul(outer_color[i],brightness)/128):0u;
        template[1]|=outside<<(i*8u);template[3]|=outside<<(i*8u);
        template[5]|=inside<<(i*8u);template[7]|=inside<<(i*8u);
    }
    PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));PE_GTE_SetV0(position[0],position[1],position[2]);
    PE_GTE_MVMVA(0x80000u);
    for (i=0;i<3;i++) translation[i]=g_pe_gte.mac[i];
    PE_RotMatrix79754_values(angles,rotation);
    if (angles[3]) effect_multiply_rotation(rotation);
    effect_scale_matrix(rotation,scale);
    for (i=0;i<9;i++) g_pe_gte.rt[i/3u][i%3u]=rotation[i];
    for (i=0;i<3;i++) g_pe_gte.tr[i]=translation[i];
    bank=PE_LoadU32(0x8009CDDCu);offset=PE_LoadU32(0x8009CDD8u);
    packet=PE_LoadU32(0x800B0E58u+bank*4u)+offset;
    PE_StoreU32(0x8009CDD8u,offset+(uint32_t)segments*36u);
    for (segment=0;segment<segments;segment++,packet+=36u) {
        int32_t start=(int32_t)((uint32_t)segment<<12u)/segments;
        int32_t end=(int32_t)(((uint32_t)segment+1u)<<12u)/segments;
        int16_t vertices[4][3];uint32_t xy[3],z[3],fourth_xy,fourth_z,depth;
        pe_addr_t ot;
        for (i=0;i<4;i++) {
            int32_t angle=(i&1u)?end:start,radius=i<2u?outer:inner;
            vertices[i][0]=(int16_t)(effect_mul(func_80077DC4(angle),radius)/4096);
            vertices[i][1]=(int16_t)(effect_mul(func_80077CF4(angle),radius)/4096);
            vertices[i][2]=0;
        }
        PE_GTE_SetV0(vertices[0][0],vertices[0][1],0);
        PE_GTE_SetV1(vertices[1][0],vertices[1][1],0);
        PE_GTE_SetV2(vertices[2][0],vertices[2][1],0);
        PE_GTE_RTPT_coordinates(xy,z);
        for (i=0;i<9;i++) PE_StoreU32(packet+i*4u,template[i]);
        for (i=0;i<3;i++) PE_StoreU32(packet+8u+i*8u,xy[i]);
        PE_GTE_AVSZ3(z);depth=(z[2]>>2u)-bias;
        PE_GTE_SetV0(vertices[3][0],vertices[3][1],0);
        PE_GTE_RTPS_coordinates(&fourth_xy,&fourth_z);
        if (depth>=4096u) return;
        PE_StoreU32(packet+32u,fourth_xy);
        bank=PE_LoadU32(0x8009CDDCu);ot=PE_LoadU32(0x800B0E38u+bank*4u)+depth*4u;
        if (abr!=255) {
            pe_addr_t mode;
            offset=PE_LoadU32(0x8009CDD8u);mode=PE_LoadU32(0x800B0E58u+bank*4u)+offset;
            PE_StoreU32(0x8009CDD8u,offset+8u);
            func_80077C84(mode,0u,1u,func_80077A64(0u,(uint32_t)abr,0u,0u)&65535u);
            PE_StoreU8(packet+7u,(uint8_t)(PE_LoadU8(packet+7u)|2u));
            func_80077AC4(ot,packet);func_80077AC4(ot,mode);
        } else func_80077AC4(ot,packet);
    }
}

void func_800D0728(pe_addr_t position,int32_t inner,int32_t outer,int32_t segments,
    pe_addr_t angles,int32_t sx,int32_t sy,pe_addr_t inner_color,pe_addr_t outer_color,
    int32_t brightness,int32_t abr)
{
    int16_t p[3],a[4];uint8_t c0[3],c1[3];unsigned i;
    for (i=0;i<3;i++) p[i]=(int16_t)PE_LoadU16(position+i*2u);
    if (angles) for (i=0;i<4;i++) a[i]=(int16_t)PE_LoadU16(angles+i*2u);
    if (inner_color) for (i=0;i<3;i++) c0[i]=PE_LoadU8(inner_color+i);
    if (outer_color) for (i=0;i<3;i++) c1[i]=PE_LoadU8(outer_color+i);
    PE_EffectRingD0728(p,inner,outer,segments,angles?a:0,sx,sy,inner_color?c0:0,outer_color?c1:0,brightness,abr);
}

/* Full D2370: a shaded textured ribbon along the rotated local Z axis. */
void PE_EffectRibbonD2370(const int16_t position[3],const int16_t angles[3],
    int32_t length,int32_t width,int32_t u,int32_t v,int32_t uw,int32_t vh,
    uint32_t clut,const uint8_t start_color[3],const uint8_t end_color[3],
    int32_t brightness,int32_t abr)
{
    uint32_t bank=PE_LoadU32(0x8009CDDCu),offset=PE_LoadU32(0x8009CDD8u);
    pe_addr_t packet=PE_LoadU32(0x800B0E58u+bank*4u)+offset;
    uint32_t page=PE_LoadU16(0x800F3370u),bias=PE_LoadU16(0x800F3374u),xy[3],z[3],depth,last_xy,last_z;
    int16_t rotation[9];int32_t translation[3],half=width/2;unsigned i;
    PE_StoreU32(0x8009CDD8u,offset+52u);
    PE_StoreU8(packet+3u,12u);PE_StoreU8(packet+7u,abr==255?0x3Cu:0x3Eu);
    if (abr!=255) page|=func_80077A64(0u,(uint32_t)abr,0u,0u);
    PE_StoreU16(packet+26u,(uint16_t)page);PE_StoreU16(packet+14u,(uint16_t)clut);
    PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));PE_GTE_SetV0(position[0],position[1],position[2]);
    PE_GTE_MVMVA(0x80000u);
    for (i=0;i<3;i++) translation[i]=g_pe_gte.mac[i];
    PE_RotMatrix79754_values(angles,rotation);effect_multiply_rotation(rotation);
    for (i=0;i<9;i++) g_pe_gte.rt[i/3u][i%3u]=rotation[i];
    for (i=0;i<3;i++) g_pe_gte.tr[i]=translation[i];
    PE_GTE_SetV0((int16_t)(0u-(uint32_t)half),0,0);PE_GTE_SetV1((int16_t)half,0,0);
    PE_GTE_SetV2((int16_t)(0u-(uint32_t)half),0,(int16_t)length);PE_GTE_RTPT_coordinates(xy,z);
    for (i=0;i<3;i++) {
        uint8_t a=(uint8_t)(effect_mul(start_color?start_color[i]:PE_LoadU8(0x800C22A0u+i),(int16_t)brightness)/128);
        uint8_t b=(uint8_t)(effect_mul(end_color?end_color[i]:PE_LoadU8(0x800C22A0u+i),(int16_t)brightness)/128);
        PE_StoreU8(packet+4u+i,a);PE_StoreU8(packet+16u+i,a);
        PE_StoreU8(packet+28u+i,b);PE_StoreU8(packet+40u+i,b);
    }
    if (!g_pe_gte.mac0) return;
    for (i=0;i<3;i++) PE_StoreU32(packet+8u+i*12u,xy[i]);
    PE_GTE_AVSZ3(z);depth=(z[2]>>2u)-bias;
    if (depth>=4096u) return;
    PE_GTE_SetV0((int16_t)half,0,(int16_t)length);PE_GTE_RTPS_coordinates(&last_xy,&last_z);
    PE_StoreU8(packet+12u,(uint8_t)u);PE_StoreU8(packet+24u,(uint8_t)u);
    PE_StoreU8(packet+36u,(uint8_t)((uint32_t)u+(uint32_t)uw-1u));
    PE_StoreU8(packet+48u,(uint8_t)((uint32_t)u+(uint32_t)uw-1u));
    PE_StoreU8(packet+13u,(uint8_t)v);PE_StoreU8(packet+37u,(uint8_t)v);
    PE_StoreU8(packet+25u,(uint8_t)((uint32_t)v+(uint32_t)vh-1u));
    PE_StoreU8(packet+49u,(uint8_t)((uint32_t)v+(uint32_t)vh-1u));
    PE_StoreU32(packet+44u,last_xy);
    func_80077AC4(PE_LoadU32(0x800B0E38u+PE_LoadU32(0x8009CDDCu)*4u)+depth*4u,packet);
}

void func_800D2370(pe_addr_t position,pe_addr_t angles,int32_t length,int32_t width,
    int32_t u,int32_t v,int32_t uw,int32_t vh,uint32_t clut,pe_addr_t start_color,
    pe_addr_t end_color,int32_t brightness,int32_t abr)
{
    int16_t p[3],a[3];uint8_t c0[3],c1[3];unsigned i;
    for (i=0;i<3;i++) {p[i]=(int16_t)PE_LoadU16(position+i*2u);a[i]=(int16_t)PE_LoadU16(angles+i*2u);}
    if (start_color) for (i=0;i<3;i++) c0[i]=PE_LoadU8(start_color+i);
    if (end_color) for (i=0;i<3;i++) c1[i]=PE_LoadU8(end_color+i);
    PE_EffectRibbonD2370(p,a,length,width,u,v,uw,vh,clut,start_color?c0:0,end_color?c1:0,brightness,abr);
}
