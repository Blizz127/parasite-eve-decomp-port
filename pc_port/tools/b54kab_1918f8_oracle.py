#!/usr/bin/env python3
"""Independent B54K-AB oracle for complete func_801918F8."""
from __future__ import annotations
import hashlib, os, pathlib, re, struct, subprocess, sys

SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"
FULL = "86c8721a16712363ff166438380acd4ca4b2f732b7c8aecb917a08298b050109"
PREFIX = "9f6476d633f517cd6e17fee8a76167180a9f87d320ecf0e62ef4e4f3b45114b1"
B, L = 0x03D2 * 0x800, 0x8018EFF0

def need(x: bool, m: str) -> None:
    if not x: raise SystemExit("FAIL: " + m)
def u32(b: bytes, o: int) -> int: return struct.unpack_from("<I", b, o)[0]
def run(a: list[str], rc: int, env=None) -> str:
    r = subprocess.run(a, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                       text=True, timeout=120, env=env)
    need(r.returncode == rc, f"exit {r.returncode}\n{r.stdout[-1500:]}")
    return r.stdout

def main() -> None:
    root = pathlib.Path(__file__).resolve().parents[2]
    p = pathlib.Path(sys.argv[1]).read_bytes()
    need(hashlib.sha1(p).hexdigest() == SHA1, "PE.IMG")
    body = p[B + 0x801918F8-L:B + 0x80191B64-L]
    need(len(body) == 155*4 and hashlib.sha256(body).hexdigest() == FULL and
         u32(body, 0x264) == 0x03E00008 and u32(body, 0x268) == 0,
         "function identity/return")
    ov = p[B:B+(0x457-0x3d2)*0x800]; enc = 0x0C06463E
    callers = [L+i for i in range(0,len(ov)-3,4) if u32(ov,i)==enc]
    need(callers == [0x80191F74,0x8019256C,0x8019257C,0x80192990], "callers")
    pref = p[B+0x801924F8-L:B+0x80192584-L]
    need(hashlib.sha256(pref).hexdigest() == PREFIX, "35-word caller prefix")
    print("  OK retail: 155 words, return, four callers, four SDK calls")
    src = (root/"pc_port/game/boot/func_801918F8_port.c").read_text()
    need("func_800749D8" in src and "func_80074924" in src and
         "(width * 2) / 3" in src and "m0360i" not in src, "native scope")
    env=os.environ.copy(); env["PE_TEST_FILTER"]="B54KY"
    tests=root/"pc_port/build/pe-native-tests"; port=root/"pc_port/build/parasite-eve-port"
    out=run([str(tests)],0,env)
    need(re.search(r"985 run, 2 passed, 0 failed, 983 skipped",out) is not None,"tests")
    disc=pathlib.Path(os.environ.get("PE_DISC1_BIN",(root/"local/pe_disc1.path").read_text().strip()))
    strict=run([str(port),"--headless","--disc-image",str(disc),"--strict-stubs"],1)
    need("func_801924F8_80192584_cut" in strict and "called from: func_801924F8" in strict,"frontier")
    print("  OK native: wide/narrow branches; 35-word caller prefix; exact cut")
    print("\nB54K-AB func_801918F8 oracle: PASS.")
if __name__ == "__main__": main()
