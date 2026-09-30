/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m350/RoomEffect_FadingModel.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `func_801992A0` renamed to `func_801992A0`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
/* vendor markers: MASPSX_FLAGS: --expand-div */
/* Single-function extract: every other function definition of the vendor TU is reduced to a
 * prototype so this file compiles only `func_801992A0` (the TU spans several of this repo's yaml functions). */
extern int D_800F3428;
extern unsigned short D_800F336C, D_800E1204[];
extern short D_800F336A;
extern int func_80077AA4(int, int);
extern void func_800CEE20(void *, int, int, int, int, unsigned int, int, int, void *);
typedef struct {
    short x, y, z, unused;
    short velocity, acceleration, brightness, size, phase, counter;
} Particle;
int func_80199148(int event, Particle *object);
int func_80077DC4(int angle);
int func_80077CF4(int angle);
typedef struct { short x,y,z,pad; } Vector;
typedef struct { short rotation[3][3]; int position[3]; } Matrix;
typedef struct { short count,delay; } State;
typedef struct { int reserved[2]; int asset; } Owner;
typedef struct { unsigned char reserved[2],count; } Model;
typedef struct {
    Owner *owner; char reserved04[0x1B0]; Model *model;
    char reserved1B8[0x80]; Matrix *transforms;
} Instance;
typedef struct { int reserved[2]; Instance *instance; } Actor;
typedef struct { int reserved[2]; void *pool; } Emitter;
extern Actor *D_800F32D0;
extern Emitter *D_800F33E0;
extern unsigned char D_8019A8C4;
extern volatile short D_800F3368,D_800F3376,D_800F3378,D_800F336E,D_800F3372,D_800F3374;
extern volatile unsigned short D_800E11FA,D_800F3370;
extern unsigned short D_800E2850[];
extern int func_8006DCE4(int,int,int,int,int);
extern int func_80199148(int,Particle *);
extern int func_800CE560(void *,int,int,int (*)(int,Particle *));
extern Particle *func_800CE610(void *);
extern int func_80071A54(void),func_80052B2C(void),func_80077DC4(int),func_80077CF4(int);
int func_801992A0(int event,State *state)
{
    Vector offset;
    Matrix rotation;
    char frameGap[8];
    if(event==1) goto update;
    if(event<2) { if(event==0) goto setup; goto done; }
    if(event==2) goto configure;
    goto done;
setup:
    {
        Instance *instance=D_800F32D0->instance;
        Matrix *matrix=instance->transforms;
        func_8006DCE4(0x549,instance->owner->asset,(short)matrix->position[0],(short)matrix->position[1],(short)matrix->position[2]);
        state->count=0; state->delay=0; D_8019A8C4=0;
        return func_800CE560(D_800F33E0->pool,20,10,func_80199148);
    }
update:
    {
        Particle *particle;
        Actor *actor;
        Instance *instance;
        register int random asm("$7");
        int index;
        int halfSize;
        int cosine,divisor;
        register int size asm("$3");
        int sine;
        int product;
        if(D_8019A8C4) return 2;
        state->delay=(unsigned short)state->delay+1;
        if(state->delay<4) return 0;
        particle=func_800CE610(D_800F33E0->pool);
        if(!particle) return 0;
        func_80071A54();
        particle->brightness=64;
        {
            int count=state->count;
                        particle->size=((unsigned int)count<<7)+4096;
        }
        random=func_80052B2C();
        actor=D_800F32D0;
        instance=actor->instance;
        {
            register int low asm("$5")=random&255;
            register int parts asm("$3")=instance->model->count-1;
            register int scaled asm("$2")=low*parts;
                        scaled=scaled/256;
                        {
                register int selected asm("$3")=scaled+1;
                asm volatile("" : : "r"(selected));
                index=selected;
            }
        }
        {
        Instance *next;
        particle->x=instance->transforms[index].position[0];
                next=actor->instance;
        asm volatile("" : : "r"(next));
        size=particle->size;
        asm volatile("" : : "r"(size), "r"(next) : "memory");
        halfSize=(unsigned short)particle->size;
        particle->y=next->transforms[index].position[1];
        asm volatile("" : : : "$2");
        {
            register int low7 asm("$2")=random&127;
            product=low7*size;
        }
        next=actor->instance;
        asm volatile("" : : "r"(halfSize), "r"(next));
        halfSize=(unsigned int)halfSize<<16;
                {
        Matrix *transforms=next->transforms;
        asm volatile("" : "=r"(halfSize) : "0"(halfSize), "r"(transforms));
        halfSize=halfSize>>17;
        asm volatile("" : "=r"(transforms) : "0"(transforms), "r"(halfSize));
        particle->z=transforms[index].position[2];
        }
        }
        particle->velocity=0; particle->acceleration=0; particle->counter=0;
        {
            int y=(unsigned short)particle->y;
            asm volatile("" : "=l"(product) : "0"(product), "r"(y), "r"(halfSize));
            particle->y=y-(product>>13);
        }
        offset.x=0;
        random=func_80071A54(); divisor=96;
        offset.z=((random%divisor+64)*halfSize)>>12;
        cosine=func_80077DC4(func_80071A54());
        sine=func_80077CF4(func_80071A54());
        rotation.rotation[0][2]=sine;
        sine=0u-(unsigned int)sine;
        rotation.rotation[2][0]=sine;
        sine=4096;
        rotation.rotation[1][1]=sine;
                rotation.rotation[0][0]=cosine; rotation.rotation[2][2]=cosine;
        rotation.position[2]=0; rotation.position[1]=0; rotation.position[0]=0;
        rotation.rotation[2][1]=0; rotation.rotation[1][2]=0;
        rotation.rotation[1][0]=0; rotation.rotation[0][1]=0;
        {
            Matrix *matrix=&rotation;
            Vector *out;
            asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( matrix ) : "$12", "$13", "$14") ; asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( matrix ) : "$12", "$13", "$14") ;
            out=&offset;
            asm volatile("lwc2 $0,0(%0)"  "\n\t" "lwc2 $1,4(%0)"  : : "r"( out ) : "memory") ; asm volatile("nop\n\t" "nop\n\t" ".word 0x4A480012") ;
            asm volatile("" : "=r"(out) : "0"(out));
            {
                register int x asm("$12");
                register int y asm("$13");
                register int z asm("$14");
                asm volatile("mfc2 %0,$9" : "=r"( x )) ; asm volatile("mfc2 %0,$10" : "=r"( y )) ; asm volatile("mfc2 %0,$11" : "=r"( z )) ;
                out->x=x; out->y=y; out->z=z;
            }
        }
        particle->x=(unsigned short)particle->x+(unsigned short)offset.x;
        particle->z=(unsigned short)particle->z+(unsigned short)offset.z;
        state->delay=0; state->count=(unsigned short)state->count+1;
        func_8006DCE4(0x5FE,D_800F32D0->instance->owner->asset,particle->x,particle->y,particle->z);
    }
    goto done;
configure:
    {
        int index=D_800E11FA;
        int palette;
        D_800F3368=32; D_800F336A=2;
        D_800F3376=32; D_800F3378=32;
        palette=D_800E2850[index];
        asm volatile("" : "=r"(palette) : "0"(palette) : "memory");
        D_800F336C=3; D_800F336E=1;
        D_800F3372=0; D_800F3374=32; D_800F3370=palette;
    }
done:
    return 0;
}
