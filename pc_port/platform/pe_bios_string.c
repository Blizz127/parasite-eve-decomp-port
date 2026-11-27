/* BIOS A-function string/memory adapters; contract and source in
 * pe_bios_string.h.  Quotes are verbatim from psx-spx "BIOS String
 * Functions" / "BIOS Memory Fill/Copy/Compare". */
#include "pe_bios_string.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

/* Byte values are compared as loaded unsigned bytes; psx-spx specifies the
 * result as "[str1+N]-[str2+N]" without stating signedness, and every
 * matched caller (func_800727B4, func_80072950, ...) only tests == 0. */

/* A(17h) strcmp: "Compares the strings up to including ending 00h byte.
 * Returns 0 if they are identical, or otherwise [str1+N]-[str2+N] ... The
 * function rejects str1/str2 values of 00000000h (and returns 0=both are
 * zero, -1=only str1 is zero, and +1=only str2 is zero)." */
int32_t pe_plat_bios_strcmp(pe_addr_t str1, pe_addr_t str2)
{
    if (str1 == 0 || str2 == 0)
        return (str1 == 0 && str2 == 0) ? 0 : (str1 == 0 ? -1 : 1);
    for (uint32_t n = 0;; n++) {
        int32_t a = PE_LoadU8(str1 + n), b = PE_LoadU8(str2 + n);
        if (a != b)
            return a - b;
        if (a == 0)
            return 0;
    }
}

/* A(18h) strncmp: "Same as 'strcmp' but stops after comparing 'maxlen'
 * characters (and returns 0 if they did match). If the strings are shorter,
 * then comparision stops at the ending 00h byte."  ("Same as strcmp" also
 * covers the 00000000h rejection.) */
int32_t pe_plat_bios_strncmp(pe_addr_t str1, pe_addr_t str2, int32_t maxlen)
{
    if (str1 == 0 || str2 == 0)
        return (str1 == 0 && str2 == 0) ? 0 : (str1 == 0 ? -1 : 1);
    for (int32_t n = 0; n < maxlen; n++) {
        int32_t a = PE_LoadU8(str1 + (uint32_t)n), b = PE_LoadU8(str2 + (uint32_t)n);
        if (a != b)
            return a - b;
        if (a == 0)
            return 0;
    }
    return 0;
}

/* A(19h) strcpy: "Copies data from src to dst, up to including the ending
 * 00h byte. Refuses to copy anything if src or dst is 00000000h. Returns
 * the incoming dst address (or 0 if copy was refused)." */
pe_addr_t pe_plat_bios_strcpy(pe_addr_t dst, pe_addr_t src)
{
    uint8_t c;
    uint32_t n = 0;

    if (dst == 0 || src == 0)
        return 0;
    do {
        c = PE_LoadU8(src + n);
        PE_StoreU8(dst + n, c);
        n++;
    } while (c != 0);
    return dst;
}

/* A(1Bh) strlen: "Returns the length of the string up to excluding the
 * ending 00h byte (or 0 when src is 00000000h)." */
int32_t pe_plat_bios_strlen(pe_addr_t src)
{
    int32_t n = 0;

    if (src == 0)
        return 0;
    while (PE_LoadU8(src + (uint32_t)n) != 0)
        n++;
    return n;
}

/* A(2Ah) memcpy: "Refuses to copy any data when dst=00000000h or when
 * len>7FFFFFFFh. The return value is always the incoming 'dst' value." */
pe_addr_t pe_plat_bios_memcpy(pe_addr_t dst, pe_addr_t src, int32_t len)
{
    if (dst != 0 && len >= 0) {
        for (int32_t n = 0; n < len; n++)
            PE_StoreU8(dst + (uint32_t)n, PE_LoadU8(src + (uint32_t)n));
    }
    return dst;
}

/* A(2Bh) memset: "Refuses to fill memory when dst=00000000h or when
 * len>7FFFFFFFh. The return value is the incoming 'dst' value (or zero,
 * when len=0 or len>7FFFFFFFh)." */
pe_addr_t pe_plat_bios_memset(pe_addr_t dst, int32_t fillbyte, int32_t len)
{
    if (len <= 0)
        return 0;
    if (dst == 0)
        return dst;
    for (int32_t n = 0; n < len; n++)
        PE_StoreU8(dst + (uint32_t)n, (uint8_t)fillbyte);
    return dst;
}

