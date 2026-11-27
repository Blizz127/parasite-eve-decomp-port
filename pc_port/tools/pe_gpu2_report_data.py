#!/usr/bin/env python3
"""Emit the per-function table rows of docs/evidence/native-gpu2-20261126/REPORT.md from the generated vectors and the
yaml rows (no provider source involved).  Usage: pe_gpu2_report_data.py VECTOR_DIR [EXPAND_VECTORS]"""
import sys
from collections import Counter

import pe_gpu2_vectors as V
import pe_gpu2_common as C

PROVIDER = {
    'func_80076EE4': 'platform/pe_plat2_gpuq.c (pe_plat_gpuq_pump)',
    'func_800773D0': 'platform/pe_plat2_gpuq.c (pe_plat_gpuq_seed_deadline)',
    'func_80076664': 'platform/pe_plat_gpu_upload_worker_host.c (+ boot/func_80076664_port.c)',
    'func_800768A0': 'platform/pe_plat_gpu_readback_worker_host.c (+ boot/func_800768A0_port.c)',
    'func_8007506C': 'boot/func_8007506C_port.c', 'func_800750CC': 'boot/func_8007506C_port.c',
    'func_80074E28': 'boot/func_8007506C_port.c', 'func_8007512C': 'boot/func_8007512C_port.c',
    'func_80075358': 'boot/func_80075358_port.c', 'func_800753B4': 'boot/func_800753B4_port.c',
    'func_800752AC': 'boot/func_800752AC_port.c', 'func_80076354': 'boot/func_800752AC_port.c',
    'func_80076B58': 'platform/pe_plat_state.c (not the 80075358 file)',
    'func_8007A214': 'boot/func_8007A214_port.c', 'func_8007A244': 'no separate symbol (inlined in func_8007A214)',
    'func_8007C544': 'boot/func_8007A214_port.c (pe_plat_cd_stream_set_range)',
    'func_80071944': 'platform/pe_plat_gpu_address.c', 'func_80071964': 'platform/pe_plat_gpu_address.c',
    'func_80071994': 'platform/pe_plat_gpu_address.c', 'func_80077A64': 'platform/pe_plat_gpu_address.c',
    'func_80077AA4': 'platform/pe_plat_gpu_address.c',
    'func_80074F44': 'platform/pe_libgpu.c', 'func_80075424': 'platform/pe_libgpu.c', 'func_800754E4': 'platform/pe_libgpu.c',
    'func_8010BD4C': 'boot/func_8010BD4C_port.c', 'func_8010C0D8': 'boot/func_8010BE3C_port.c',
    'func_8010BE3C': 'platform/pe_plat_mdec_reset.c (clean-room; boot file no longer binds it)'}
BATCH = {f: b for b, fs in V.BATCHES.items() for f in fs}
BATCH['func_8010BD4C'] = 'gpu2d'
vec = sys.argv[1]
cnt = Counter()
for b in V.BATCHES:
    try:
        for ln in open('%s/%s/%s.txt' % (vec, b, b)):
            cnt['func_' + ln.split()[0]] += 1
    except FileNotFoundError:
        pass
if len(sys.argv) > 2:
    for ln in open(sys.argv[2]):
        if not ln.startswith('#'):
            cnt['func_' + ln.split()[0]] += 1
print('| function | yaml offset/size | provider file(s) | batch | cases |')
print('|---|---|---|---|---|')
for f in sorted(PROVIDER, key=lambda x: (BATCH[x], x)):
    if f in V.OVL:
        off, size, ent = C.func_range(f, V.OVL_YAML, V.OVL_VRAM)
        loc = 'ovl_039F %#x/%#x' % (off, size)
    else:
        off, size, ent = C.func_range(f)
        loc = 'disc1 %#x/%#x' % (off, size)
    print('| %s | %s | %s | %s | %d |' % (f, loc, PROVIDER[f], BATCH[f], cnt.get(f, 0)))
