#!/usr/bin/env python3
"""Source-check independent field calls across a captured M34 firing window.

This discovers writers; it does not emulate the intervening frame history.
"""
import argparse,json,subprocess,sys
from pathlib import Path

def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('directory',type=Path)
 p.add_argument('--device-fragments',action='store_true',help='check original input/OTC prefix and outer draw/presentation tail with explicit device inputs instead of field calls')
 args=p.parse_args()
 files=sorted(args.directory.glob('frame-*.bin'));assert files,'no frame captures'
 frames=[int(f.stem.split('-')[1]) for f in files]
 assert frames==list(range(frames[0],frames[-1]+1)),'missing frame captures'
 if args.device_fragments:
  from pe_m34_frame_device_probe import probe,source_image
  source=source_image();rows=[]
  for f in files:
   for mode in ('prefix','tail'):
    row=probe(f,source,mode,1024,1)
    assert 'error' not in row,(f,mode,row.get('error'))
    assert not any(any(d['writers']) for d in row['dependencies']),(f,mode,row['dependencies'])
    rows.append(row)
   print(f.name,'prefix/tail source checked; no watched writes',flush=True)
  report={'scope':'Independent prefix and tail probes, explicit device inputs and BIOS contracts; not full hardware/BIOS stack proof.','rows':rows}
  target=args.directory/'independent-device-frame-writers.json'
  target.write_text(json.dumps(report,indent=2)+'\n')
  print('PASS',len(rows),'probes',target,flush=True)
  return
 rows=[]
 probe=Path(__file__).with_name('pe_m34_effect_stack_probe.py')
 for f in files:
  assert f.stat().st_size==0x200000,('incomplete capture',str(f))
  result=subprocess.run([sys.executable,str(probe),'--frame','--counter-prefix',
                         '--watch-entry-sp','0x801FEEE8',str(f)],capture_output=True,text=True)
  if result.returncode:
   print(f.name,result.stderr,flush=True);raise SystemExit(result.returncode)
  row=json.loads(result.stdout);row['capture']=f.name;rows.append(row)
  changed=[(x['offset'],hex(x['value']),sorted(set(v[1] for v in x['writers'] if v)))
           for x in row['dependencies'] if any(x['writers'])]
  print(f.name,'pcs',row['prefix_pcs'],'writers',changed,flush=True)
 report={'scope':'Independent original counter/mailbox/field calls on present-hook captures; no intervening input, outer drawing or presentation history emulated. Supplied initial CPU stack and GTE state.','rows':rows}
 (args.directory/'independent-field-writers.json').write_text(json.dumps(report,indent=2)+'\n')
 print('PASS',len(rows),'source-checked independent field probes',flush=True)
if __name__=='__main__':main()