/* Host sprintf (see pe_bios_string.h).  Standard C formatting semantics. */
int32_t PE_Bios_SprintfHost(char *dst, uint32_t cap, pe_addr_t fmt, const uint32_t *args, int nargs)
{
    uint32_t out = 0, f = fmt;
    int ai = 0;
#define NEXT_ARG() (ai < nargs ? args[ai++] : 0u)
#define FORMAT_ERROR() do { if (cap) dst[0] = 0; return -1; } while (0)
#define EMIT(ch) do { if (out == INT32_MAX) FORMAT_ERROR(); if (out + 1u < cap) dst[out] = (char)(ch); out++; } while (0)
#define EMIT_REPEAT(ch, count) do { \
    uint64_t repeat_count = (uint64_t)(count); \
    if (repeat_count > (uint64_t)INT32_MAX - out) FORMAT_ERROR(); \
    uint32_t available = cap && out < cap - 1u ? cap - 1u - out : 0u; \
    size_t write_count = repeat_count < available ? (size_t)repeat_count : available; \
    if (write_count) memset(dst + out, (ch), write_count); \
    out += (uint32_t)repeat_count; \
} while (0)
    for (;;) {
        char c = (char)PE_LoadU8(f++);
        char digits[40];
        int n = 0, left = 0, zero = 0, plus = 0, space = 0, width = 0, prec = -1;
        int neg = 0, short_arg = 0, alternate = 0;
        int extra_zeroes = 0, zero_insert = 0;
        uint32_t v;
        if (!c) break;
        if (c != '%') { EMIT(c); continue; }
        for (;;) {
            c = (char)PE_LoadU8(f++);
            if (c == '-') left = 1;
            else if (c == '0') zero = 1;
            else if (c == '+') plus = 1;
            else if (c == ' ') space = 1;
            else if (c == '#') alternate = 1;
            else break;
        }
        if (c == '*') { width = (int32_t)NEXT_ARG(); c = (char)PE_LoadU8(f++); }
        else while (c >= '0' && c <= '9') {
            int digit = c - '0';
            if (width > (INT_MAX - digit) / 10) FORMAT_ERROR();
            width = width * 10 + digit; c = (char)PE_LoadU8(f++);
        }
        if (c == '.') {
            prec = 0; c = (char)PE_LoadU8(f++);
            if (c == '*') { prec = (int32_t)NEXT_ARG(); c = (char)PE_LoadU8(f++); }
            else while (c >= '0' && c <= '9') {
                int digit = c - '0';
                if (prec > (INT_MAX - digit) / 10) FORMAT_ERROR();
                prec = prec * 10 + digit; c = (char)PE_LoadU8(f++);
            }
        }
        while (c == 'h' || c == 'l' || c == 'L') {
            if (c == 'h') short_arg = 1;
            c = (char)PE_LoadU8(f++);
        }
        if (width == INT32_MIN) FORMAT_ERROR();
        if (width < 0) { left = 1; width = -width; }
        switch (c) {
        case 'c':
            digits[n++] = (char)NEXT_ARG();
            break;
        case 's': {
            pe_addr_t s = NEXT_ARG();
            int len = 0;
            if (alternate && s) {
                len = PE_LoadU8(s++);
                if (prec >= 0 && len > prec) len = prec;
            } else {
                while (s && (prec < 0 || len < prec) && PE_LoadU8(s + (uint32_t)len)) len++;
            }
            if (!left && width > len) EMIT_REPEAT(' ', width - len);
            for (int i = 0; i < len; i++) EMIT(PE_LoadU8(s + (uint32_t)i));
            if (left && width > len) EMIT_REPEAT(' ', width - len);
            continue;
        }
        case 'n': {
            pe_addr_t target = NEXT_ARG();
            if (short_arg) PE_StoreU16(target, (uint16_t)out);
            else PE_StoreU32(target, out);
            continue;
        }
        case 'd': case 'i': case 'u': case 'x': case 'X': case 'o': case 'p': {
            unsigned base = (c == 'x' || c == 'X' || c == 'p') ? 16u : c == 'o' ? 8u : 10u;
            const char *hex = (c == 'X' || c == 'p') ? "0123456789ABCDEF" : "0123456789abcdef";
            /* The original %p always prints eight uppercase hexadecimal digits. */
            if (c == 'p') prec = 8;
            v = NEXT_ARG();
            if (short_arg) {
                v &= 0xFFFFu;
                if ((c == 'd' || c == 'i') && (v & 0x8000u)) v |= 0xFFFF0000u;
            }
            /* Retail prefixes zero too ("%#x" of 0 is "0x0"; bios_sprintf_retail_vectors.tsv). */
            int prefix = alternate && (c == 'p' || c == 'x' || c == 'X');
            if ((c == 'd' || c == 'i') && (int32_t)v < 0) { neg = 1; v = 0u - v; }
            if (v || prec != 0)
                do { digits[n++] = hex[v % base]; v /= base; } while (v && n < 32);
            /* Retail emits no digit at all for alternate octal zero with precision 0
             * (bios_sprintf_retail_vectors.tsv), unlike host C's "0". */
            if (c == 'o' && alternate && n && digits[n - 1] != '0') digits[n++] = '0';
            while (prec >= 0 && n < prec && n < 32) digits[n++] = '0';
            if (prec > 32) extra_zeroes = prec - 32;
            if (neg) digits[n++] = '-';
            else if ((c == 'd' || c == 'i') && plus) digits[n++] = '+';
            else if ((c == 'd' || c == 'i') && space) digits[n++] = ' ';
            if (zero && !left && prec < 0) {
                int sign = neg || ((c == 'd' || c == 'i') && (plus || space));
                char s = sign ? digits[--n] : 0;
                while (n + sign + 2 * prefix < width && n < 38) digits[n++] = '0';
                if (sign) digits[n++] = s;
            }
            if (prefix) {
                digits[n++] = c;
                digits[n++] = '0';
            }
            /* digits are reversed */
            for (int i = 0; i < n / 2; i++) { char t = digits[i]; digits[i] = digits[n - 1 - i]; digits[n - 1 - i] = t; }
            if (zero && !left && prec < 0 && width > n) {
                extra_zeroes = width - n;
            }
            zero_insert = prefix ? 2 : (neg || ((c == 'd' || c == 'i') && (plus || space)) ? 1 : 0);
            break;
        }
        case '%':
            digits[n++] = '%';
            break;
        case 0:
            f--;
            continue;
        default:
            digits[n++] = c;
            break;
        }
        int64_t padding = (int64_t)width - n - extra_zeroes;
        if (!left && padding > 0) EMIT_REPEAT(' ', padding);
        for (int i = 0; i < n; i++) {
            if (i == zero_insert) EMIT_REPEAT('0', extra_zeroes);
            EMIT(digits[i]);
        }
        if (zero_insert == n) EMIT_REPEAT('0', extra_zeroes);
        if (left && padding > 0) EMIT_REPEAT(' ', padding);
    }
    if (cap) dst[out < cap ? out : cap - 1u] = 0;
    return (int32_t)out;
