/* Field floor collision, from retail AF84.s:
 * 1AE40 movement/height, 1B5FC neighboring-edge search, 1C164 triangle
 * crossing, and 1C7DC wall slide. Uses the map's 22/28-byte triangles,
 * shared edge records and actor radius; no scene-specific barriers.
 *
 * Includes 1AE40's cached-wall gate and 1D170's script polygon boundary.
 * Invalid/absent mesh records are ignored for resource-free VM fixtures.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

#define MESH_P  0x8009D1FCu
#define PLANES  0x8009D1D8u
#define EDGES   0x8009CE14u
#define WALL    0x8009CE0Cu
#define WALL_ID 0x8009CE18u
#define RADIUS  0x8009CE2Cu
#define VISITED 0x8009DFB0u
#define AYA     0x8009D254u
#define INPUT   0x8009D2E8u

static int32_t floor_s16(pe_addr_t a) { return (int16_t)PE_LoadU16(a); }
static int32_t floor_mul(int32_t a, int32_t b)
{
    return (int32_t)((int64_t)a * b);
}
static int32_t floor_sub(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}
static int32_t floor_add(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}
static int32_t floor_abs(int32_t a)
{
    return a < 0 ? (int32_t)(0u - (uint32_t)a) : a;
}
static int32_t floor_div(int32_t a, int32_t b)
{
    if (b == 0) return a < 0 ? 1 : -1;
    if (a == INT32_MIN && b == -1) return INT32_MIN;
    return a / b;
}
static unsigned floor_stride(void)
{
    return PE_LoadU32(PLANES) ? 28u : 22u;
}
static int floor_valid(pe_addr_t rec)
{
    pe_addr_t mesh = PE_LoadU32(MESH_P);
    pe_addr_t base;
    unsigned stride = floor_stride();
    if (!mesh || !PE_RangeIsRam(mesh, 40u)) return 0;
    base = PE_LoadU32(mesh + 0x1Cu);
    return base != 0u && rec >= base && (rec - base) % stride == 0u
        && (rec - base) / stride < PE_LoadU16(mesh + 4u)
        && PE_RangeIsRam(rec, stride);
}
static void floor_vertex(unsigned id, int32_t *x, int32_t *z)
{
    int three_d = PE_LoadU32(PLANES) != 0u;
    pe_addr_t v = PE_LoadU32(PE_LoadU32(MESH_P) + 0x18u)
        + id * (three_d ? 6u : 4u);
    *x = floor_s16(v);
    *z = floor_s16(v + (three_d ? 4u : 2u));
}
static void floor_clear_visited(void)
{
    unsigned i, count = PE_LoadU16(PE_LoadU32(MESH_P) + 8u);
    for (i = 0; i < count && i < 20u; i++)
        PE_StoreU32(VISITED + i * 4u, 0u);
}

/* 1B5FC's two layout arms share the same three-edge radius test.
 * Return zero with WALL/ID published on the first solid edge. */
