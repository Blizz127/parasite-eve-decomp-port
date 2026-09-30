/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m273/RoomEffect_ThresholdModelTriple.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `func_80193CB8` renamed to `func_80193CB8`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
/* Single-function extract: every other function definition of the vendor TU is reduced to a
 * prototype so this file compiles only `func_80193CB8` (the TU spans several of this repo's yaml functions). */
typedef struct { short x,y,z,phase,scaleXZ,scaleY; unsigned short parameter12,unknown14; } Effect;
typedef struct { short m[3][3],pad; int t[3]; } Matrix;
typedef struct { int x,y,z,pad; } Vector;
typedef struct { unsigned char unknown[0x594]; int x,y,z; } Transform;
typedef struct {
    unsigned char unknown[14],kind,unknown15[7];
    short value22; unsigned char unknown24[2]; unsigned short value26;
    unsigned char unknown28[0x21C]; Transform *transform;
} State;
typedef struct { unsigned char unknown[8]; State *state; } StateContext;
typedef struct { unsigned char unknown[8]; void *pool; } Context;
typedef struct { void *asset,*pool; } AssetState;
extern int D_800E27EC;
extern short D_8019AC18[];
extern int D_8019AC0C[];
extern int D_800F3428,D_800966EC[];
extern unsigned short D_800E11FA,D_800E2850[],D_800F336C,D_800E1204[];
extern Context *D_800F33E0;
extern StateContext *D_800F32D0;
extern AssetState D_8019AE8C;
extern void *D_8019AE90,*D_800B0E64;
extern unsigned char D_8019AE9A;
extern unsigned short D_800942EC;
extern int func_80077A64(int,int,int,int),func_80077DC4(int),func_80077CF4(int);
extern unsigned short func_80077AA4(int,int);
extern void func_800C6EC0(int,int),func_800C6ED8(int);
extern void func_800C6EF8(void *),func_800C6FA0(void *,int);
extern void *func_80071A44(void *,int,unsigned long);
extern void func_80078CC4(Matrix *,Vector *),func_800C71E4(void *,Matrix *),func_800C6F4C(void *);
extern int func_800D0728(void *obj, short a, short b, int c, short *params, int x0, int x1, int *p0, int *p1, short y, int one);
extern void *func_8006E498(void *,unsigned int);
extern void func_800C6D5C(void *,int,int);
extern int func_800CE560(void *,int,int,int (*)());
extern int func_800CE5AC(void **,int,int,int,int (*)());
extern void *func_800CE610(void *);
extern void func_800CE688(void *),func_800CE78C(void *);
extern int func_80193CB8(),func_80193B5C();
int func_80193B5C(int mode, void **arg1);
int func_80193CB8(int mode,Effect *effect) {
    Matrix matrix;
    Vector copy,scale;
    if(mode==1) {
        int frame=D_800E27EC;
        int *wave;
        register int *shifted asm("$4");
        register int *table asm("$6");
        unsigned int index;
        if(frame>=32) return 1;
        frame*=32;
        index=frame&0xFFF;
        asm("" : "=r"(index) : "0"(index) : "$6");
        table=D_800966EC;
        wave=&table[index];
        shifted=&table[(frame+1024)&0xFFF];
        {
            register int sample asm("$2")=*wave;
            effect->scaleXZ=(short)sample/2;
        }
        effect->scaleY=(((short *)shifted)[1]+4096)*2;
        asm("" : : : "memory");
        {
            unsigned int phase=(unsigned short)effect->phase;
            int parameter=((short *)wave)[1];
            asm("" : "=r"(phase), "=r"(parameter) : "0"(phase), "1"(parameter));
            effect->phase=phase+256;
            effect->parameter12=parameter>>6;
        }
    } else if(mode==2) {
        int flags=func_80077A64(0,1,0,0);
        int page=(unsigned short)(D_800E2850[D_800E11FA]|flags);
        int kind=D_800F336C,palette;
        int cosine,sine;
        register void **asset asm("$17");
        register int specialKind asm("$3")=4;
        asm("" : : "r"(page) : "memory");
        palette=D_800E1204[kind];
        if(kind==specialKind && D_800F3428) palette+=11;
        else palette+=7;
        func_800C6EC0(page,func_80077AA4(0,palette));
        func_800C6ED8(1);
        asset=&D_8019AE8C.asset;
        func_800C6EF8(*asset);
        func_800C6FA0(*asset,effect->parameter12);
        cosine=func_80077DC4(effect->phase);
        sine=func_80077CF4(effect->phase);
        matrix.m[0][2]=sine; matrix.m[2][0]=-sine;
        matrix.m[0][0]=cosine; matrix.m[2][2]=cosine;
        matrix.t[0]=matrix.t[1]=matrix.t[2]=0;
        matrix.m[0][1]=matrix.m[1][0]=matrix.m[1][2]=matrix.m[2][1]=0;
        matrix.m[1][1]=4096;
        func_80071A44(&scale,0,16);
        scale.x=effect->scaleXZ; scale.y=effect->scaleY; scale.z=effect->scaleXZ;
        copy=scale;
        func_80078CC4(&matrix,&copy);
        {
            int x=effect->x;
            void *model=*asset;
            matrix.t[0]=x; matrix.t[1]=effect->y; matrix.t[2]=effect->z;
            func_800C71E4(model,&matrix);
        }
        func_800C6F4C(*asset);
    }
    return 0;
}
int func_80193F30(int mode);
