#!/usr/bin/env python3
"""Run a scratch check with explicit before/after source and binary hashes."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
from datetime import datetime, timezone


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def parse_source(value):
    if '=' in value:
        name, raw = value.split('=', 1)
    else:
        name = raw = value
    if not name or '\n' in name or '\r' in name:
        raise ValueError('invalid source label')
    path = Path(raw).resolve(strict=True)
    if not path.is_file():
        raise ValueError(f'not a source file: {raw}')
    return name, path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--log', required=True, help='new output file; never overwrites')
    parser.add_argument('--source', action='append', default=[], help='tested source PATH or PIN_NAME=PATH; repeat for each compiled or executed source')
    parser.add_argument('--reference-source', action='append', default=[], help='hashed reference PATH or PIN_NAME=PATH that is not asserted to be compiled or executed')
    parser.add_argument('--binary', action='append', default=[], help='tested binary path, if applicable')
    parser.add_argument('--sources-executed', action='store_true', help='attest the command compiles or directly executes the declared sources during this invocation')
    parser.add_argument('command', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    command = args.command
    if command and command[0] == '--':
        command = command[1:]
    if not command:
        parser.error('provide a command after --')
    if not args.source and not args.binary:
        parser.error('provide at least one tested source or binary')
    if args.sources_executed and not args.source:
        parser.error('--sources-executed needs at least one tested source')
    try:
        sources = [parse_source(item) for item in args.source]
        references = [parse_source(item) for item in args.reference_source]
        if len({name for name, _ in sources + references}) != len(sources) + len(references):
            raise ValueError('duplicate source/reference labels')
        binaries = [Path(item).resolve(strict=True) for item in args.binary]
        before = {name: digest(path) for name, path in sources}
        before_references = {name: digest(path) for name, path in references}
        before_binaries = {str(path): digest(path) for path in binaries}
        with open(args.log, 'x', encoding='utf-8', buffering=1) as out:
            print('EVIDENCE_RUN_START ' + datetime.now(timezone.utc).isoformat(), file=out)
            print('CWD ' + os.getcwd(), file=out)
            print('COMMAND ' + shlex.join(command), file=out)
            executable = shutil.which(command[0])
            print('EXECUTABLE ' + (str(Path(executable).resolve()) if executable else 'UNRESOLVED'), file=out)
            print('EVIDENCE_RUN_PROTOCOL 2', file=out)
            print('TESTED_SOURCES_BEGIN', file=out)
            for name, _ in sources:
                print(f'{before[name]}  {name}', file=out)
            print('TESTED_SOURCES_END', file=out)
            if references:
                print('REFERENCE_SOURCES_BEGIN', file=out)
                for name, _ in references:
                    print(f'{before_references[name]}  {name}', file=out)
                print('REFERENCE_SOURCES_END', file=out)
            if args.sources_executed:
                print('SOURCE_EXECUTION_ATTESTED', file=out)
            for value in before_binaries.values():
                print('SHA256 ' + value, file=out)
            print('COMMAND_OUTPUT_BEGIN', file=out)
            out.flush()
            try:
                result = subprocess.run(command, stdout=out, stderr=subprocess.STDOUT, check=False)
                rc = result.returncode
            except OSError as exc:
                print('RUN_LAUNCH_ERROR ' + str(exc), file=out)
                rc = 127
            # The child may leave its last line unterminated.
            print('\nCOMMAND_OUTPUT_END', file=out)
            changed = []
            for name, path in sources:
                try:
                    after = digest(path)
                except OSError:
                    after = 'UNREADABLE'
                print(f'SOURCE_AFTER {after}  {name}', file=out)
                if after != before[name]:
                    changed.append(name)
            for name, path in references:
                try:
                    after = digest(path)
                except OSError:
                    after = 'UNREADABLE'
                print(f'REFERENCE_AFTER {after}  {name}', file=out)
                if after != before_references[name]:
                    changed.append(name)
            for raw, value in before_binaries.items():
                try:
                    after = digest(Path(raw))
                except OSError:
                    after = 'UNREADABLE'
                print(f'BINARY_AFTER {after}  {raw}', file=out)
                if after != value:
                    changed.append(raw)
            # Keep protocol-2 header positions and child-output delimiters
            # unchanged for existing release-evidence readers. These records
            # map the pre-run hashes to their resolved paths outside child output.
            for name, path in sources:
                print('SOURCE_INPUT ' + json.dumps({'label': name, 'path': str(path), 'sha256': before[name]}, sort_keys=True), file=out)
            for name, path in references:
                print('REFERENCE_INPUT ' + json.dumps({'label': name, 'path': str(path), 'sha256': before_references[name]}, sort_keys=True), file=out)
            for path, value in before_binaries.items():
                print('BINARY_INPUT ' + json.dumps({'path': path, 'sha256': value}, sort_keys=True), file=out)
            print(f'RUN_EXIT_CODE {rc}', file=out)
            print('EVIDENCE_RUN_END ' + datetime.now(timezone.utc).isoformat(), file=out)
            if changed:
                print('SOURCE_OR_BINARY_CHANGED ' + ', '.join(changed), file=out)
                return 1
            if rc == 0:
                print('SOURCE_SNAPSHOT_STABLE', file=out)
                print('RUN_SUCCESS', file=out)
                return 0
            return rc if 0 < rc < 126 else 1
    except (OSError, ValueError) as exc:
        print(f'evidence runner: {exc}', file=sys.stderr)
        return 2


if __name__ == '__main__':
    sys.exit(main())
