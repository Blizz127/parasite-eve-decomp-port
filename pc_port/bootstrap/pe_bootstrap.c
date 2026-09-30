/*
 * Phase 6D-S — Centralized bootstrap policy implementation.
 */
#include "pe_bootstrap.h"
#include "stub_registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_strict_enabled = 0;
static int g_strict_triggered = 0;

BootstrapArgCall g_bootstrap_arg_calls[BOOTSTRAP_MAX_ARG_CALLS];
int g_bootstrap_arg_call_count = 0;
BootstrapArgCall4 g_bootstrap_arg4_calls[BOOTSTRAP_MAX_ARG4_CALLS];
int g_bootstrap_arg4_call_count = 0;
BootstrapArgCall5 g_bootstrap_arg5_calls[BOOTSTRAP_MAX_ARG5_CALLS];
int g_bootstrap_arg5_call_count = 0;

/* ── Provider sequence state ────────────────────────────────────────── */

#define BOOTSTRAP_MAX_PROVIDERS 16

typedef struct {
    const char *symbol;
    int  values[BOOTSTRAP_MAX_SEQ];
    int  count;
    int  index;
} ProviderSeq;

static ProviderSeq g_providers[BOOTSTRAP_MAX_PROVIDERS];
static int         g_provider_count = 0;

void Bootstrap_Init(void)
{
    g_strict_enabled  = 0;
    g_strict_triggered = 0;
    g_stub_count = 0;
    g_stub_order_count = 0;
    g_stub_bootstrap_invocations = 0;
    Bootstrap_ResetArgCallLog();
    Bootstrap_ResetArg4CallLog();
    Bootstrap_ResetArg5CallLog();
    Bootstrap_ClearSequences();
}

void Bootstrap_Shutdown(void)
{
    Stub_PrintSummary();
}

void Bootstrap_EnableStrict(void)
{
    g_strict_enabled = 1;
}

void Bootstrap_SetIntSequence(const char *symbol, const int *values, int count)
{
    if (!symbol || !values || count < 1 || count > BOOTSTRAP_MAX_SEQ) {
        fprintf(stderr, "FATAL: Bootstrap_SetIntSequence: bad arguments\n");
        abort();
    }
    for (int i = 0; i < g_provider_count; i++) {
        if (strcmp(g_providers[i].symbol, symbol) == 0) {
            memcpy(g_providers[i].values, values, (size_t)count * sizeof(int));
            g_providers[i].count = count;
            g_providers[i].index = 0;
            return;
        }
    }
    if (g_provider_count >= BOOTSTRAP_MAX_PROVIDERS) {
        fprintf(stderr, "FATAL: Bootstrap provider table overflow\n");
        abort();
    }
    g_providers[g_provider_count].symbol = symbol;
    memcpy(g_providers[g_provider_count].values, values,
           (size_t)count * sizeof(int));
    g_providers[g_provider_count].count = count;
    g_providers[g_provider_count].index = 0;
    g_provider_count++;
}

void Bootstrap_ClearSequences(void)
{
    g_provider_count = 0;
}

static int sequence_next(const char *symbol, int *out)
{
    for (int i = 0; i < g_provider_count; i++) {
        if (strcmp(g_providers[i].symbol, symbol) == 0) {
            if (g_providers[i].index < g_providers[i].count) {
                *out = g_providers[i].values[g_providers[i].index++];
                return 1;
            }
            return 0;  /* exhausted — caller uses default */
        }
    }
    return 0;
}

static void check_strict(const char *symbol, const char *caller)
{
    if (g_strict_enabled && !g_strict_triggered) {
        g_strict_triggered = 1;
        fprintf(stderr,
            "FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider: %s\n"
            "       called from: %s\n",
            symbol, caller ? caller : "(unknown)");
        exit(1);
    }
}

int Bootstrap_ReturnInt(const char *symbol, const char *caller, int value)
{
    int scripted;
    check_strict(symbol, caller);
    Stub_Record(symbol, "BOOTSTRAP_RET");
    if (sequence_next(symbol, &scripted)) {
        return scripted;
    }
    return value;
}

int Bootstrap_ReturnInt1(const char *symbol, const char *caller, int value,
                         uint32_t arg0)
{
    if (g_bootstrap_arg_call_count < BOOTSTRAP_MAX_ARG_CALLS) {
        BootstrapArgCall *call =
            &g_bootstrap_arg_calls[g_bootstrap_arg_call_count++];
        call->symbol = symbol;
        call->caller = caller;
        call->arg0 = arg0;
    }
    return Bootstrap_ReturnInt(symbol, caller, value);
}