static int floor_edges(int32_t x, int32_t z, pe_addr_t rec, unsigned depth)
{
    int three_d = PE_LoadU32(PLANES) != 0u;
    unsigned vertex_off = three_d ? 8u : 2u;
    unsigned edge_off = three_d ? 14u : 8u;
    unsigned neighbor_off = three_d ? 20u : 14u;
    unsigned last_id = PE_LoadU16(rec + vertex_off + 4u);
    int32_t last_x, last_z;
    pe_addr_t neighbors[3] = {0,0,0};
    int32_t radius = PE_LoadU16(RADIUS);
    unsigned i;
    if (!floor_valid(rec) || depth > 640u) return 0;
    floor_vertex(last_id, &last_x, &last_z);
    for (i = 0; i < 3u; i++) {
        unsigned id = PE_LoadU16(rec + vertex_off + i * 2u);
        unsigned edge = PE_LoadU16(rec + edge_off + i * 2u);
        pe_addr_t mark = VISITED + (edge >> 5) * 4u;
        uint32_t bit = 1u << (edge & 31u);
        int32_t px, pz, dx, dz, distance, along, ax, az;
        pe_addr_t normal;
        unsigned neighbor;
        floor_vertex(id, &px, &pz);
        if (edge >= 640u || (PE_LoadU32(mark) & bit)) goto next;
        PE_StoreU32(mark, PE_LoadU32(mark) | bit);
        if ((px < x-radius && last_x < x-radius)
            || (px > x+radius && last_x > x+radius)
            || (pz < z-radius && last_z < z-radius)
            || (pz > z+radius && last_z > z+radius)) goto next;
        normal = PE_LoadU32(EDGES) + edge * 12u;
        dx = x-px; dz = z-pz;
        distance = floor_div(floor_sub(floor_mul(dz, last_x-px),
                                      floor_mul(dx, last_z-pz)),
                             floor_s16(normal + 2u));
        if (floor_abs(distance) > radius) goto next;
        ax = id < last_id ? dx : x-last_x;
        az = id < last_id ? dz : z-last_z;
        along = (int32_t)(func_8003708C(PE_LoadU32(normal+4u), (uint32_t)ax << 16)
                       + func_8003708C(PE_LoadU32(normal+8u), (uint32_t)az << 16));
        if (along < 0) {
            along = floor_add(floor_mul(ax,ax), floor_mul(az,az));
            if (along > floor_mul(radius,radius)) goto next;
        }
        if ((int32_t)PE_LoadU32(normal) < along) {
            ax = id < last_id ? x-last_x : dx;
            az = id < last_id ? z-last_z : dz;
            if (floor_add(floor_mul(ax,ax), floor_mul(az,az))
                > floor_mul(radius,radius)) goto next;
        }
        neighbor = PE_LoadU16(rec + neighbor_off + i*2u);
        if (neighbor != 0xFFFFu) {
            pe_addr_t next_rec = PE_LoadU32(PE_LoadU32(MESH_P)+0x1Cu)
                + neighbor * floor_stride();
            if (floor_valid(next_rec) && !(PE_LoadU8(next_rec) & 0x80u)) {
                neighbors[i] = next_rec;
                goto next;
            }
        }
        PE_StoreU16(WALL, (uint16_t)px);
        PE_StoreU16(WALL+2u, (uint16_t)pz);
        PE_StoreU16(WALL+4u, (uint16_t)last_x);
        PE_StoreU16(WALL+6u, (uint16_t)last_z);
        PE_StoreU16(0x8009CE1Cu, (uint16_t)(px > last_x ? px : last_x));
        PE_StoreU16(0x8009CE20u, (uint16_t)(px < last_x ? px : last_x));
        PE_StoreU16(0x8009CE24u, (uint16_t)(pz > last_z ? pz : last_z));
        PE_StoreU16(0x8009CE28u, (uint16_t)(pz < last_z ? pz : last_z));
        PE_StoreU16(WALL_ID, (uint16_t)edge);
        return 0;
    next:
        last_id = id; last_x = px; last_z = pz;
    }
    for (i = 0; i < 3u; i++)
        if (neighbors[i] && !floor_edges(x,z,neighbors[i],depth+1u)) return 0;
    return 1;
}

/* 1C7DC: project the attempted step onto the wall tangent, then move
 * outward by whole map units until the actor radius clears the edge. */
