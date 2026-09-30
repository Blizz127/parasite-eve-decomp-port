#!/usr/bin/env python3
"""Attachment command sites in the inspected Day1 route and initial Day2 maps.

Inventory only: neither complete day membership nor reachable execution proof.
"""
import hashlib,json
from pe_day1_route_audit import EDGES,decode_token
from pe_day2_route_audit import ROOT,parse_field_table,extract_script_from_package,decode_script,find_disc,read_form1

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 names={decode_token(ex,t).rstrip(b'\0').decode() for a,_,b in EDGES for t in (a,b)}
 maps={int(n[1:5]) for n in names if n.startswith('M') and n[1:5].isdigit()}|{41,42,43,351}
 maps.discard(0);disc=find_disc(ROOT);result=[]
 for rec in parse_field_table(ex):
  if rec['map_id'] not in maps:continue
  meta=rec['meta'];package=read_form1(disc,1013+rec['start'],(meta&255)+(meta>>8&4095)+(meta>>20))
  off,raw=extract_script_from_package(package,meta);script=decode_script(raw);sites=[]
  for i,m in enumerate(script['modules']):
   for c in m['commands']:
    if c['opcode'] in (0xA4,0xC5,0xCC,0xD4,0xD5,0xD6):sites.append(dict(module=i,command=c))
  result.append(dict(map=rec['map_id'],sha256=script['sha256'],sites=sites))
 assert {r['map'] for r in result}==maps
 path=ROOT/'local/live/attachment-script-inventory.json';path.write_text(json.dumps(result,indent=2)+'\n')
 print(f'PASS {len(result)} inspected route/shared packages, {sum(len(r["sites"]) for r in result)} attachment sites')
 for r in result:
  if r['sites']:print(r['map'],[(x['module'],hex(x['command']['opcode']),hex(x['command']['offset'])) for x in r['sites']])
if __name__=='__main__':main()