void Bootstrap_ReturnVoid1(const char *symbol, const char *caller,
                           uint32_t arg0)
{
    if (g_bootstrap_arg_call_count < BOOTSTRAP_MAX_ARG_CALLS) {
        BootstrapArgCall *call =
            &g_bootstrap_arg_calls[g_bootstrap_arg_call_count++];
        call->symbol = symbol;
        call->caller = caller;
        call->arg0 = arg0;
    }
    Bootstrap_ReturnVoid(symbol, caller);
}

void Bootstrap_ResetArgCallLog(void)
{
    g_bootstrap_arg_call_count = 0;
}

void Bootstrap_ReturnVoid4(const char *symbol, const char *caller,
                           uintptr_t arg0, uintptr_t arg1,
                           uintptr_t arg2, uintptr_t arg3)
{
    Bootstrap_ReturnVoid4Indirect(symbol, caller, 0,
                                  arg0, arg1, arg2, arg3);
}

void Bootstrap_ReturnVoid4Indirect(const char *symbol, const char *caller,
                                   uintptr_t target,
                                   uintptr_t arg0, uintptr_t arg1,
                                   uintptr_t arg2, uintptr_t arg3)
{
    Bootstrap_RecordArg4Indirect(symbol, caller, target,
                                 arg0, arg1, arg2, arg3, NULL, 0);
    Bootstrap_ReturnVoid(symbol, caller);
}

void Bootstrap_RecordArg4Indirect(const char *symbol, const char *caller,
                                  uintptr_t target,
                                  uintptr_t arg0, uintptr_t arg1,
                                  uintptr_t arg2, uintptr_t arg3,
                                  const void *payload,
                                  uint32_t payload_size)
{
    if (payload_size > 16u || (payload_size != 0u && payload == NULL)) {
        fprintf(stderr, "FATAL: Bootstrap_RecordArg4Indirect: bad payload\n");
        abort();
    }
    if (g_bootstrap_arg4_call_count < BOOTSTRAP_MAX_ARG4_CALLS) {
        BootstrapArgCall4 *call =
            &g_bootstrap_arg4_calls[g_bootstrap_arg4_call_count++];
        call->symbol = symbol;
        call->caller = caller;
        call->target = target;
        call->arg0 = arg0;
        call->arg1 = arg1;
        call->arg2 = arg2;
        call->arg3 = arg3;
        call->payload_size = payload_size;
        if (payload_size != 0u)
            memcpy(call->payload, payload, payload_size);
    }
}

int Bootstrap_ReturnInt4Indirect(const char *symbol, const char *caller,
                                 int value, uintptr_t target,
                                 uintptr_t arg0, uintptr_t arg1,
                                 uintptr_t arg2, uintptr_t arg3,
                                 const void *payload, uint32_t payload_size)
{
    Bootstrap_RecordArg4Indirect(symbol, caller, target,
                                 arg0, arg1, arg2, arg3,
                                 payload, payload_size);
    return Bootstrap_ReturnInt(symbol, caller, value);
}

void Bootstrap_ResetArg4CallLog(void)
{
    g_bootstrap_arg4_call_count = 0;
}

void Bootstrap_ReturnVoid5(const char *symbol, const char *caller,
                           uintptr_t arg0, uintptr_t arg1,
                           uintptr_t arg2, uintptr_t arg3,
                           uintptr_t arg4)
{
    Bootstrap_ReturnVoid5Indirect(symbol, caller, 0,
                                  arg0, arg1, arg2, arg3, arg4);
}

void Bootstrap_ReturnVoid5Indirect(const char *symbol, const char *caller,
                                   uintptr_t target,
                                   uintptr_t arg0, uintptr_t arg1,
                                   uintptr_t arg2, uintptr_t arg3,
                                   uintptr_t arg4)
{
    if (g_bootstrap_arg5_call_count < BOOTSTRAP_MAX_ARG5_CALLS) {
        BootstrapArgCall5 *call =
            &g_bootstrap_arg5_calls[g_bootstrap_arg5_call_count++];
        call->symbol = symbol;
        call->caller = caller;
        call->target = target;
        call->arg0 = arg0;
        call->arg1 = arg1;
        call->arg2 = arg2;
        call->arg3 = arg3;
        call->arg4 = arg4;
    }
    Bootstrap_ReturnVoid(symbol, caller);
}

void Bootstrap_ResetArg5CallLog(void)
{
    g_bootstrap_arg5_call_count = 0;
}

void Bootstrap_ReturnVoid(const char *symbol, const char *caller)
{
    check_strict(symbol, caller);
    Stub_Record(symbol, "BOOTSTRAP_RET");
}

int Bootstrap_InvocationCount(void)
{
    int n = 0;
    for (int i = 0; i < g_stub_count; i++) {
        if (strcmp(g_stub_registry[i].classification, "BOOTSTRAP_RET") == 0) {
            n++;
        }
    }
    return n;
}

void Bootstrap_PrintSummary(void)
{
    Stub_PrintSummary();
}
