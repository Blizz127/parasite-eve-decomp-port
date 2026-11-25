#!/usr/bin/env python3
"""Compare the ready channel-3 host probe to synthetic original-code outcomes."""
import json
import subprocess
import sys
from pathlib import Path

rows = json.loads(Path(sys.argv[1]).read_text())
request = ''.join(' '.join(f'{value:x}' for value in row[:7]) + '\n' for row in rows)
result = subprocess.run([sys.argv[2]], input=request, text=True, capture_output=True, check=True)
lines = result.stdout.splitlines()
expected = int(sys.argv[3]) if len(sys.argv) > 3 else 72
assert len(rows) == len(lines) == expected, (len(rows), len(lines), expected)
failures = []
for index, (row, line) in enumerate(zip(rows, lines)):
    parts = line.split()
    assert len(parts) == 7, line
    observed = [int(value, 16) for value in parts[2:]]
    if int(parts[0]) != index or int(parts[1]) != 0 or observed != row[7:]:
        failures.append((index, row[7:], observed, parts[1]))
print(f'original vs host ready channel-3 outcomes: {len(rows)} cases, {len(failures)} mismatches')
if failures:
    print(failures[:3])
    sys.exit(1)
