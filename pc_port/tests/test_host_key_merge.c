/* Synthetic key IDs and pad masks only; no window, game, device or save. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "host_key_merge.h"
#include "host_pad.h"
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"failed line %d\n",__LINE__);return 1; } } while(0)
int main(void)
{
    HostKeyMerge m;HostKeyMerge_Reset(&m);
    CHECK(HostKeyMerge_Held(&m)==0 && HostKeyMerge_Held(NULL)==0);
    CHECK(HostKeyMerge_Event(&m,21,HOST_PAD_CROSS,1));
    CHECK(HostKeyMerge_Event(&m,31,HOST_PAD_CROSS,1));
    CHECK(HostKeyMerge_Event(&m,21,HOST_PAD_CROSS,0));
    CHECK(HostKeyMerge_Held(&m)==HOST_PAD_CROSS);
    CHECK(HostKeyMerge_Event(&m,31,0,0) && HostKeyMerge_Held(&m)==0);
    CHECK(HostKeyMerge_Event(&m,21,HOST_PAD_CROSS,1));
    CHECK(HostKeyMerge_Event(&m,31,HOST_PAD_CROSS,1));
    CHECK(HostKeyMerge_Event(&m,31,0,0) && HostKeyMerge_Held(&m)==HOST_PAD_CROSS);
    CHECK(HostKeyMerge_Event(&m,21,0,0) && HostKeyMerge_Held(&m)==0);
    /* Repeat is idempotent; one release clears that physical key. */
    CHECK(HostKeyMerge_Event(&m,21,HOST_PAD_CROSS,1));
    CHECK(HostKeyMerge_Event(&m,21,HOST_PAD_CROSS,1));
    CHECK(HostKeyMerge_Event(&m,21,0,0) && HostKeyMerge_Held(&m)==0);
    /* A new mapping for a still-held key replaces its old contribution. */
    CHECK(HostKeyMerge_Event(&m,21,HOST_PAD_CROSS,1));
    CHECK(HostKeyMerge_Event(&m,31,HOST_PAD_CROSS,1));
    CHECK(HostKeyMerge_Event(&m,21,HOST_PAD_CIRCLE,1));
    CHECK(HostKeyMerge_Held(&m)==(HOST_PAD_CROSS|HOST_PAD_CIRCLE));
    CHECK(HostKeyMerge_Event(&m,31,0,0) && HostKeyMerge_Held(&m)==HOST_PAD_CIRCLE);
    CHECK(HostKeyMerge_Event(&m,511,HOST_PAD_START,1));
    CHECK(!HostKeyMerge_Event(&m,512,HOST_PAD_UP,1));
    CHECK(!HostKeyMerge_Event(&m,UINT32_MAX,HOST_PAD_UP,1));
    CHECK(!HostKeyMerge_Event(NULL,0,HOST_PAD_UP,1));
    CHECK(HostKeyMerge_Held(&m)==(HOST_PAD_CIRCLE|HOST_PAD_START));
    HostKeyMerge_Reset(&m);HostKeyMerge_Reset(NULL);
    CHECK(HostKeyMerge_Held(&m)==0);
    CHECK(HostKeyMerge_Event(&m,21,HOST_PAD_CROSS,0) && HostKeyMerge_Held(&m)==0);
    /* HostKeyInput: the backend glue. */
    {
        HostKeyInput in;HostKeyInput_Reset(&in);HostKeyInput_Reset(NULL);
        CHECK(HostKeyInput_Held(&in)==0 && HostKeyInput_Held(NULL)==0);
        CHECK(HostKeyInput_Release(NULL,1)==0 && HostKeyInput_Mapped(NULL,1,HOST_PAD_CROSS,1)==0);
        /* shared button: either key keeps it held */
        CHECK(HostKeyInput_Mapped(&in,36,HOST_PAD_CROSS,1)==HOST_PAD_CROSS);
        CHECK(HostKeyInput_Mapped(&in,52,HOST_PAD_CROSS,1)==HOST_PAD_CROSS);
        CHECK(HostKeyInput_Release(&in,36)==HOST_PAD_CROSS);
        CHECK(HostKeyInput_Mapped(&in,36,HOST_PAD_CROSS,0)==HOST_PAD_CROSS);  /* late Mapped up is harmless */
        CHECK(HostKeyInput_Release(&in,52)==0);
        /* a release consumed before the pad table still clears */
        CHECK(HostKeyInput_Mapped(&in,36,HOST_PAD_CROSS,1)==HOST_PAD_CROSS);
        CHECK(HostKeyInput_Release(&in,36)==0);
        /* an unmapped key-down contributes nothing */
        CHECK(HostKeyInput_Mapped(&in,40,0,1)==0);
        /* out-of-range IDs fall back to per-bit set/clear, never lost */
        CHECK(HostKeyInput_Mapped(&in,HOST_KEY_MERGE_KEYS,HOST_PAD_START,1)==HOST_PAD_START);
        CHECK(HostKeyInput_Mapped(&in,52,HOST_PAD_CROSS,1)==(HOST_PAD_START|HOST_PAD_CROSS));
        CHECK(HostKeyInput_Release(&in,HOST_KEY_MERGE_KEYS)==(HOST_PAD_START|HOST_PAD_CROSS)); /* fallback waits for Mapped */
        CHECK(HostKeyInput_Mapped(&in,UINT32_MAX,HOST_PAD_START,0)==HOST_PAD_CROSS);
        CHECK(HostKeyInput_Mapped(&in,HOST_KEY_MERGE_KEYS,HOST_PAD_SELECT,1)==(HOST_PAD_SELECT|HOST_PAD_CROSS));
        HostKeyInput_Reset(&in);
        CHECK(HostKeyInput_Held(&in)==0);
        CHECK(HostKeyInput_Mapped(&in,52,HOST_PAD_CROSS,1)==HOST_PAD_CROSS);  /* usable after reset */
    }
    puts("synthetic key merge, remap, repeat, range, reset and backend-glue controls pass");return 0;
}