static void floor_slide(pe_addr_t actor)
{
    pe_addr_t n = PE_LoadU32(EDGES) + PE_LoadU16(WALL_ID)*12u;
    uint32_t tx = PE_LoadU32(n+4u), tz = PE_LoadU32(n+8u);
    uint32_t old_x = PE_LoadU32(actor+0x40u), old_z = PE_LoadU32(actor+0x48u);
    uint32_t along = func_8003708C(tx, PE_LoadU32(actor+0x28u)-old_x)
                   + func_8003708C(tz, PE_LoadU32(actor+0x30u)-old_z);
    uint32_t base_x = old_x + func_8003708C(tx,along);
    uint32_t base_z = old_z + func_8003708C(tz,along);
    int32_t px = floor_s16(WALL), pz = floor_s16(WALL+2u);
    int32_t dx = floor_s16(WALL+4u)-px, dz = floor_s16(WALL+6u)-pz;
    int32_t radius = PE_LoadU16(RADIUS);
    unsigned step, dir;
    PE_StoreU32(INPUT, PE_LoadU32(INPUT) | 8u);
    for (step=0;;step++) {
        for (dir=0; dir<4u; dir++) {
            uint32_t x=base_x, z=base_z, offset=step<<16;
            int32_t distance;
            if (dir==0) x+=offset;
            else if (dir==1) x-=offset;
            else if (dir==2) z+=offset;
            else z-=offset;
            distance = floor_div(floor_sub(
                floor_mul(((int32_t)z>>16)-pz, dx),
                floor_mul(((int32_t)x>>16)-px, dz)), floor_s16(n+2u));
            if (floor_abs(distance) > radius) {
                PE_StoreU32(actor+0x28u,x);
                PE_StoreU32(actor+0x30u,z);
                return;
            }
        }
    }
}

/* 1C164: follow only neighbors crossed by this movement segment. */
static pe_addr_t floor_crossing(pe_addr_t rec, pe_addr_t previous,
    int32_t x, int32_t z, int32_t old_x, int32_t old_z, unsigned depth)
{
    unsigned voff = PE_LoadU32(PLANES) ? 8u : 2u;
    unsigned noff = PE_LoadU32(PLANES) ? 20u : 14u;
    int32_t px,pz, vx=old_x-x, vz=old_z-z;
    unsigned i;
    if (!floor_valid(rec) || depth > 640u) return 0;
    floor_vertex(PE_LoadU16(rec+voff+4u), &px,&pz);
    for (i=0; i<3u; i++) {
        int32_t qx,qz, ex,ez, ax,az, den,a,b;
        unsigned neighbor;
        pe_addr_t next_rec, result;
        floor_vertex(PE_LoadU16(rec+voff+i*2u), &qx,&qz);
        ex=qx-px; ez=qz-pz; ax=x-qx; az=z-qz;
        den=floor_sub(floor_mul(vz,ex), floor_mul(vx,ez));
        a=floor_sub(floor_mul(ez,ax), floor_mul(ex,az));
        b=floor_sub(floor_mul(vx,az), floor_mul(vz,ax));
        px=qx; pz=qz;
        if (!den || (den>0 ? (a<0 || a>den || b<0 || b>den)
                              : (a>0 || a<den || b>0 || b<den))) continue;
        neighbor=PE_LoadU16(rec+noff+i*2u);
        if (neighbor==0xFFFFu) continue;
        next_rec=PE_LoadU32(PE_LoadU32(MESH_P)+0x1Cu)+neighbor*floor_stride();
        if (next_rec==previous || !floor_valid(next_rec)
            || (PE_LoadU8(next_rec)&0x80u)) continue;
        if (func_8001C614(next_rec,x,z)) return next_rec;
        result=floor_crossing(next_rec,rec,x,z,old_x,old_z,depth+1u);
        if (result) return result;
    }
    return 0;
}

/* 1AF50..1B174: retain the previous wall while moving toward it within
 * its extent. The first xmax test deliberately uses z, not z-radius;
 * the retail branch is asymmetric. Extent differences wrap to s16. */
