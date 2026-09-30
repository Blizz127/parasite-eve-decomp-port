/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m350/RoomEffect_SequenceController.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_SequenceController` renamed to `func_801958DC`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
int func_80077DC4(int angle);
int func_80077CF4(int angle);
typedef struct { short x,y,z,pad; } Vector;
typedef struct { short rotation[3][3]; int position[3]; } Matrix;
typedef struct { unsigned char state; } Action;
typedef struct { unsigned int flags; char reserved04[20]; Action *action; } Owner;
typedef struct {
    Owner *owner; char reserved04[10]; unsigned char animation,length;
    char reserved10[4]; unsigned int frame;
    char reserved18[34]; short yaw;
    char reserved3C[0x1C0]; Vector position;
    char reserved204[0x34]; Matrix *transforms;
} Instance;
typedef struct { int reserved[2]; Instance *instance; } Actor;
extern Actor *D_800F32D0;
extern Instance *D_8019A7F8,*D_8009D254;
extern Vector D_8019A7A0;
extern short D_8019A7FC;
extern volatile short D_8019A7FE,D_8019A800,D_8019A802;
extern volatile unsigned char D_8019A804,D_8019A805,D_8019A806;
extern int D_800966EC[];
extern int D_800DFF80(Vector *,Vector *);
extern int D_800DFFB8(int,int,int);
int func_801958DC(int event) {
    Instance *instance=D_800F32D0->instance;
    Vector offset;
    if(event==0) {
        register int one asm("$2")=1;
        {
            Owner *owner=instance->owner;
            register unsigned int flags asm("$2")=owner->flags;
            flags |= 0x40000000;
            owner->flags=flags;
        }
        asm volatile("" : : : "memory");
        one=2;
        D_8019A7FC=one;
        D_8019A804=0; D_8019A805=0; D_8019A806=0;
        D_8019A7FE=0; D_8019A800=0; D_8019A802=0;
        asm volatile("" : : : "$2");
        return 0;
    }
    if(event==1) {
        Matrix *matrix;
        int frame;
        int repetitions;
        short *repeat;
        if(instance->owner && instance->owner->action->state==1)
            instance->owner->action->state=2;
        matrix=&D_8019A7F8->transforms[17];
        offset.x=0; offset.y=-144; offset.z=128;
        asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( matrix ) : "$12", "$13", "$14") ;
        asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( matrix ) : "$12", "$13", "$14") ;
        asm volatile("lwc2 $0,0(%0)"  "\n\t" "lwc2 $1,4(%0)"  : : "r"( &offset ) : "memory") ;
        asm volatile("nop\n\t" "nop\n\t" ".word 0x4A480012") ;
        {
            register int x asm("$12");
            register int y asm("$13");
            register int z asm("$14");
            register Vector *out asm("$7")=&D_8019A7A0;
            asm volatile("" : "=r"(out) : "0"(out));
            asm volatile("mfc2 %0,$9" : "=r"( x )) ; asm volatile("mfc2 %0,$10" : "=r"( y )) ; asm volatile("mfc2 %0,$11" : "=r"( z )) ;
            out->x=x; out->y=y; out->z=z;
        }
        if(instance->animation!=13) return 0;
        repeat=&D_8019A7FC;
        repetitions=*repeat;
        asm("" : : "r"(repetitions));
        frame=*(unsigned short *)((char *)&instance->frame+2);
        if(repetitions>0) {
            register int speed asm("$16");
            int angle;
            frame=(short)frame;
            if(frame>=25) {
                instance->frame=0x60000;
                *repeat=*(volatile unsigned short *)repeat-1;
            }
            if(frame<19) return 0;
            speed=*(short *)&D_800966EC[((frame-19)*341)&4095];
            speed/=32;
            angle=D_800DFF80(&D_8009D254->position,&instance->position);
            instance->yaw=D_800DFFB8(instance->yaw,(short)angle,speed);
        } else {
            if((short)frame>=instance->length-1) {
                if(instance->owner) instance->owner->action->state=4;
                D_8019A804=1;
                return 1;
            }
        }
    }
    return 0;
}
