#!/usr/bin/env python3
"""Check compile_commands.json input paths against an explicit hold list.

This checks only the named compilation units, not headers, generated inputs,
link inputs, command side effects, or completeness of the supplied manifest.
It does not authorize a build or alter any files.
"""
import argparse
import hashlib
import json
import os
import sys


def canonical(path, base):
    return os.path.realpath(os.path.join(base, path))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", help="existing compile_commands.json")
    parser.add_argument("--held", action="append", required=True,
                        help="exact held source path; repeat for each held file")
    parser.add_argument("--expected-sha256", required=True,
                        help="SHA256 of the previously identified manifest bytes")
    args = parser.parse_args()
    try:
        held = {canonical(path, os.getcwd()) for path in args.held}
        expected = args.expected_sha256.lower()
        if len(expected) != 64 or any(c not in '0123456789abcdef' for c in expected):
            raise ValueError("expected SHA256 must be 64 hexadecimal characters")
        with open(args.manifest, "rb") as handle:
            manifest_bytes = handle.read()
        actual = hashlib.sha256(manifest_bytes).hexdigest()
        if actual != expected:
            raise ValueError(f"manifest SHA256 mismatch: observed {actual}")
        entries = json.loads(manifest_bytes)
        if not isinstance(entries, list) or not entries:
            raise ValueError("manifest must be a nonempty array")
        found = set()
        for entry in entries:
            if not isinstance(entry, dict):
                raise ValueError("manifest entry must be an object")
            directory, source = entry.get("directory"), entry.get("file")
            if not isinstance(directory, str) or not os.path.isabs(directory):
                raise ValueError("entry directory must be an absolute path")
            if not isinstance(source, str) or not source:
                raise ValueError("entry file must be a nonempty path")
            path = canonical(source, directory)
            if path in held:
                found.add(path)
    except (OSError, ValueError, TypeError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2
    print(f"manifest SHA256 {actual}")
    print(f"checked {len(entries)} compilation-unit entries against {len(held)} held paths")
    if found:
        for path in sorted(found):
            print(f"HELD: {path}")
        return 1
    print("No explicitly listed held compilation units found; manifest completeness and other inputs are unverified.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
