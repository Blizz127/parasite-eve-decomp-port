#!/usr/bin/env python3
"""Run a command, copy its combined output to a log, and SIGKILL it when a
line matches a pattern (interruption-recovery coverage: a hard kill with no
clean shutdown, as when the player's machine loses power or the process is
killed).

Usage:
  pe_run_kill_on.py --log LOG --pattern REGEX [--after-lines N] -- CMD...

--after-lines N waits for N more output lines after the match before the
kill (0: kill at the matching line). Prints one KILL_ON line saying whether
the pattern was seen and the kill sent, then exits with the child's status
(-9 when killed, as 137).
"""
import argparse
import os
import re
import signal
import subprocess
import sys


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--log", required=True)
    ap.add_argument("--pattern", required=True)
    ap.add_argument("--after-lines", type=int, default=0)
    ap.add_argument("cmd", nargs=argparse.REMAINDER)
    a = ap.parse_args()
    cmd = a.cmd[1:] if a.cmd[:1] == ["--"] else a.cmd
    if not cmd:
        ap.error("no command")
    pat = re.compile(a.pattern)
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    seen, killed, left = False, False, None
    with open(a.log, "wb") as log:
        for raw in proc.stdout:
            log.write(raw)
            if killed:
                continue
            if not seen and pat.search(raw.decode("utf-8", "replace")):
                seen, left = True, a.after_lines
            elif seen:
                left -= 1
            if seen and left <= 0:
                os.kill(proc.pid, signal.SIGKILL)
                killed = True
                log.flush()
    rc = proc.wait()
    print("KILL_ON pattern_seen=%d killed=%d child_rc=%d" % (seen, killed, rc))
    return 128 + signal.SIGKILL if rc == -signal.SIGKILL else (rc if rc >= 0 else 128 - rc)


if __name__ == "__main__":
    sys.exit(main())
