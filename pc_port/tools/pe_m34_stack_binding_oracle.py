#!/usr/bin/env python3
"""Compare M34 native writer binding against original field calls.

Isolated captured inputs only: each field call uses its corresponding capture,
while original stack and native provenance carry between calls. This is not
a continuous whole-RAM/native/hardware comparison or a connected-state restore.
"""
import argparse
import hashlib
import inspect
import json
import os
import re
import struct
import subprocess
import tempfile
from pathlib import Path

import pe_battle_hud_oracle as m
from pe_m34_frame_device_probe import OFFSETS, source_image

NATIVE = r'''
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
void Trace_Direct(const char*s){(void)s;}
int main(int argc,char**argv){
 if(argc!=6)return 2;
 PE_RamInit();Bootstrap_Init();PE_Port_RunControlReset();HostFB_Init();
 int lo=atoi(argv[3]),hi=atoi(argv[4]);
 for(unsigned frame=60260;frame<=(unsigned)atoi(argv[5]);frame++){
  char path[1024];snprintf(path,sizeof(path),"%s/frame-%06u.bin",argv[1],frame);FILE*f=fopen(path,"rb");
  if(!f||fread(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE)return 3;fclose(f);
  if(lo>=0){PE_StoreU8(0x800B0DCEu,(uint8_t)lo);PE_StoreU8(0x800B0DCFu,(uint8_t)hi);}
  D_8009D250++;func_80065400();func_80035558_walk_cut();
  if(PE_Port_ShouldStop())return 4;
 }
 FILE*f=fopen(argv[2],"wb");if(!f)return 5;
 size_t n=fwrite(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f);
 return fclose(f)||n!=PE_RAM_SIZE?6:0;
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    parser.add_argument('--build-dir', type=Path, default=m.ROOT / 'pc_port/build')
    parser.add_argument('--report', type=Path)
    parser.add_argument('--end-frame',type=int,default=60270)
    args = parser.parse_args()
    assert args.end_frame>=60270
    files = [args.directory / f'frame-{frame:06d}.bin' for frame in range(60260, args.end_frame+1)]
    captures = [p.read_bytes() for p in files]
    assert all(len(r) == 0x200000 for r in captures)
    source = source_image()
    cases = [(-1, -1), (0, 0), (12, 87), (127, 127), (200, 250)]
    rows, seen_all = [], set()
    with tempfile.TemporaryDirectory(prefix='pe-m34-binding-') as directory:
        tmp = Path(directory)
        src, binary, output = tmp / 'native.c', tmp / 'native', tmp / 'output.bin'
        src.write_text(NATIVE)
        subprocess.run(['cc', '-O2', *[f'-I{m.ROOT}/pc_port/{x}' for x in
            ('include', 'platform', 'bootstrap', 'src', '.')], str(src),
            str(args.build_dir.resolve() / 'libpe_field_runtime.a'), '-o', str(binary)], check=True)
        for lo, hi in cases:
            seen, inputs, addresses = set(), [], []

            def watch(pc, regs, ram):
                seen.add(pc)
                a = pc & 0x1FFFFF
                assert ram[a:a + 4] == source[a:a + 4], ('instruction', hex(pc))
                if pc == 0x8018F434:
                    addresses.append(regs[6]&0x1FFFFF)
                    inputs.append([struct.unpack_from('<I', ram, (regs[29] + off) & 0x1FFFFF)[0]
                                   for off in OFFSETS])

            code = inspect.getsource(m.execute)
            anchor = '        w = struct.unpack_from'
            assert code.count(anchor) == 1
            code = code.replace(anchor, '        watch(pc,r,ram)\n' + anchor, 1)
            namespace = dict(m.__dict__, watch=watch)
            exec(code, namespace)
            stack, gte, scratch = None, {}, bytearray(1024)
            for raw in captures:
                ram = bytearray(raw)
                if stack is not None:
                    ram[0x1FE000:] = stack
                if lo >= 0:
                    ram[0xB0DCE], ram[0xB0DCF] = lo, hi
                namespace['execute'](ram, 0x8003F4D0, stop_at=(0x8003F4F8,),
                    scratchpad=scratch, initial_cop_data=dict(enumerate(gte.get('data', []))),
                    initial_cop_control=dict(enumerate(gte.get('control', []))),
                    final_gte=gte, instruction_budget=1000000)
                stack = ram[0x1FE000:]
                for pc in seen | {pc + 4 for pc in seen}:
                    a = pc & 0x1FFFFF
                    assert raw[a:a + 4] == source[a:a + 4], ('captured source', hex(pc))
            assert inputs
            result = subprocess.run([str(binary), str(args.directory.resolve()), str(output),
                str(lo), str(hi),str(args.end_frame)], env=dict(os.environ, PE_M34_STACK_TRACE='1'),
                text=True, capture_output=True)
            assert result.returncode == 0, result.stderr
            matches = re.findall(r'M34_STACK projectile .*mask=3F words=([0-9A-F,]+)', result.stderr)
            native_inputs = [[int(x, 16) for x in row.split(',')] for row in matches]
            assert native_inputs == inputs, (lo, hi, native_inputs, inputs,result.stderr)
            native = output.read_bytes()
            assert len(native) == 0x200000
            records=[]
            for address in addresses:
                original_record=ram[address:address+72]
                assert native[address:address+72]==original_record,('72-byte projectile',lo,hi,hex(address))
                records.append(dict(address=hex(address),hex=original_record.hex()))
            # Report other differences; they are not silently folded into the
            # initializer proof. Captured calls do not match the entire frame.
            differences = [i for i, (a, b) in enumerate(zip(native[:0x1FE000], ram[:0x1FE000])) if a != b]
            rows.append(dict(envelope=[lo, hi], retained=inputs, pcs=len(seen),
                projectiles=records, other_ram_difference_count=len(differences),
                other_ram_difference_addresses=[hex(x) for x in differences], native_trace=result.stderr))
            seen_all.update(seen)
            print('PASS envelope', (lo, hi), 'retained', [[hex(x) for x in row] for row in inputs],
                  '72-byte projectiles; other RAM differences', len(differences), flush=True)
    report = dict(scope=__doc__, captures=[dict(name=p.name, sha256=hashlib.sha256(r).hexdigest())
        for p, r in zip(files, captures)], cases=rows, pcs=len(seen_all))
    if args.report:
        args.report.write_text(json.dumps(report, indent=2) + '\n')
    print('PASS', len(rows), 'original/native writer histories;', len(seen_all), 'PCs', flush=True)


if __name__ == '__main__':
    main()
