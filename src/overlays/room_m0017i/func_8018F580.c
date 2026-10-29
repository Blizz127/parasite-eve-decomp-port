/* func_8018F580 — overlay room_m0017i (shared across room_m017/018/021/045/102/151/319
 * at the identical VRAM address per vendor sym tables), VRAM 0x8018F580, 0x5C4 bytes.
 * RoomFx_ModelDraw(unsigned char *geom): per-frame quad/tri/quad/tri submission of a room
 * model's 4 index tables, backface-culled via GTE nclip (MAC0 sign), depth-sorted into the
 * double-buffered OT via addPrim.
 * Profile era_o2_g0 (default, -O2 -G0).
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count separately
 * from plain C.
 * Vendor tree (khasinski) only declares this symbol (room_lib.h `RoomFx_ModelDraw`); no
 * vendor body exists to crib — written fresh from a full retail disassembly trace.
 * Levers: both table bases as function-scope locals (vt, zt: $s0/$t9 across
 *   all four loops); gte_nclip as the Psy-Q `nop; nop; .word` form; the OT slot
 *   passed straight into addPrim; the depth sum split from its shift
 *   (`otz = a+b+c+d; otz >>= 4;`, `otz = a+b+c; otz = otz / 3 >> 2;`);
 *   `for (...; i++, idx += 6)` so idx fills the lhu/slt gap and moves into the
 *   bnez delay slot; zt loaded after the ordering base in the prologue.
 * Hand-written in this repo by lane-f580 (lane notes: build/wt-lane-f580/build/drafts/f580lane/NOTES.md). */

typedef struct { unsigned addr : 24; unsigned len : 8; } P_TAG;
#define getaddr(p) (((P_TAG *)(p))->addr)
#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (_addr))
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, (unsigned int)(p))

typedef struct { char *ordering[2]; char *other_buffers[6]; char *packets[2]; } RenderBufferPrefix;
extern RenderBufferPrefix D_800B0E38;
extern int D_8009CDDC;
extern unsigned int D_800B1644[];
extern int D_800A636C[];

#define gte_mtc2_12(x) asm volatile("mtc2 %0,$12" : : "r"(x))
#define gte_mtc2_13(x) asm volatile("mtc2 %0,$13" : : "r"(x))
#define gte_mtc2_14(x) asm volatile("mtc2 %0,$14" : : "r"(x))
#define gte_nclip() asm volatile("nop\n\tnop\n\t.word 0x4B400006")
#define gte_stopz(r0) asm volatile("swc2 $24,0(%0)" : : "r"(r0) : "memory")

void func_8018F580(unsigned char *geom)
{
    register int *scratch asm("$19") = (int *)0x1F800000;
    unsigned short *idx;
    unsigned char *rec;
    int frame;
    unsigned int *base;
    int i;
    int i0, i1, i2, i3;
    unsigned int v0v, v1v, v2v;
    int clip;
    int otz;
    unsigned int *ot;
    unsigned int *rp;
    unsigned int *vt;
    int *zt;

    vt = D_800B1644;
    idx = *(unsigned short **)(geom + 0x10);
    rec = *(unsigned char **)(geom + 0x54);
    frame = D_8009CDDC;
    base = (unsigned int *)D_800B0E38.ordering[frame];
    zt = D_800A636C;

    for (i = 0; i < *(unsigned short *)(*(unsigned char **)geom + 0x8); i++, idx += 6) {
        i0 = idx[2];
        i1 = idx[3];
        i2 = idx[4];
        v0v = vt[i0];
        v1v = vt[i1];
        v2v = vt[i2];
        gte_mtc2_12(v0v);
        gte_mtc2_14(v2v);
        gte_mtc2_13(v1v);
        gte_nclip();
        rp = (unsigned int *)(rec + frame * 52);
        i3 = idx[5];
        gte_stopz(scratch);
        clip = *scratch;
        if (clip < 0) {
            otz = zt[i0] + zt[i1] + zt[i2] + zt[i3];
            otz >>= 4;
            if (otz < 0x1000) {
                addPrim(base + otz, rp);
                rp[2] = v0v;
                rp[5] = v1v;
                rp[8] = v2v;
                rp[0xB] = vt[i3];
            }
        } else {
            setaddr(rp, 0);
        }
        rec += 0x68;
    }

    for (i = 0; i < *(unsigned short *)(*(unsigned char **)geom + 0xA); i++, idx += 6) {
        i0 = idx[2];
        i1 = idx[3];
        i2 = idx[4];
        v0v = vt[i0];
        v1v = vt[i1];
        v2v = vt[i2];
        gte_mtc2_12(v0v);
        gte_mtc2_14(v2v);
        gte_mtc2_13(v1v);
        gte_nclip();
        rp = (unsigned int *)(rec + frame * 40);
        gte_stopz(scratch);
        clip = *scratch;
        if (clip < 0) {
            otz = zt[i0] + zt[i1] + zt[i2];
            otz = otz / 3 >> 2;
            if (otz < 0x1000) {
                addPrim(base + otz, rp);
                rp[2] = v0v;
                rp[5] = v1v;
                rp[8] = v2v;
            }
        } else {
            setaddr(rp, 0);
        }
        rec += 0x50;
    }

    for (i = 0; i < *(unsigned short *)(*(unsigned char **)geom + 0xC); i++, idx += 6) {
        i0 = idx[2];
        i1 = idx[3];
        i2 = idx[4];
        v0v = vt[i0];
        v1v = vt[i1];
        v2v = vt[i2];
        gte_mtc2_12(v0v);
        gte_mtc2_14(v2v);
        gte_mtc2_13(v1v);
        gte_nclip();
        rp = (unsigned int *)(rec + frame * 36);
        i3 = idx[5];
        gte_stopz(scratch);
        clip = *scratch;
        if (clip < 0) {
            otz = zt[i0] + zt[i1] + zt[i2] + zt[i3];
            otz >>= 4;
            if (otz < 0x1000) {
                addPrim(base + otz, rp);
                rp[2] = v0v;
                rp[4] = v1v;
                rp[6] = v2v;
                rp[8] = vt[i3];
            }
        } else {
            setaddr(rp, 0);
        }
        rec += 0x48;
    }

    for (i = 0; i < *(unsigned short *)(*(unsigned char **)geom + 0xE); i++, idx += 6) {
        i0 = idx[2];
        i1 = idx[3];
        i2 = idx[4];
        v0v = vt[i0];
        v1v = vt[i1];
        v2v = vt[i2];
        gte_mtc2_12(v0v);
        gte_mtc2_14(v2v);
        gte_mtc2_13(v1v);
        gte_nclip();
        rp = (unsigned int *)(rec + frame * 28);
        gte_stopz(scratch);
        clip = *scratch;
        if (clip < 0) {
            otz = zt[i0] + zt[i1] + zt[i2];
            otz = otz / 3 >> 2;
            if (otz < 0x1000) {
                addPrim(base + otz, rp);
                rp[2] = v0v;
                rp[4] = v1v;
                rp[6] = v2v;
            }
        } else {
            setaddr(rp, 0);
        }
        rec += 0x38;
    }
}
