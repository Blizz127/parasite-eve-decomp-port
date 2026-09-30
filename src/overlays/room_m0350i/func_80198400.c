/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m350/RoomEffect_SweptAreaEmitter.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_SweptAreaEmitter` renamed to `func_80198400`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
int func_80077DC4(int angle);
int func_80077CF4(int angle);
typedef struct { short x,y,z,pad; } Vector;
typedef struct { short rotation[3][3]; int position[3]; } Matrix;
typedef struct { short y,z,shade; } Particle;
typedef struct { unsigned int flags; char reserved04[0x48]; unsigned int status; } Owner;
typedef struct {
    Owner *owner;
    char reserved04[10];
    unsigned char animation;
    char reserved0F[7];
    unsigned short frame;
    char reserved18[0x1D0];
    Matrix transform;
    char reserved208[0x30];
    Matrix *transforms;
} Instance;
typedef struct { int reserved[2]; Instance *instance; } Actor;
typedef struct { int reserved[2]; void *pool; } Emitter;
extern Actor *D_800F32D0;
extern Emitter *D_800F33E0;
extern Instance *D_8009D254;
extern int D_800E27EC;
extern unsigned char D_8019A8C0;
extern short D_8019A898[4],D_8019A8A0[4];
extern volatile short rightX asm("D_8019A898");
extern volatile short leftX asm("D_8019A8A0");
extern Vector D_8019A8A8,D_8019A8B0;
extern volatile short D_8019A89C,D_8019A89E,D_8019A8A4;
extern short D_8019A8AE;
extern short D_8019A890[];
extern short D_800F3368,D_800F336A,D_800F3376,D_800F3378;
extern short D_800F336C,D_800F336E,D_800F3372,D_800F3374,D_800F3370;
extern unsigned short D_800E11EA,D_800E2850[];
extern int func_801981D0(int,Particle *);
extern int func_800CE560(void *,int,int,int (*)(int,Particle *));
extern Particle *func_800CE610(void *);
extern int func_80052B2C(void);
extern void func_80078C34(Matrix *,Vector *,Vector *);
int func_80198400(int event) {
    Vector offset,vertices[4];
    int area;
    Instance *instance=D_800F32D0->instance;
    if(event==1) goto update;
    if(event<2) {
        if(event==0) goto setup;
        goto done;
    }
    if(event==2) goto configure;
    goto done;
setup:
    return func_800CE560(D_800F33E0->pool,8,4,func_801981D0);
update:
    if(instance->frame>=31) return 2;
    if(!(D_800E27EC&1)) {
        Particle *particle=func_800CE610(D_800F33E0->pool);
        if(particle) {
            int random=func_80052B2C()&255;
            particle->y=random;
            particle->z=(random>>2)+64;
        }
    }
    if(D_8019A8C0 || instance->animation!=9) return 0;
    {
        register Vector *from asm("$5")=(Vector *)&D_8019A898;
        register Vector *to asm("$4")=&D_8019A8A8;
        asm("" : "=r"(from),"=r"(to) : "0"(from),"1"(to));
        *to=*from;
    }
    {
        register Vector *from asm("$5")=(Vector *)&D_8019A8A0;
        register Vector *to asm("$4")=&D_8019A8B0;
        asm("" : "=r"(from),"=r"(to) : "0"(from),"1"(to));
        *to=*from;
    }
    offset.x=256; offset.y=0; offset.z=0;
    func_80078C34(&instance->transform,&offset,&offset);
    {
        int positionX=instance->transform.position[0];
        unsigned short x;
        unsigned short z;
        int positionZ;
        x=offset.x;
        z=offset.z;
        rightX=x+positionX;
        positionZ=((volatile Instance *)instance)->transform.position[2];
        D_8019A89E=1;
        positionZ=z+positionZ;
        D_8019A89C=positionZ;
        leftX=((volatile Instance *)instance)->transform.position[0]-x;
        D_8019A8A4=((volatile Instance *)instance)->transform.position[2]-z;
    }
    asm volatile("" ::: "memory");
    vertices[0]=*(Vector *)&D_8019A898;
    vertices[1]=*(Vector *)&D_8019A8A0;
    vertices[2]=D_8019A8B0;
    {
        int valid=D_8019A8AE;
        vertices[3]=D_8019A8A8;
        if(!valid) return 0;
        {
            int i=0;
            int previous=((unsigned int)vertices[3].z<<16)|(unsigned short)vertices[3].x;
            int point=((unsigned int)D_8009D254->transform.position[2]<<16)|(unsigned short)D_8009D254->transform.position[0];
            int *out=&area;
            for(;i<4;) {
                int current=((unsigned int)vertices[i].z<<16)|(unsigned short)vertices[i].x;
                asm volatile("mtc2 %0,$12" : : "r"( point )) ; asm volatile("mtc2 %0,$14" : : "r"( previous )) ; asm volatile("mtc2 %0,$13" : : "r"( current )) ;
                asm volatile("nop\n\t" "nop\n\t" ".word 0x4B400006") ; asm volatile("swc2 $24,0(%0)" : : "r"( out ) : "memory") ;
                if(area<0) break;
                i++;
                previous=current;
            }
            if(i<4) return 0;
            {
                Instance *player=D_8009D254;
                asm("" : "=r"(player) : "0"(player));
                D_8019A8C0=1;
                asm volatile("" ::: "memory");
                player->owner->status|=0x4000;
                if(instance->owner) instance->owner->flags|=0x80000000u;
            }
        }
    }
    goto done;
configure:
    {
        short i=0;
        int index,color;
        short *destination=D_8019A890;
        Matrix *transforms;
        transforms=instance->transforms;
        for(;i<3;i++) destination[i]=(transforms[11].position[i]+transforms[15].position[i])>>1;
        index=D_800E11EA;
        D_800F3368=32; D_800F336A=2;
        D_800F3376=32; D_800F3378=32;
        D_800F3376=32; D_800F3378=32;
        color=D_800E2850[index];
        D_800F336C=3; D_800F336E=0;
        D_800F3372=0; D_800F3374=0;
        D_800F3370=color;
    }
done:
    return 0;
}
