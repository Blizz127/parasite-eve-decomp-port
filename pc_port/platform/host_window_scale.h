#ifndef HOST_WINDOW_SCALE_H
#define HOST_WINDOW_SCALE_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Letterbox rectangle: the largest aspect-preserving fit of fb_w x fb_h
 * inside width x height, centred. */
static void HostWindow_FitRect(int width, int height, int fb_w, int fb_h,
                               int *x0, int *y0, int *w, int *h)
{
    *w=width; *h=height;
    if ((int64_t)width*fb_h>(int64_t)height*fb_w)
        *w=(int)((int64_t)height*fb_w/fb_h);
    else *h=(int)((int64_t)width*fb_h/fb_w);
    *x0=(width-*w)/2; *y0=(height-*h)/2;
}

/* Nearest-neighbour sampling shared by the CPU scaler and the X11 XRender
 * path, so both put identical pixels on screen.  The step is the 16.16
 * fixed-point source/destination ratio that XRender is given as its
 * picture transform; source index i is what pixman's NEAREST filter picks
 * for destination pixel i: sample at the pixel centre (i + 0.5) through
 * the transform (rounded to 16.16), minus pixman_fixed_e, truncated. */
static int32_t HostWindow_NearestStep(int src, int dst)
{
    return (int32_t)(((int64_t)src << 16) / (dst > 0 ? dst : 1));
}
static int HostWindow_NearestIndex(int i, int32_t step)
{
    int64_t centre = ((int64_t)step * 0x8000 + 0x8000) >> 16;
    return (int)(((int64_t)step * i + centre - 1) >> 16);
}

/* Fit the complete game image inside the current client area, preserving
 * its aspect ratio at arbitrary window sizes (including downscaling).
 * Nearest neighbour (HostWindow_NearestIndex); everything outside the
 * fitted rectangle is black.  The column map is computed once per call
 * and a destination row whose source row equals the previous one is
 * copied, so the cost is one pass over the output (no per-pixel division, no double write of the bars). */
static void HostWindow_ScaleBGRA(uint8_t *out, int width, int height,
                                 const uint8_t *rgb, int fb_w, int fb_h)
{
    static int *xmap; static int xmap_cap;
    int w,h,x0,y0,x,y,prev=-1;
    int32_t xstep,ystep;
    size_t row_bytes=(size_t)width*4u;
    HostWindow_FitRect(width,height,fb_w,fb_h,&x0,&y0,&w,&h);
    if (w>xmap_cap) {
        int *p=(int *)realloc(xmap,(size_t)w*sizeof(*p));
        if (!p) { memset(out,0,row_bytes*(size_t)height); return; }
        xmap=p; xmap_cap=w;
    }
    xstep=HostWindow_NearestStep(fb_w,w); ystep=HostWindow_NearestStep(fb_h,h);
    for(x=0;x<w;x++) xmap[x]=HostWindow_NearestIndex(x,xstep)*3;
    if (y0>0) memset(out,0,row_bytes*(size_t)y0);
    if (y0+h<height) memset(out+row_bytes*(size_t)(y0+h),0,row_bytes*(size_t)(height-y0-h));
    for(y=0;y<h;y++) {
        int sy=HostWindow_NearestIndex(y,ystep);
        uint8_t *line=out+row_bytes*(size_t)(y+y0);
        uint8_t *dst=line+(size_t)x0*4u;
        if (x0>0) memset(line,0,(size_t)x0*4u);
        if (x0+w<width) memset(dst+(size_t)w*4u,0,(size_t)(width-x0-w)*4u);
        if (sy==prev) { memcpy(dst,dst-row_bytes,(size_t)w*4u); continue; }
        prev=sy;
        {
            const uint8_t *src=rgb+(size_t)sy*fb_w*3u;
            for(x=0;x<w;x++) {
                const uint8_t *p=src+xmap[x];
                dst[0]=p[2];dst[1]=p[1];dst[2]=p[0];dst[3]=0;dst+=4;
            }
        }
    }
}

/* Fullscreen policy: an explicit --fullscreen wins; PE_FULLSCREEN=0/1
 * overrides; otherwise go fullscreen when running inside Gamescope or Steam
 * Game Mode / Big Picture (Steam Deck, Bazzite, SteamOS), where a windowed
 * 960x720 window would be scaled with a border around it.  env(name) returns
 * the value or NULL (getenv in the host; a table in tests). */
typedef const char *(*HostWindowEnvFn)(const char *name);
static inline int HostWindow_EnvIn(const char *v, const char *needle)
{
    size_t n = strlen(needle);
    for (; v && *v; v++) {
        size_t i = 0;
        while (i < n && v[i] && (v[i] | 0x20) == (needle[i] | 0x20)) i++;
        if (i == n) return 1;
    }
    return 0;
}
static inline int HostWindow_WantFullscreen(int requested, HostWindowEnvFn env)
{
    const char *o = env("PE_FULLSCREEN"), *v;
    if (requested) return 1;
    if (o && *o) return *o != '0';
    if ((v = env("GAMESCOPE_WAYLAND_DISPLAY")) && *v) return 1;
    if (HostWindow_EnvIn(env("XDG_CURRENT_DESKTOP"), "gamescope")) return 1;
    if (HostWindow_EnvIn(env("XDG_SESSION_DESKTOP"), "gamescope")) return 1;
    if ((v = env("SteamDeck")) && *v == '1') return 1;
    if ((v = env("SteamGamepadUI")) && *v == '1') return 1;
    if ((v = env("SteamTenfoot")) && *v == '1') return 1;
    return 0;
}
#endif
