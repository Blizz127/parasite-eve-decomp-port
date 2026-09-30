/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m263/func_80193180.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `func_80193180` renamed to `func_80193180`, and vendor symbol
 * names mapped to this repo's address names from the vendor sym tables. */
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef float f32;
int func_80077DC4 (int angle);
int func_80077CF4 (int angle);
typedef struct RoomBlob4 {
    char b[4];
} RoomBlob4;
typedef struct RoomBounceFx {
    short x;
    short y;
    short z;
    short pad6;
    short vx;
    short vy;
    short vz;
    short ay;
    short state;
    short timer;
} RoomBounceFx;
extern RoomBlob4 D_8018F1DC ;
extern short D_800942EC ;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern void *D_800BCFA4;
int func_80077CF4(int angle);
int func_80077DC4(int angle);
int func_80077AA4(int width, int texture);
int func_800CEE20();
int func_800D2104();
int func_80193180  (int mode, RoomBounceFx *fx) {
    RoomBlob4 blob;
    int saved0;
    int saved2;
    blob = D_8018F1DC ;
    saved2 = (int)&blob;
    if (mode == 1) {
        goto mode1;
    }
    if (mode == 2) {
        goto mode2;
    }
    goto ret0;
mode1:
    if (fx->state == 0) {
        goto updateBounce;
    }
    if (fx->state == 1) {
        goto updateRise;
    }
    goto ret0;
updateBounce:
    {
        int vx;
        int vz;
        register int rawVy asm("$3");
        int nextVy;
        register int y asm("$2");
        fx->timer++;
        fx->x += fx->vx;
        fx->y += fx->vy;
        fx->z += fx->vz;
        vx = fx->vx * 511;
        if (vx < 0) {
            vx += 511;
        }
        fx->vx = vx >> 9;
        vz = fx->vz * 511;
        if (vz < 0) {
            vz += 511;
        }
        rawVy = (unsigned short)fx->vy;
        asm volatile("" : : "r"( rawVy )) ;
        fx->vz = vz >> 9;
        asm volatile("" : : : "memory") ;
        y = fx->y;
        asm volatile("" : : "r"( y )) ;
        nextVy = rawVy - 1 ;
        fx->vy = nextVy;
        asm volatile("" : : : "memory") ;
        if (y >= D_800942EC ) {
            int bounce;
            asm volatile("" : : : "memory") ;
            bounce = (nextVy << 16) >> 16;
            bounce = -bounce;
            fx->vy = bounce;
        }
        if (fx->timer < 24) {
            goto ret0;
        }
        return 1;
    }
updateRise:
    {
        fx->timer++;
        fx->y += fx->vy;
        fx->vy += fx->ay;
        asm volatile("" : : : "memory") ;
        if (fx->y >= D_800942EC ) {
            fx->y = D_800942EC ;
        }
        if (fx->timer < 32) {
            goto ret0;
        }
        return 1;
    }
mode2:
    if (fx->state == 0) {
        goto drawBounce;
    }
    if (fx->state == 1) {
        goto drawRise;
    }
    goto ret0;
drawBounce:
    {
        int sinValue;
        int scale;
        register int angle asm("$4");
        register int fade asm("$2");
        int texture;
        sinValue = func_80077CF4((fx->timer << 10) / 24);
        angle = (fx->timer * 1204) / 24;
        asm volatile("" : : "r"( angle )) ;
        scale = ((unsigned int)sinValue >> 31) + sinValue;
        scale >>= 1;
        saved2 = scale + 0x1000;
        fade = func_80077DC4(angle);
        if (fade < 0) {
            fade += 0x7F;
        }
        fade >>= 7;
        saved0 = fade + 0x28;
        texture = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            texture += 4;
        }
        texture = func_80077AA4(0x10 , texture);
        func_800CEE20(fx, 0, saved2, saved2,
                      D_800F336A * (fx->timer / 6) + 0xC8 ,
                      (unsigned short)texture, 1, saved0, 0);
        goto ret0;
    }
drawRise:
    {
        int angle;
        void **matrixSlot;
        angle = func_80077DC4(fx->timer << 5);
        if (angle < 0) {
            angle += 0x1F;
        }
        saved0 = angle >> 5;
        matrixSlot = &D_800BCFA4;
        asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( *matrixSlot ) : "$12", "$13", "$14") ;
        asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( *matrixSlot ) : "$12", "$13", "$14") ;
        func_800D2104(fx, (void *)saved2, saved0, 1);
    }
ret0:
    return 0;
}
