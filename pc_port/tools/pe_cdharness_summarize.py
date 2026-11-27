#!/usr/bin/env python3
"""Summarise the sw-cdharness legs: one line per run dir (rc/times, per-function calls + first/last VBlank, stop names, symbolised stop stack).
usage: pe_cdharness_summarize.py WORKDIR NMFILE   (NMFILE = `nm -n` of the exact binary that ran; PIE-relative stop offsets are resolved against it)"""
import bisect, glob, os, re, sys
W, NM = sys.argv[1], sys.argv[2]
syms = []
for l in open(NM):
    p = l.split()
    if len(p) == 3 and p[1] in 'tTwW': syms.append((int(p[0], 16), p[2]))
addrs = [a for a, _ in syms]
def sym(a):
    i = bisect.bisect_right(addrs, a) - 1
    return '%s+0x%x' % (syms[i][1], a - syms[i][0])
NAMES = {'8007AAB4': 'ack', '8007B010': 'poll', '8007B558': 'cmd', '8007BF44': 'sector', '8007C564': 'stream', '8007CEAC': 'dma'}
for d in sorted(glob.glob(os.path.join(W, 'run-*'))):
    tag = os.path.basename(d)[4:]
    log = next(iter(glob.glob(d + '/cold.log') + glob.glob(d + '/day6-*.log') + glob.glob(d + '/run.log')), None)
    times = open(d + '/times.txt').read().split('\n') if os.path.exists(d + '/times.txt') else []
    st, en = (times[0][6:26] if times else '?'), next((t for t in times if t.startswith('end')), 'end ? rc=?')
    ent = []
    sp = d + '/capture.cap.stats'
    if os.path.exists(sp):
        for l in open(sp):
            m = re.match(r'ENTRY (\w+) calls=(\d+) distinct=(\d+) logged=\d+ first_vb=(\d+) last_vb=(\d+)', l)
            if m and int(m.group(2)): ent.append('%s=%s(first %s,last %s)' % (NAMES[m.group(1)], m.group(2), m.group(4), m.group(5)))
    if os.path.exists(sp):
        m = re.search(r'timeout_recover_calls=(\d+)', open(sp).read())
        if m: ent.append('TimeoutRecover=%s' % m.group(1))
    stops = stk = ''
    result = ''
    if log:
        txt = open(log, errors='replace').read()
        stops = ','.join(sorted(set(re.findall(r'BOOTSTRAP_RET\] (\S+)', txt) + re.findall(r'pe_plat_cd_stream_stop\("([^"]+)"\)', txt))))
        m = re.search(r'\[STOP\].*', txt)
        if m: stk = ' <- '.join(sym(int(x, 16)) for x in re.findall(r'\+0x([0-9a-f]+)', m.group(0))[2:5])
        notes = sorted(set(re.findall(r'\[PE_TEST_CD\] (\w+)', txt)) - {'coverage'})
        m = re.search(r'stop_reason=(\S+)', txt)
        result = (m.group(1) if m else '-')
    drv = ''
    if os.path.exists(d + '/driver.out'):
        m = re.findall(r'DAY6 RESULT=(\S+)', open(d + '/driver.out').read())
        drv = m[-1] if m else ''
    print('%-24s %s %s | %s | %s | stop_reason=%s day6=%s | boundary=%s | %s' % (tag, st, en.replace('end ', ''), ' '.join(ent) or '-', ','.join(notes) if log else '-', result, drv or '-', stops or '-', stk or '-'))
