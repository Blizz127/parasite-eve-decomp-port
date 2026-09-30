/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m273/RoomEffect_DirectedRings.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_DirectedRings` renamed to `func_801969D8`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
int func_80077DC4(int angle);
int func_80077CF4(int angle);
typedef struct { short x,y,z,w; } Vector;
typedef struct { short m[3][3]; int t[3]; } Matrix;
typedef struct { Vector position,rotation,velocity; } Particle;
typedef struct { unsigned char unknown[8]; void *pool; } Context;
typedef struct { unsigned char unknown[0x2A]; short x,unknown2C,y,unknown30,z; } Player;
typedef struct { unsigned char unknown[8]; void *value; } Owner;
typedef struct { Owner *owner; } State;
typedef struct { unsigned char unknown[8]; State *state; } StateContext;
extern Context *D_800F33E0;
extern StateContext *D_800F32D0;
extern Player *D_8009D254;
extern unsigned char D_8019AF69,D_8019AF68;
extern short D_8019AEFC,D_8019AEFE,D_8019AF00;
extern unsigned short D_8019AF60;
extern Vector D_8019AD60;
extern int D_800966EC[];
extern unsigned short D_800E11E8,D_800E2850[];
extern unsigned short D_800F3368,D_800F336A,D_800F336C,D_800F336E;
extern unsigned short D_800F3370,D_800F3372,D_800F3374;
extern volatile unsigned short D_800F3376,D_800F3378;
extern int func_8019665C();
extern int func_800CE560(void *,int,int,int (*)());
extern Particle *func_800CE610(void *);
extern int func_8005186C(int),func_80079FB4(int,int);
extern void func_80079754(Vector *,Matrix *);
extern void func_8006DCE4(int,void *,int,int,int);
int func_801969D8(int mode) {
    Vector playerPosition;
    Matrix matrix;
    switch(mode) {
    case 0:
        return func_800CE560(D_800F33E0->pool,24,40,func_8019665C);
    case 1: {
        register unsigned char *stopped asm("$17")=&D_8019AF69;
        Player *player;
        int dx,dy,dz,x,y,z; short spread; short angle;
        if(*stopped) return 2;
        player=D_8009D254;
        playerPosition.x=x=player->x;
        dx=x-D_8019AEFC;
        playerPosition.y=y=player->y;
        playerPosition.z=z=player->z;
        dz=z-D_8019AF00;
        dy=y-D_8019AEFE;
        angle=func_80079FB4(dy,func_8005186C((unsigned int)dx*dx+(unsigned int)dz*dz))&4095;
        if(angle>768) angle=768;
        if(angle<384) angle=384;
        spread=angle-256;
        if(D_8019AF68) {
            short i=0;
            int width=(short)spread;
            register unsigned short *heading asm("$19")=(unsigned short *)(stopped-9);
            register Matrix *transform asm("$16")=&matrix;
            matrix.t[0]=0; matrix.t[1]=0; matrix.t[2]=0;
            do {
                Particle *particle=func_800CE610(D_800F33E0->pool);
                short *wave;
                if(!particle) break;
                wave=(short *)&D_800966EC[(i*4096/6)&4095];
                particle->rotation.x=angle+wave[1]*160/4096;
                particle->rotation.y=wave[0]*width/4096+(*heading+2048);
                particle->rotation.z=0; particle->rotation.w=1;
                func_80079754(&particle->rotation,transform);
                {
                    register int m0 asm("$12"), m1 asm("$13"), m2 asm("$14");
                    m0 = ((int *)transform)[0];
                    m1 = ((int *)transform)[1];
                    asm volatile("ctc2 %0,$0" : : "r"( m0 )) ;
                    asm volatile("ctc2 %0,$1" : : "r"( m1 )) ;
                    m0 = ((int *)transform)[2];
                    m1 = ((int *)transform)[3];
                    m2 = ((int *)transform)[4];
                    asm volatile("ctc2 %0,$2" : : "r"( m0 )) ;
                    asm volatile("ctc2 %0,$3" : : "r"( m1 )) ;
                    asm volatile("ctc2 %0,$4" : : "r"( m2 )) ;
                    m0 = ((int *)transform)[5];
                    m1 = ((int *)transform)[6];
                    asm volatile("ctc2 %0,$5" : : "r"( m0 )) ;
                    m2 = ((int *)transform)[7];
                    asm volatile("ctc2 %0,$6" : : "r"( m1 )) ;
                    asm volatile("ctc2 %0,$7" : : "r"( m2 )) ;
                }
                { Vector *input=&D_8019AD60; asm volatile("lwc2 $0,0(%0)"  "\n\t" "lwc2 $1,4(%0)"  : : "r"( input ) : "memory") ; }
                asm volatile("nop") ;
                asm volatile("nop") ;
                asm volatile(".word 0x4A480012") ;
                {
                    register Vector *out=&particle->velocity;
                    register int x asm("$12"),y asm("$13"),z asm("$14");
                    asm("" : "=r"(out) : "0"(out));
                    asm volatile("mfc2 %0,$9" : "=r"( x )) ; asm volatile("mfc2 %0,$10" : "=r"( y )) ; asm volatile("mfc2 %0,$11" : "=r"( z )) ;
                    out->x=x; out->y=y; out->z=z;
                }
                particle->position.x=heading[-50];
                particle->position.y=heading[-49];
                particle->position.z=heading[-48];
                particle->position.w=0; particle->velocity.w=0;
                i++;
            } while(i<6);
            i=0; width=(short)spread/2; heading=&D_8019AF60;
            do {
                Particle *particle=func_800CE610(D_800F33E0->pool);
                short *wave;
                if(!particle) break;
                {
                    register int *table asm("$3")=D_800966EC;
                    wave=(short *)&table[((unsigned int)i<<10)&3072];
                }
                particle->rotation.x=angle+wave[1]*96/4096;
                particle->rotation.y=wave[0]*width/4096+(*heading+2048);
                particle->rotation.z=0; particle->rotation.w=1;
                asm("" : : "r"(&particle->rotation));
                transform=&matrix;
                asm("" : "=r"(transform) : "0"(transform));
                func_80079754(&particle->rotation,transform);
                {
                    register int m0 asm("$12"), m1 asm("$13"), m2 asm("$14");
                    m0 = ((int *)transform)[0];
                    m1 = ((int *)transform)[1];
                    asm volatile("ctc2 %0,$0" : : "r"( m0 )) ;
                    asm volatile("ctc2 %0,$1" : : "r"( m1 )) ;
                    m0 = ((int *)transform)[2];
                    m1 = ((int *)transform)[3];
                    m2 = ((int *)transform)[4];
                    asm volatile("ctc2 %0,$2" : : "r"( m0 )) ;
                    asm volatile("ctc2 %0,$3" : : "r"( m1 )) ;
                    asm volatile("ctc2 %0,$4" : : "r"( m2 )) ;
                    m0 = ((int *)transform)[5];
                    m1 = ((int *)transform)[6];
                    asm volatile("ctc2 %0,$5" : : "r"( m0 )) ;
                    m2 = ((int *)transform)[7];
                    asm volatile("ctc2 %0,$6" : : "r"( m1 )) ;
                    asm volatile("ctc2 %0,$7" : : "r"( m2 )) ;
                }
                { Vector *input=&D_8019AD60; asm volatile("lwc2 $0,0(%0)"  "\n\t" "lwc2 $1,4(%0)"  : : "r"( input ) : "memory") ; }
                asm volatile("nop") ;
                asm volatile("nop") ;
                asm volatile(".word 0x4A480012") ;
                {
                    register Vector *out=&particle->velocity;
                    register int x asm("$12"),y asm("$13"),z asm("$14");
                    asm("" : "=r"(out) : "0"(out));
                    asm volatile("mfc2 %0,$9" : "=r"( x )) ; asm volatile("mfc2 %0,$10" : "=r"( y )) ; asm volatile("mfc2 %0,$11" : "=r"( z )) ;
                    out->x=x; out->y=y; out->z=z;
                }
                particle->position.x=heading[-50];
                particle->position.y=heading[-49];
                particle->position.z=heading[-48];
                particle->position.w=0; particle->velocity.w=0;
                i++;
            } while(i<4);
            func_8006DCE4(0x5D1,D_800F32D0->state->owner->value,D_8019AEFC,D_8019AEFE,D_8019AF00);
        }
        break;
    }
    case 2: {
        unsigned int index=D_800E11E8;
        unsigned short palette;
        D_800F3368=16; D_800F336A=1;
        D_800F3376=16; D_800F3378=16;
        D_800F3376=16; D_800F3378=16;
        palette=D_800E2850[index];
        D_800F336C=2; D_800F336E=0; D_800F3372=0; D_800F3374=0;
        D_800F3370=palette;
        break;
    }
    }
    return 0;
}
