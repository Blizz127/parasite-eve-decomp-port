#!/usr/bin/env python3
"""Mechanical audit of the clean-room build shapes (no -DG2_SMOKE): for every checker group, compile the checker with
`-c` and verify with `nm` that (1) the group's own provider symbols are UNDEFINED (a provider is demanded at link time),
(2) every guest callee that appears in the group's vector lines has a checker stub defined under its public binding name
(headers: `#define func_X name` in pc_port/include/pe_plat/*.h and psx_compat.h; guest name when no binding exists), and
(3) nothing else the checker needs is undefined (besides libc).  Usage (repo root): pe_gpu2_cleanroom_audit.py VECTOR_DIR OUTDIR"""
import glob
import os
import re
import subprocess
import sys

vec, out = sys.argv[1:3]
os.makedirs(out, exist_ok=True)
env = dict(os.environ, TMPDIR=out)
bind = {}
for f in glob.glob('pc_port/include/pe_plat/*.h') + ['pc_port/include/psx_compat.h']:
    for m in re.finditer(r'#define\s+func_([0-9A-F]{8})\s+(\w+)', open(f).read()):
        bind[int(m.group(1), 16)] = m.group(2)
bind.update({0x80073A44: 'HostFB_VSync', 0x80075EE0: 'g2_stub_80075EE0'})   # no public binding at the stub level: neutral stub name (a guest-name call is bound with -Wl,--defsym=func_80075EE0=g2_stub_80075EE0)
# callees the gpu2e provider reaches by guest address (PE_GuestCall, like a jalr through a pointer): checker callees without a stub symbol
BY_ADDRESS = {0x80076170, 0x80076208, 0x800762A0, 0x80076150, 0x800762BC}
# groups: batch -> [(macro, entries, provider symbols)]
G = {
    'gpu2a': [('CHK_PUMP', [0x80076EE4], ['pe_plat_gpuq_pump']),
              ('CHK_SEED', [0x800773D0], ['pe_plat_gpuq_seed_deadline'])],
    'gpu2b': [('CHK_WRAP', [0x8007506C, 0x800750CC, 0x8007512C], ['pe_plat_gpuimg_load_image', 'pe_plat_gpuimg_store_image', 'pe_plat_gpuimg_move_image']),
              ('CHK_RECT', [0x80074E28], ['pe_plat_gpuimg_report_rect']),
              ('CHK_WORK', [0x80076664, 0x800768A0], ['pe_plat_gpuimg_upload_worker', 'pe_plat_gpuimg_readback_worker'])],
    'gpu2c': [('CHK_ADDRESS', [0x80071944, 0x80071964, 0x80071994, 0x80077A64, 0x80077AA4],
               ['pe_plat_gpuaddr_optional_chunk', 'pe_plat_gpuaddr_pixel_chunk', 'pe_plat_gpuaddr_pixel_data', 'pe_plat_gpuaddr_texture_page', 'pe_plat_gpuaddr_clut']),
              ('CHK_PRIM', [0x80075358, 0x800753B4], ['pe_plat_gpuprim_draw_prim', 'pe_plat_gpuprim_draw_otag']),
              ('CHK_OTC', [0x800752AC], ['pe_plat_gpuprim_clear_otag_reverse']),
              ('CHK_OTCW', [0x80076354], ['pe_plat_gpuprim_otc_worker']),
              ('CHK_WORDS', [0x80076B58], ['pe_plat_ctl_gpu_transfer_words']),
              ('CHK_POOL', [0x8007A214], ['pe_plat_gpuprim_pool_init']),
              ('CHK_POOLW', [0x8007A244], ['pe_plat_gpuprim_pool_clear']),
              ('CHK_RANGE', [0x8007C544], ['pe_plat_gpuprim_stream_set_range']),
              ('CHK_LIBGPU', [0x80074F44, 0x80075424, 0x800754E4], ['pe_plat_gpuprim_clear_image', 'pe_plat_gpuprim_put_draw_env', 'pe_plat_gpuprim_draw_otag_env'])],
    'gpu2e': [('CHK_SETENV', [0x80075EE0], ['pe_plat_gpuprim_set_draw_env'])],
    'gpu2d': [('CHK_EXPAND', [0x8010BD4C], ['pe_plat_mdec_expand_table_host']),
              ('CHK_CB', [0x8010C0D8], ['pe_plat_mdec_out_callback_host']),
              ('CHK_RESET', [0x8010BE3C], ['pe_plat_mdec_reset'])],
}
vec_lines = {}
for b in G:
    for ln in open('%s/%s/%s.txt' % (vec, b, b)):
        vec_lines.setdefault(int(ln.split()[0], 16), []).append(ln)
exp_callees = {0x8010BD4C: set()}
bad = 0
print('| batch | group | entries | callees in vectors (binding name) | stubs missing | provider symbols demanded | extra undefined | result |')
print('|---|---|---|---|---|---|---|---|')
for b, groups in G.items():
    for macro, entries, provs in groups:
        obj = '%s/%s_%s.o' % (out, b, macro)
        cmd = ['cc', '-std=gnu11', '-Wall', '-Wextra', '-Werror', '-D' + macro, '-DG2_BATCH_NAME="%s"' % macro, '-Ipc_port/include',
               '-Ipc_port/platform', '-Ipc_port/bootstrap', '-c', 'pc_port/tools/pe_gpu2_%s_check.c' % b, '-o', obj]
        r = subprocess.run(cmd, capture_output=True, text=True, env=env)
        if r.returncode:
            print('| %s | %s | compile failed: %s |' % (b, macro, r.stderr.strip().split('\n')[0][:120]))
            bad += 1
            continue
        defd = set(l.split()[2] for l in subprocess.run(['nm', '--defined-only', obj], capture_output=True, text=True).stdout.split('\n') if len(l.split()) == 3)
        und = set(l.split()[1] for l in subprocess.run(['nm', '-u', obj], capture_output=True, text=True).stdout.split('\n') if len(l.split()) == 2)
        callees = set()
        for e in entries:
            for ln in vec_lines.get(e, []):
                for m in re.finditer(r'C:func_([0-9A-F]{8}):', ln):
                    callees.add(int(m.group(1), 16))
                for m in re.finditer(r'(?:^| |;)func_([0-9A-F]{8})=', ln.split(' R ')[1].split(' RET ')[0] if ' R ' in ln else ''):
                    pass
        names = {}
        hooked = 0
        for c in sorted(callees):
            if c in entries:
                continue
            if c not in bind or c in BY_ADDRESS:
                hooked += 1                   # reached through a RAM function-pointer word (PE_GuestHook_Call stub)
                continue
            names[c] = bind[c]
        missing = sorted(n for c, n in names.items() if n not in defd and n not in provs)
        # callee stub must not be a provider symbol of this group
        demanded = sorted(p for p in provs if p in und)
        extra = sorted(u for u in und if u not in provs and re.match(r'(PE_|Bootstrap_|pe_plat_|func_|Host|g2_)', u))
        ok = not missing and demanded == sorted(provs) and not extra and not any(p in defd for p in provs)
        bad += not ok
        print('| %s | %s | %s | %s | %s | %s | %s | %s |' % (
            b, macro, ' '.join('%X' % e for e in entries),
            ' '.join('%s' % n for n in names.values()) or '-', ' '.join(missing) or 'none',
            'all %d' % len(provs) if demanded == sorted(provs) else 'MISSING ' + ' '.join(sorted(set(provs) - set(demanded))),
            ' '.join(extra) or 'none', 'OK' if ok else 'GAP'))
print('\ngroups with gaps: %d' % bad)
sys.exit(1 if bad else 0)