static int floor_cached_wall(int32_t x, int32_t z, int32_t old_x, int32_t old_z)
{
    int32_t current, previous;
    int32_t radius=PE_LoadU16(RADIUS);
    int32_t xmax=floor_s16(0x8009CE1Cu), xmin=floor_s16(0x8009CE20u);
    int32_t zmax=floor_s16(0x8009CE24u), zmin=floor_s16(0x8009CE28u);
    int32_t width=(int16_t)(xmax-xmin), height=(int16_t)(zmax-zmin);
    g_pe_gte.sxy[0]=PE_LoadU32(WALL);
    g_pe_gte.sxy[1]=PE_LoadU32(WALL+4u);
    g_pe_gte.sxy[2]=(uint16_t)x | ((uint32_t)(uint16_t)z<<16);
    current=PE_GTE_NCLIP();
    g_pe_gte.sxy[2]=(uint16_t)old_x | ((uint32_t)(uint16_t)old_z<<16);
    previous=PE_GTE_NCLIP();
    if ((int32_t)((uint32_t)current^(uint32_t)previous)>=0
        && floor_abs(previous)<floor_abs(current)) return 0;
    if (xmax<x-radius && (height<width || zmax<z || z+radius<zmin)) return 0;
    if (x+radius<xmin && (height<width || zmax<z-radius || z+radius<zmin)) return 0;
    if (zmax<z-radius && (width<height || xmax<x-radius || x+radius<xmin)) return 0;
    if (z+radius<zmin && (width<height || xmax<x-radius || x+radius<xmin)) return 0;
    return 1;
}

void func_8001AE40(pe_addr_t actor)
{
    pe_addr_t rec = PE_LoadU32(actor+0x1A4u);
    int32_t x=floor_s16(actor+0x2Au), z=floor_s16(actor+0x32u);
    int32_t old_x=floor_s16(actor+0x42u), old_z=floor_s16(actor+0x4Au);
    uint32_t flags=PE_LoadU32(actor+0x98u)&~0x80000u;
    int is_aya=actor==PE_LoadU32(AYA);
    int pass;
    if (!floor_valid(rec) || PE_LoadU32(EDGES)==0u) return;
    PE_StoreU32(actor+0x98u, flags);
    if (x==old_x && z==old_z
        && PE_LoadU32(actor+0x2Cu)==PE_LoadU32(actor+0x44u)) {
        if (is_aya) PE_StoreU32(INPUT,PE_LoadU32(INPUT)&~8u);
        return;
    }
    PE_StoreU32(actor+0x1A8u,rec);
    PE_StoreU16(RADIUS,(uint16_t)((int32_t)((uint32_t)PE_LoadU16(actor+0x224u)
                                      * PE_LoadU16(actor+0x26u)) / 4096));
    pass=0;
    if (!is_aya || !(PE_LoadU32(INPUT)&8u)
        || !floor_cached_wall(x,z,old_x,old_z)) {
        if (is_aya) PE_StoreU32(INPUT,PE_LoadU32(INPUT)&~8u);
        floor_clear_visited();
        pass=floor_edges(x,z,rec,0);
    }
    if (!pass) {
        unsigned wall;
        if (!is_aya) goto reject;
        floor_slide(actor);
        x=floor_s16(actor+0x2Au); z=floor_s16(actor+0x32u);
        floor_clear_visited();
        wall=PE_LoadU16(WALL_ID);
        pass=floor_edges(x,z,rec,0);
        if (!pass) {
            if (wall==PE_LoadU16(WALL_ID)) {
                floor_clear_visited();
                PE_StoreU32(VISITED+(wall>>5)*4u,1u<<(wall&31u));
                if (floor_edges(x,z,rec,0)) goto reject;
            }
            floor_slide(actor);
            x=floor_s16(actor+0x2Au); z=floor_s16(actor+0x32u);
            floor_clear_visited();
            if (!floor_edges(x,z,rec,0)) goto reject;
        }
    }
    if (!func_8001C614(rec,x,z)) {
        rec=floor_crossing(rec,0,x,z,old_x,old_z,0);
    }
    /* Retail does not reject a zero crossing result (1B3A0). Its following
     * loads use physical RAM at address zero; retain the zero result when
     * publishing the triangle pointer, translating only the record reads. */
    pe_addr_t height_rec=rec?rec:PE_RAM_BASE;
    if (PE_LoadU32(PLANES)) {
        pe_addr_t plane=PE_LoadU32(PLANES)+PE_LoadU16(height_rec+2u)*12u;
        uint32_t y=PE_LoadU32(height_rec+4u)
            - func_8003708C(PE_LoadU32(plane),PE_LoadU32(actor+0x28u))
            - func_8003708C(PE_LoadU32(plane+8u),PE_LoadU32(actor+0x30u));
        y=func_8003708C(y,PE_LoadU32(plane+4u));
        if ((flags&2u) && (int32_t)PE_LoadU32(actor+0x2Cu)<(int32_t)y) {
            /* Still above the floor during scripted gravity. */
        } else {
            PE_StoreU32(actor+0x2Cu,y);
            if (flags&2u) {
                PE_StoreU32(actor+0x6Cu,0);
                PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)&~2u);
            }
        }
    } else {
        unsigned layer=PE_LoadU8(height_rec+1u);
        pe_addr_t entry=PE_LoadU32(PE_LoadU32(0x8009CE08u)+layer*4u);
        int32_t height=floor_s16(entry);
        if (flags&2u) {
            if ((int32_t)PE_LoadU32(actor+0x2Cu)>=height
                && (int32_t)PE_LoadU32(actor+0x44u)<height) {
                PE_StoreU32(actor+0x2Cu,(uint32_t)height);
                PE_StoreU32(actor+0x6Cu,0);
                PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)&~2u);
            }
        } else if (layer!=PE_LoadU8(PE_LoadU32(actor+0x1A4u)+1u)) {
            if (floor_abs(height-floor_s16(actor+0x2Eu))
                >= (int32_t)((uint32_t)PE_LoadU16(actor+0x10u)<<16)) goto reject;
            PE_StoreU32(actor+0x2Cu,(uint32_t)height<<16);
        }
    }
    PE_StoreU32(actor+0x1A4u,rec);
    return;
