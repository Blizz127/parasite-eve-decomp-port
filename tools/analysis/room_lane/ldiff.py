#!/usr/bin/env python3
"""ldiff.py <target> <src> <vram> <size> [flags] : era_link_check + side-by-side disasm diff."""
import sys, os, subprocess, tempfile, contextlib, shutil, io, difflib, re
from pathlib import Path
ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools" / "analysis")); sys.path.insert(0, str(ROOT / "tools" / "build"))
work = ROOT / os.environ.get('ROOM_LANE_WORK', 'build/room_lane') / 'ldwork'
if work.exists(): shutil.rmtree(work)
work.mkdir(parents=True)
@contextlib.contextmanager
def fixed(*a, **k):
    yield str(work)
tempfile.TemporaryDirectory = fixed
import era_link_check as E
sys.argv = ["x", "--target"] + sys.argv[1:]
rc = E.main()
tgt, src, vram, size = sys.argv[2], sys.argv[3], int(sys.argv[4], 0), int(sys.argv[5], 0)
OD = str(E.HOST / "usr/bin/mipsel-linux-gnu-objdump")
def dis_elf():
    out = subprocess.run([OD, "-d", "--no-show-raw-insn", "-M", "no-aliases,reg-names=numeric", str(work / "x.elf")], capture_output=True, text=True, env=E.HOST_ENV).stdout
    return out
from overlay_targets import load_target  # noqa
t = load_target(tgt, root=ROOT)
blob = Path(t.retail).read_bytes() if Path(t.retail).is_absolute() else (ROOT / t.retail).read_bytes()
off = vram - t.vram + t.file_start
(work / "rom.bin").write_bytes(blob[off:off + size])
rom = subprocess.run([OD, "-D", "-b", "binary", "-m", "mips:3000", "-EL", "--no-show-raw-insn", "-M", "no-aliases,reg-names=numeric", f"--adjust-vma={vram:#x}", str(work / "rom.bin")], capture_output=True, text=True, env=E.HOST_ENV).stdout
def norm(txt, lim):
    res = []
    for l in txt.splitlines():
        m = re.match(r"\s*([0-9a-f]+):\s+(.*)$", l)
        if not m: continue
        a = int(m.group(1), 16)
        if a < vram or a >= vram + lim: continue
        res.append(re.sub(r"0x([0-9a-f]{8})", r"\1", re.sub(r"\s+", " ", m.group(2).split("<")[0]).strip()))
    return res
A = norm(rom, size); B = norm(dis_elf(), size + 0x40)
for l in difflib.unified_diff(A, B, "rom", "c", n=2, lineterm=""):
    print(l)
sys.exit(rc)
