/* Host-side opt-in enhancements (Options in the dev menu): how the finished
 * frame is shown, how the SPU resamples its voices, how the window behaves.
 * They sit beside pe_plat/enhance.h's game-facing hooks but in their own
 * header on purpose: the game ports include enhance.h (through game_port.h),
 * and a presenter or mixer option must not rebuild them.  Every option is in
 * the same registry, `enhancements.conf` and dev menu as the others, and every
 * one defaults to the retail behaviour: at the default the presenter / mixer
 * / window runs the code it ran before the option existed.
 *
 *   display_filter=<0|1|2>  PE_DISPLAY_FILTER   DISPLAY FILTER
 *   display_fit=<0|1|2>     PE_DISPLAY_FIT      WINDOW FIT
 *   spu_interpolation=<0|1> PE_SPU_INTERPOLATION SPU INTERPOLATION
 *   pause_on_focus_loss=<0|1> PE_PAUSE_ON_FOCUS_LOSS PAUSE ON FOCUS LOSS
 */
#ifndef PE_PLAT_ENHANCE_HOST_H
#define PE_PLAT_ENHANCE_HOST_H

#ifdef __cplusplus
extern "C" {
#endif

/* ── Graphics ────────────────────────────────────────────────────────── */
/* DISPLAY FILTER (platform/host_filter_crt.h): scanlines, and scanlines plus
 * an aperture-grille mask, drawn on the finished window image after the
 * display scaler.  OFF is retail.  The values equal HOST_CRT_*. */
enum { PE_ENH_FILTER_OFF = 0, PE_ENH_FILTER_SCANLINES = 1, PE_ENH_FILTER_CRT = 2 };
int  pe_enh_display_filter(void);       /* PE_ENH_FILTER_OFF (default) .. CRT */
void pe_enh_set_display_filter(int mode);

/* WINDOW FIT (platform/host_filter_fit.h): where the frame lands in the
 * window.  KEEP ASPECT is the retail letterbox; INTEGER SCALE the largest
 * whole multiple of the native frame that fits, black around it; STRETCH
 * fills the window ignoring the aspect ratio.  The values equal HOST_FIT_*. */
enum { PE_ENH_FIT_KEEP = 0, PE_ENH_FIT_INTEGER = 1, PE_ENH_FIT_STRETCH = 2 };
int  pe_enh_display_fit(void);          /* PE_ENH_FIT_KEEP (default) .. STRETCH */
void pe_enh_set_display_fit(int mode);

/* ── Audio ───────────────────────────────────────────────────────────── */
/* SPU INTERPOLATION (platform/pe_spu.c): how the synthesizer resamples a
 * voice's sample stream.  GAUSSIAN is the console's 4-point Gaussian table
 * and the code that always ran (retail, default); CUBIC is a 4-point cubic
 * (Catmull-Rom) through the same four samples, which keeps the high
 * frequencies the Gaussian smooths away.  Only the mixer's resampling step
 * changes.  The values equal PE_SPU_INTERP_*. */
enum { PE_ENH_SPU_GAUSSIAN = 0, PE_ENH_SPU_CUBIC = 1 };
int  pe_enh_spu_interpolation(void);    /* PE_ENH_SPU_GAUSSIAN (default) or CUBIC */
void pe_enh_set_spu_interpolation(int mode);

/* ── Quality of life ─────────────────────────────────────────────────── */
/* PAUSE ON FOCUS LOSS (platform/host_filter_pause.h, wired in host_window.c):
 * while the game window does not have the keyboard focus, the presenter holds
 * the game before its next emulated VBlank, so everything on emulated time
 * (logic, sound, movies) stops, and resumes it when the focus returns.  OFF
 * (default, retail) never waits.  A window-only feature: headless runs have
 * no window and never pause. */
int  pe_enh_pause_on_focus_loss(void);  /* 0 (default) or 1 */
void pe_enh_set_pause_on_focus_loss(int on);

#ifdef __cplusplus
}
#endif

#endif /* PE_PLAT_ENHANCE_HOST_H */
