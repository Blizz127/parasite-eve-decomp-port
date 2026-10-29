/* The opt-in host-side enhancements (docs/port/ENHANCEMENTS.md, GRAPHICS_OPTIONS.md):
 * one registration point for test_native.c.  Each feature's tests live in their own
 * header, in the order the features were added. */
#include "test_enh_display.h"   /* DISPLAY FILTER (CRT) and WINDOW FIT */
#include "test_enh_audio.h"     /* SPU INTERPOLATION (Gaussian / cubic) */
#include "test_enh_qol.h"       /* PAUSE ON FOCUS LOSS */

static void test_ENHX_all(void)
{
    test_ENHX_display_all();
    test_ENHX_audio_all();
    test_ENHX_qol_all();
    enh_isolate();                                  /* leave defaults for later tests */
    enh_hermetic();
}