reject:
    PE_StoreU32(actor+0x28u,PE_LoadU32(actor+0x40u));
    PE_StoreU32(actor+0x2Cu,PE_LoadU32(actor+0x44u));
    PE_StoreU32(actor+0x30u,PE_LoadU32(actor+0x48u));
    PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x80000u);
}

/* 1CE88: radius contact with the closed script polygon. Coordinates are
 * signed high halves of 16.16 points; return the contacted edge's index. */
static int floor_polygon_contact(int32_t x, int32_t z, pe_addr_t points, unsigned count)
{
    /* Original's two unused endpoint reads precede this empty-list return. */
    if (!count) return -1;
    int32_t px=floor_s16(points+count*8u-6u);
    int32_t pz=floor_s16(points+count*8u-2u);
    int32_t radius=PE_LoadU16(RADIUS);
    for (unsigned i=0;i<count;i++) {
        int32_t qx=floor_s16(points+i*8u+2u), qz=floor_s16(points+i*8u+6u);
        int32_t dx=px-qx, dz=pz-qz, ax=x-qx, az=z-qz;
        int32_t in[3]={dx,0,dz}, normal[3];
        uint32_t sum;
        int32_t length, distance, along;
        px=qx; pz=qz;
        if ((qx<x-radius && qx+dx<x-radius) ||
            (x+radius<qx && x+radius<qx+dx) ||
            (qz<z-radius && qz+dz<z-radius) ||
            (z+radius<qz && z+radius<qz+dz)) continue;
        length=(int32_t)func_80078004((uint32_t)floor_add(floor_mul(dx,dx),floor_mul(dz,dz)));
        distance=floor_abs(floor_div(floor_sub(floor_mul(az,dx),floor_mul(ax,dz)),length));
        if (radius<distance) continue;
        if (!PE_NormalizeVectorRetail(in,normal,&sum)) return -1;
        along=(int32_t)(func_8003708C((uint32_t)normal[0]<<4,(uint32_t)ax<<16)
                      +func_8003708C((uint32_t)normal[2]<<4,(uint32_t)az<<16));
        if (along<0) {
            along=floor_add(floor_mul(ax,ax),floor_mul(az,az));
            if (floor_mul(radius,radius)<along) continue;
        }
        if (length<(along>>16)) {
            ax=x-(qx+dx); az=z-(qz+dz);
            if (floor_mul(radius,radius)<floor_add(floor_mul(ax,ax),floor_mul(az,az))) continue;
        }
        return (int)i;
    }
    return -1;
}

