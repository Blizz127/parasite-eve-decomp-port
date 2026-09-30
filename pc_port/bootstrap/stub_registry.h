/*
 * Phase 6A — Stub registry.
 *
 * Every unresolved PS1 SDK function reached by the boot slice must be
 * explicitly classified here.  No anonymous empty stubs.
 *
 * Classification:
 *   IMPLEMENTED    — Real host equivalent
 *   HOST_ADAPTED   — Narrow wrapper around host functionality
 *   BOOTSTRAP_RET  — Temporary return value for milestone gating
 *   UNSUPPORTED    — Traps with symbol name at runtime
 */

#ifndef STUB_REGISTRY_H
#define STUB_REGISTRY_H

/* ── Invocation tracking ───────────────────────────────────────────── */

#define MAX_TRACE_DEPTH 256

typedef struct {
    const char *symbol;
    const char *classification;
    int         invoked;
} StubEntry;

extern StubEntry g_stub_registry[];
extern int       g_stub_count;
extern int       g_stub_bootstrap_invocations;

/* Call this ONCE per unsupported/bootstrap function on first invocation */
void Stub_Record(const char *symbol, const char *classification);

/* Dump summary counts to stdout */
void Stub_PrintSummary(void);

/* ── Ordered call log (for test verification) ──────────────────────── */
#define MAX_ORDER_LOG 4096
extern const char *g_stub_order_log[MAX_ORDER_LOG];
extern int g_stub_order_count;
void Stub_ResetOrderLog(void);

/* ── Bootstrap disc mode ───────────────────────────────────────────── */
extern int g_bootstrap_disc;  /* set by --bootstrap-disc */
extern int g_strict_stubs;    /* set by --strict-stubs  */

#endif
