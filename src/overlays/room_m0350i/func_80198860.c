/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m350/RoomEffect_AttachedCloud.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_AttachedCloud` renamed to `func_80198860`, and vendor symbol
 * names mapped to this repo's address names from the vendor sym tables. */
typedef struct { short x,y,z,pad; } Vector;
typedef struct { signed int low:16; signed int high:16; } TrigEntry;
typedef struct {
    int *anchor;
    short position[3],size,animatedSize,firstShade,secondShade,reserved;
} Particle;
extern int D_800E27EC,D_800F3428;
extern short D_800966EC[],D_800966EE[],D_800F336A;
extern unsigned short D_800F336C,D_800E1204[];
extern int D_8019A3D0[];
extern unsigned short func_80077AA4 (int,int);
extern void func_800CEE20(void *,int,int,int,int,int,int,int,void *);
int func_80198860(int event,Particle *particle)
{
    if(event==1) {
        int timer=D_800E27EC;
        int shade;
        int size;
        register int product asm("$2");
        if(timer>=8) return 1;
        shade=((TrigEntry *)((char *)D_800966EE-2+(((unsigned int)timer*3<<8)&0x3F00)))->high >> 5;
        particle->secondShade=128-((unsigned int)timer<<4);
        size=particle->size;
        particle->firstShade=shade;
        product=*(short *)((char *)D_800966EC+(((unsigned int)timer<<9)&0x3E00))*size;
        {
            int twice=(unsigned int)product<<1;
            product=(unsigned int)twice+(unsigned int)product;
        }
        particle->animatedSize=product/4096;
    } else if(event==2) {
        Vector world;
        char frameGap[8];
        register int kind asm("$2");
        int palette;
        unsigned short clut;
        world.x=(unsigned int)particle->anchor[0]+(unsigned short)particle->position[0];
        world.y=(unsigned int)particle->anchor[1]+(unsigned short)particle->position[1];
        world.z=(unsigned int)particle->anchor[2]+(unsigned short)particle->position[2];
        if(particle->firstShade>0) {
            kind=D_800F336C; palette=D_800E1204[kind];
            if(kind==4 && D_800F3428) palette+=4;
            clut= func_80077AA4 (32,palette);
            func_800CEE20(&world,0,particle->size,particle->size,D_800F336A*2+216,clut,1,particle->firstShade,D_8019A3D0);
        }
        kind=D_800F336C; palette=D_800E1204[kind];
        if(kind==4 && D_800F3428) palette+=4;
        clut= func_80077AA4 (32,palette);
        func_800CEE20(&world,0,particle->animatedSize,particle->animatedSize,D_800F336A*2+216,clut,1,particle->secondShade,D_8019A3D0);
    }
    return 0;
}
