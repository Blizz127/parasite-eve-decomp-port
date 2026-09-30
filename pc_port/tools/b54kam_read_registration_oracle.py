#!/usr/bin/env python3
"""Independent B54K-AM oracle for func_80081314's registration prefix."""
from __future__ import annotations
import argparse, hashlib, os, pathlib, struct, subprocess

EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
PEIMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"

def require(ok: bool, msg: str) -> None:
    if not ok: raise SystemExit(f"FAIL: {msg}")

def u32(b: bytes, o: int) -> int: return struct.unpack_from("<I", b, o)[0]

def run(argv: list[str], code: int, env=None) -> str:
    p = subprocess.run(argv, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                       text=True, timeout=180, check=False, env=env)
    require(p.returncode == code, f"exit {p.returncode}, expected {code}\n{p.stdout[-2000:]}")
    return p.stdout

def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("peimg", type=pathlib.Path)
    ap.add_argument("exe", type=pathlib.Path)
    ap.add_argument("--tests", type=pathlib.Path)
    ap.add_argument("--port", type=pathlib.Path)
    ap.add_argument("--disc", type=pathlib.Path)
    a = ap.parse_args(); root = pathlib.Path(__file__).resolve().parents[2]
    peimg=a.peimg.read_bytes(); exe=a.exe.read_bytes()
    require(hashlib.sha1(peimg).hexdigest()==PEIMG_SHA1,"PE.IMG identity")
    require(hashlib.sha1(exe).hexdigest()==EXE_SHA1,"EXE identity")

    spans=((0x71B14,0x71BE8,"fe43d63bd26dd2279dbdc1c8858998cace3f0da9d7cd16172aa61b8d32a6ce42"),
           (0x6F8C8,0x6FC18,"5c9c7e3533b61fe6c52eacfa4cac2b7934a7cfbba68c55089109db8c8604ad80"),
           (0x72CC8,0x72CDC,"42f22e14a8047c7d2f9c841c34f5705b8cbd9064725ea4a8babc9023641ea80d"),
           (0x72CF0,0x72D14,"9743a63ceffa537f486fa3134ac61bc08a1a5103f6731bee9244908114f6123e"))
    for s,e,h in spans:
        require(hashlib.sha256(exe[s:e]).hexdigest()==h,f"retail span {s:x} identity")
    require(hashlib.sha256(exe[0x71B14:0x71B8C]).hexdigest()==
            "ec6afdca38a2662a48bf9f6fa9f97113a48c2ed75c9e8b814c69d69a0d77da35",
            "30-word registration prefix")
    # CdlReadS (0x1B) dispatches through the four-command path at 0x8007F2A4.
    require(exe[0x21DC:0x21E5]==b"CdlReadS\0" and
            u32(exe,0x249C+(0x1B-3)*4)==0x8007F2A4,
            "CdlReadS name/jump-table path")
    print("  OK retail: complete wrappers; 30-word registration prefix; CdlReadS path")

    src=(root/"pc_port/game/boot/func_80081314_port.c").read_text()
    libcd=(root/"pc_port/platform/pe_libcd.c").read_text()
    require("mode & 0x100u" in src and "mode & 0x20u" in src and
            "func_800824F0(0x8007C214u)" in src and
            "func_800824C8(0x800813E8u)" in src and
            "func_8007F0C8(mode & 0xFFu, location, 0x1Bu, 0u" in src and
            "func_8007F0C8_completion_selector" in libcd,
            "native registration prefix")
    tests=a.tests or root/"pc_port/build/pe-native-tests"
    port=a.port or root/"pc_port/build/parasite-eve-port"
    disc=a.disc or pathlib.Path(os.environ.get("PE_DISC1_BIN",(root/"local/pe_disc1.path").read_text().strip()))
    env=os.environ.copy(); env["PE_TEST_FILTER"]="B54KAM"
    out=run([str(tests)],0,env)
    require("B54KAM_read_registration_prefix... PASS" in out and "0 failed" in out,"focused controls")
    strict=run([str(port),"--headless","--disc-image",str(disc),"--strict-stubs"],1)
    require("func_8007F0C8_completion_selector" in strict and
            "called from: func_8007F0C8" in strict,"strict frontier")
    print("  OK native: registration plus real queue issue; strict boundary pre-delivery")
    print("\nB54K-AM read-registration oracle: PASS.")

if __name__ == "__main__": main()