/* 1CBA0: project onto the contacted edge, then search +x,-x,+z,-z
 * by increasing whole units until radius clearance is strict. */
static void floor_polygon_slide(pe_addr_t actor, pe_addr_t points, unsigned count, int edge)
{
    pe_addr_t point=points+(uint32_t)edge*8u;
    int32_t qx=floor_s16(point+2u), qz=floor_s16(point+6u);
    pe_addr_t previous=edge>0?point-8u:points+(count-1u)*8u;
    int32_t px=floor_s16(previous+2u), pz=floor_s16(previous+6u);
    int32_t in[3]={qx-px,0,qz-pz}, normal[3];
    uint32_t sum, tx, tz, along, base_x, base_z;
    int32_t dx=px-qx, dz=pz-qz, length, radius=PE_LoadU16(RADIUS);
    if (!PE_NormalizeVectorRetail(in,normal,&sum)) return;
    tx=(uint32_t)normal[0]<<4; tz=(uint32_t)normal[2]<<4;
    along=func_8003708C(tx,PE_LoadU32(actor+0x28u)-PE_LoadU32(actor+0x40u))
         +func_8003708C(tz,PE_LoadU32(actor+0x30u)-PE_LoadU32(actor+0x48u));
    base_x=PE_LoadU32(actor+0x40u)+func_8003708C(tx,along);
    base_z=PE_LoadU32(actor+0x48u)+func_8003708C(tz,along);
    length=(int32_t)func_80078004((uint32_t)floor_add(floor_mul(dx,dx),floor_mul(dz,dz)));
    for (unsigned step=0;;step++) {
        for (unsigned dir=0;dir<4u;dir++) {
            uint32_t x=base_x,z=base_z,offset=step<<16;
            if (dir==0) x+=offset;
            else if (dir==1) x-=offset;
            else if (dir==2) z+=offset;
            else z-=offset;
            int32_t distance=floor_abs(floor_div(floor_sub(
                floor_mul(((int32_t)z>>16)-qz,dx),
                floor_mul(((int32_t)x>>16)-qx,dz)),length));
            if (radius<distance) {
                PE_StoreU32(actor+0x28u,x);
                PE_StoreU32(actor+0x30u,z);
                return;
            }
        }
    }
}

/* Complete 1D170 call graph, enabled by D2E8 bit 4 at 1AA0C. */
static void floor_polygon_boundary(void)
{
    pe_addr_t actor=PE_LoadU32(AYA),points=PE_LoadU32(0x8009D2F8u);
    unsigned count=PE_LoadU16(0x8009D264u);
    int32_t radius=floor_mul(PE_LoadU16(actor+0x224u),PE_LoadU16(actor+0x26u));
    if (radius<0) radius=floor_add(radius,4095);
    PE_StoreU16(RADIUS,(uint16_t)(radius>>12));
    int edge=(int16_t)floor_polygon_contact(floor_s16(actor+0x2Au),floor_s16(actor+0x32u),points,count);
    if (edge<0) return;
    floor_polygon_slide(actor,points,count,edge);
    if ((int16_t)floor_polygon_contact(floor_s16(actor+0x2Au),floor_s16(actor+0x32u),points,count)>=0) {
        PE_StoreU32(actor+0x28u,PE_LoadU32(actor+0x40u));
        PE_StoreU32(actor+0x2Cu,PE_LoadU32(actor+0x44u));
        PE_StoreU32(actor+0x30u,PE_LoadU32(actor+0x48u));
    }
}

void func_8001A9F8_floor_cut(void)
{
    if (PE_LoadU32(INPUT)&4u) floor_polygon_boundary();
    pe_addr_t actor=PE_LoadU32(0x8009D20Cu);
    while (actor) {
        if (!(PE_LoadU32(actor+0x98u)&0x80u)) func_8001AE40(actor);
        actor=PE_LoadU32(actor+4u);
    }
}