#undef NEXT_ARG
#undef EMIT
#undef EMIT_REPEAT
#undef FORMAT_ERROR
}

int32_t PE_Bios_Sprintf(pe_addr_t dst, pe_addr_t fmt, const uint32_t *args, int nargs)
{
    char buf[1024];
    int32_t n = PE_Bios_SprintfHost(buf, sizeof buf, fmt, args, nargs);
    if (n < 0) { PE_StoreU8(dst, 0u); return n; }
    uint32_t len = (uint32_t)n < sizeof buf ? (uint32_t)n : (uint32_t)sizeof buf - 1u;
    for (uint32_t i = 0; i < len; i++) PE_StoreU8(dst + i, (uint8_t)buf[i]);
    PE_StoreU8(dst + len, 0u);
    return n;
}

static struct { unsigned n; pe_addr_t fmt; uint32_t args[4]; } g_printf_log;

unsigned PE_Bios_PrintfLog(pe_addr_t *last_fmt, uint32_t last_args[4])
{
    if (last_fmt) *last_fmt = g_printf_log.fmt;
    if (last_args) memcpy(last_args, g_printf_log.args, sizeof g_printf_log.args);
    return g_printf_log.n;
}

void PE_Bios_PrintfLogReset(void) { memset(&g_printf_log, 0, sizeof g_printf_log); }

int32_t PE_Bios_Printf(pe_addr_t fmt, const uint32_t *args, int nargs)
{
    char buf[1024];
    g_printf_log.n++;
    g_printf_log.fmt = fmt;
    memset(g_printf_log.args, 0, sizeof g_printf_log.args);
    for (int i = 0; i < nargs && i < 4; i++) g_printf_log.args[i] = args[i];
    int32_t n = PE_Bios_SprintfHost(buf, sizeof buf, fmt, args, nargs);
    if (n < 0) return n;
    size_t len = (uint32_t)n < sizeof buf ? (uint32_t)n : sizeof buf - 1u;
    fputs("[TTY] ", stderr);
    (void)fwrite(buf, 1u, len, stderr);
    if (!len || buf[len - 1u] != '\n') fputc('\n', stderr);
    return n;
}

/* Host replacement for func_80071A74 (printf through the BIOS A(3Fh) vector;
 * pe_port_compat.h): lowered code calls it with the guest format and up to
 * seven word arguments (0 past the call site's own). */
int32_t PE_Printf_Guest(pe_addr_t fmt, uint32_t a1, uint32_t a2, uint32_t a3,
                        uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7)
{
    const uint32_t args[7] = {a1, a2, a3, a4, a5, a6, a7};
    return PE_Bios_Printf(fmt, args, 7);
}
