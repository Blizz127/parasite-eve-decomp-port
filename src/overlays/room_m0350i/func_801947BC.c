/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m350/RoomEffect_HomingTrailController.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_HomingTrailController` renamed to `func_801947BC`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
int func_80077DC4(int angle);
int func_80077CF4(int angle);
typedef struct { short x,y,z,pad; } Vector;
typedef struct { short rotation[3][3]; int position[3]; } Matrix;
typedef struct {
    Vector position,rotation;
    short speed,reserved12,brightness,frame;
    unsigned char hit,reserved19[3];
} Particle;
typedef struct { unsigned int flags; char reserved04[72]; unsigned int status; } Owner;
typedef struct {
    Owner *owner; char reserved04[10]; unsigned char animation;
    char reserved0F[0x1ED]; int position[3];
} Instance;
typedef struct { int reserved[2]; Instance *instance; } Actor;
extern Actor *D_800F32D0;
extern Instance *D_8009D254;
extern int D_800E27EC,D_800966EC[],D_800F3428;
extern short D_800966EE[],D_8019A7FE,D_8019A800,D_8019A802;
extern unsigned short D_800F336C,D_800E1204[],D_800942EC;
extern char D_8019A3C0[],D_8019A4E4[],D_8019A3C8[];
extern int D_800DFF80(void *,void *),D_800DFFB8(int,int,int);
extern int func_8005186C(int),func_80077AA4(int,int);
extern void func_80079754(Vector *,Matrix *);
extern void func_800CEE20(void *,int,int,int,int,int,int,int,int);
extern void func_800D004C(void *,int,int,int,void *,int,int,void *,void *,int,int);
int func_801947BC(int event,Particle *p)
{
    Matrix matrix;
    Vector offset,ground;
    char frameGap[16];
    if(event==1) {
        int timer=D_800E27EC;
        int brightness;
        if(timer>=48) {
            int result=1;
            unsigned short *count=(unsigned short *)&D_8019A802;
            *count=*count-1;
            return result;
        }
        if(p->frame>=48) return 0;
        if(!p->hit) {
            if(p->rotation.x>0) p->rotation.x-=64;
            else {
                int *trig=D_800966EC;
                int speed=*(short *)((char *)trig+((timer<<6)&0x3FC0))/4;
                int angle=D_800DFF80(D_8009D254->position,D_800F32D0->instance->position);
                int yaw=D_800DFFB8(p->rotation.y,(short)angle,speed);
                int sum,product;
                p->rotation.y=yaw;
                product=(short)trig[(D_800E27EC+2048)&4095]*160;
                sum=yaw+80;
                p->rotation.y=sum-product/4096;
            }
            func_80079754(&p->rotation,&matrix);
            matrix.position[0]=p->position.x;
            matrix.position[1]=p->position.y;
            matrix.position[2]=p->position.z;
            offset.x=0; offset.y=0; offset.z=0u-(unsigned short)p->speed;
            asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( &matrix ) : "$12", "$13", "$14") ; asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( &matrix ) : "$12", "$13", "$14") ;
            asm volatile("lwc2 $0,0(%0)"  "\n\t" "lwc2 $1,4(%0)"  : : "r"( &offset ) : "memory") ; asm volatile("nop\n\t" "nop\n\t" ".word 0x4A480012") ;
            {
                register int x asm("$12");
                register int y asm("$13");
                register int z asm("$14");
                asm volatile("mfc2 %0,$9" : "=r"( x )) ; asm volatile("mfc2 %0,$10" : "=r"( y )) ; asm volatile("mfc2 %0,$11" : "=r"( z )) ;
                p->position.x=x; p->position.y=y; p->position.z=z;
            }
        }
        if(p->frame<40) brightness=((short)*(int *)((char *)D_800966EC+((p->frame<<11)&0x3800))>>6)+128;
        else {
            unsigned int value=*(short *)((char *)D_800966EE+(((p->frame-40)<<9)&0x3E00));
            brightness=value>>5;
        }
        p->brightness=brightness;
        if(!p->hit && D_8009D254->animation>=4) {
            int dx=D_8009D254->position[0]-p->position.x;
            int dz=D_8009D254->position[2]-p->position.z;
            if(func_8005186C((unsigned int)dx*dx+(unsigned int)dz*dz)<128) {
                short *count;
                short n;
                p->hit=1;
                D_8009D254->owner->status|=0x4000;
                if(D_800F32D0->instance->owner) D_800F32D0->instance->owner->flags|=0x80000000;
                p->speed=0;
                if(p->frame<40) p->frame=40;
                count=&D_8019A7FE; n=*count;
                if(n<4) {
                    register int next asm("$4")=n;
                    asm volatile("" : "=r"(next) : "0"(next));
                    *count=next+1;
                    asm volatile("" : "=r"(count) : "0"(count), "r"(next) : "memory");
                    count-=0x2B;
                    ((Particle **)count)[n]=p;
                }
            }
        }
        if(p->frame>=36 && !(p->frame&3)) {
            short *count=&D_8019A7FE;
            short n=*count;
            if(n<4) {
                register int next asm("$4")=n;
                asm volatile("" : "=r"(next) : "0"(next));
                *count=next+1;
                asm volatile("" : "=r"(count) : "0"(count), "r"(next) : "memory");
                count-=0x2B;
                ((Particle **)count)[n]=p;
            }
        }
        {
            register short *count asm("$4")=&D_8019A800;
            unsigned short n=*(unsigned short *)count;
            int next=n+1;
            int shifted=(unsigned int)n<<16;
            *count=next;
            asm volatile("" : "=r"(count) : "0"(count) : "memory");
            count-=0x24;
            *(Vector *)((shifted>>13)+(unsigned int)count)=p->position;
        }
        p->frame=(unsigned short)p->frame+1;
    } else if(event==2) {
        int clut,palette;
        if(p->frame>=48) return 0;
        palette=D_800E1204[D_800F336C];
        if(D_800F336C==4 && D_800F3428) palette+=4;
        clut=func_80077AA4(32,palette);
        func_800CEE20(p,0,4096,4096,108,(unsigned short)clut,1,p->brightness,0);
        ground.x=p->position.x; ground.y=D_800942EC; ground.z=p->position.z;
        func_800D004C(&ground,192,192,8,D_8019A3C0,4096,4096,D_8019A4E4,D_8019A3C8,p->brightness,1);
    }
    return 0;
}
