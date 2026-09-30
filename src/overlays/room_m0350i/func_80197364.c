/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m350/RoomEffect_AimedImpactController.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_AimedImpactController` renamed to `func_80197364`, and vendor symbol
 * names mapped to this repo's address names from the vendor sym tables. */
typedef struct { short x, y, z, pad; } Vector;
typedef struct { short rotation[3][3]; int translation[3]; } Matrix;
typedef struct { Vector velocity; short x, y, z; unsigned char stopped, pad; } Particle;
typedef struct { int reserved[2]; void *pool; } Emitter;
typedef struct { int reserved[2]; int soundMode; } Owner;
typedef struct { char reserved[0xF4]; int position[3]; } Transform;
typedef struct {
    Owner *owner;
    char reserved04[0x3A - sizeof(Owner *)];
    unsigned short yaw;
    char reserved3C[0x1FC];
    Transform *transform;
} Instance;
typedef struct { int reserved[2]; Instance *instance; } Actor;
typedef struct { char reserved[0x1FC]; int position[3]; } Player;
extern Emitter *D_800F33E0;
extern Actor *D_800F32D0;
extern Player * D_8009D254 ;
extern unsigned char D_8019A86E, D_8019A855;
extern short D_8019A86A;
extern int func_80196F2C(int, Particle *);
extern int func_800CE560(void *, int, int, int (*)(int, Particle *));
extern Particle *func_800CE610(void *);
extern int func_8005186C (int);
extern int func_80079FB4 (int, int);
extern int func_80052B2C (void);
extern Matrix * func_80079754 (Vector *, Matrix *);
extern Vector * func_80078C34 (Matrix *, Vector *, Vector *);
extern int func_8006DCE4 (int, int, short, short, short);
int func_80197364(int event)
{
    Matrix matrix;
    Vector vector;
    if (event == 1) goto update;
    if (event < 2) {
        if (event == 0) goto setup;
        goto done;
    }
    goto done;
setup:
    return func_800CE560(D_800F33E0->pool, 16, 4, func_80196F2C);
update:
    {
        Particle *effect;
        Instance *instance;
        Transform *transform;
        Player *player;
        int dx, dz, dy;
        int angle;
        if (D_8019A86E) return 2;
        if (!D_8019A855) return 0;
        effect = func_800CE610(D_800F33E0->pool);
        if (!effect) return 0;
        instance = D_800F32D0->instance;
        transform = instance->transform;
        player = D_8009D254 ;
        effect->x = transform->position[0];
        effect->y = transform->position[1];
        effect->z = transform->position[2];
        effect->stopped = 0;
        dx = transform->position[0] - player->position[0];
        dz = transform->position[2] - player->position[2];
        dy = transform->position[1] - player->position[1];
        angle = func_80079FB4 (dy, func_8005186C (dx * dx + dz * dz));
        angle += (signed char)func_80052B2C () / 2;
        angle &= 0xFFF;
        vector.x = angle;
        if (angle < 0xD00) vector.x = 0xD00;
        {
            int offset = (signed char)func_80052B2C ();
            Vector *v = &vector;
            register Vector *input asm("$4") = v;
            register Matrix *m asm("$5") = &matrix;
            int yaw = instance->yaw;
            asm("" : "=r"(offset) : "0"(offset), "r"(input), "r"(m), "r"(yaw));
            offset += 0x800;
            yaw += offset;
            vector.y = yaw;
            vector.z = 0;
            func_80079754 (input, m);
        }
        vector.x = 0;
        vector.y = 0;
        vector.z = 128;
        func_80078C34 (&matrix, &vector, &effect->velocity);
        {
            Actor *actor = D_800F32D0;
            D_8019A855 = 0;
            asm volatile("" : : : "memory");
            {
                int x = effect->x;
                func_8006DCE4 (0x60A, actor->instance->owner->soundMode,
                    x, effect->y, effect->z);
            }
        }
        D_8019A86A = 8;
    }
done:
    return 0;
}
