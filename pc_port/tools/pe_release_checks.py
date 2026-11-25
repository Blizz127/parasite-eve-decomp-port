#!/usr/bin/env python3
"""Render release check claims from pinned, explicit evidence; fail closed."""
import argparse
import hashlib
import json
import re
import shlex
from pathlib import Path


def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def read_source_pins(path):
    pins = {}
    for line in Path(path).read_text().splitlines():
        digest, name = line.split('  ', 1)
        if not re.fullmatch(r'[0-9a-f]{64}', digest) or name in pins:
            raise ValueError('invalid or duplicate source pin')
        pins[name] = digest
    if not pins:
        raise ValueError('source pins are empty')
    return pins


def directly_runs_binary(lines, binary_path):
    """Recognize direct exec or a simple env invocation; reject wrappers."""
    try:
        header = lines[:lines.index('TESTED_SOURCES_BEGIN')]
        cwd = next(line[4:] for line in header if line.startswith('CWD '))
        command = shlex.split(next(line[8:] for line in header if line.startswith('COMMAND ')))
        executable = Path(next(line[11:] for line in header if line.startswith('EXECUTABLE '))).resolve()
    except (StopIteration, ValueError, IndexError):
        return False
    wanted = Path(binary_path).resolve()
    if executable == wanted:
        return True
    if executable != Path('/usr/bin/env').resolve() or not command:
        return False
    args = command[1:]
    while args and (args[0] in ('-i', '--ignore-environment') or re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*=.*', args[0], re.S)):
        args = args[1:]
    if args and args[0] == '--':
        args = args[1:]
    if not args or '/' not in args[0]:
        return False
    return (Path(cwd) / args[0]).resolve() == wanted


def completed_runner_log(lines):
    """Require an enclosing runner footer, not a nested run's footer."""
    if (len(lines) < 4 or not lines[0].startswith('EVIDENCE_RUN_START ') or
        lines[-4] != 'RUN_EXIT_CODE 0' or
        not lines[-3].startswith('EVIDENCE_RUN_END ') or
        lines[-2:] != ['SOURCE_SNAPSHOT_STABLE', 'RUN_SUCCESS']):
        return False
    depth = 0
    for index, line in enumerate(lines):
        if line.startswith('EVIDENCE_RUN_START '):
            depth += 1
        elif line.startswith('EVIDENCE_RUN_END '):
            depth -= 1
            if depth < 0 or (depth == 0 and index != len(lines) - 3):
                return False
    return depth == 0


def enclosing_evidence_sections(lines, tested_sources_end):
    """Separate the runner header and trailer from its command output.

    Older runner logs have no output delimiters. Their canonical header can
    still be parsed, but their trailer cannot be independently separated from
    child output; callers must retain that provenance limitation.
    """
    i = tested_sources_end + 1
    if i < len(lines) and lines[i] == 'REFERENCE_SOURCES_BEGIN':
        try:
            i = lines.index('REFERENCE_SOURCES_END', i + 1) + 1
        except ValueError:
            raise ValueError('reference source block is incomplete')
    if i < len(lines) and lines[i] == 'SOURCE_EXECUTION_ATTESTED':
        i += 1
    while i < len(lines) and re.fullmatch(r'SHA256 [0-9a-f]{64}', lines[i]):
        i += 1
    header = set(lines[:i])
    if i < len(lines) and lines[i] == 'COMMAND_OUTPUT_BEGIN':
        ends = [index for index, line in enumerate(lines)
                if line == 'COMMAND_OUTPUT_END']
        if not ends or ends[-1] <= i or ends[-1] >= len(lines) - 4:
            raise ValueError('runner output boundaries are incomplete')
        return header, set(lines[ends[-1] + 1:]), True
    return header, set(lines), False  # Legacy runner: no output boundaries.


def render(manifest):
    candidate = manifest['candidate']
    source_commit = candidate['source_commit']
    if not re.fullmatch(r'[0-9a-f]{40}', source_commit):
        raise ValueError('candidate commit must be a full git hash')
    pin_file = candidate['source_pins_file']
    if sha(pin_file) != candidate['source_pins_sha256']:
        raise ValueError('source pin file hash mismatch')
    pins = read_source_pins(pin_file)
    binaries = candidate['binaries']
    if not binaries:
        raise ValueError('no candidate binary pins')
    for name, binary in binaries.items():
        if sha(binary['path']) != binary['sha256']:
            raise ValueError(f'candidate binary hash mismatch: {name}')
    groups = {'binary': [], 'source': [], 'earlier': []}
    seen = set()
    for check in manifest['checks']:
        name, kind = check['name'], check['kind']
        if not name or '\n' in name or name in seen or kind not in groups:
            raise ValueError('invalid or duplicate check')
        seen.add(name)
        evidence = check['evidence']
        if sha(evidence['log_path']) != evidence['log_sha256']:
            raise ValueError(f'log hash mismatch: {name}')
        marker = evidence['pass_marker']
        log_text = Path(evidence['log_path']).read_text(errors='replace')
        log_lines = set(log_text.splitlines())
        if not marker or marker not in log_lines:
            raise ValueError(f'pass marker missing: {name}')
        if kind == 'earlier':
            # Historical logs may predate the evidence runner. When a runner
            # exit is present, the last one belongs to the enclosing run;
            # nested negative controls may legitimately emit earlier exits.
            lines = log_text.splitlines()
            exits = [i for i, line in enumerate(lines)
                     if line.startswith('RUN_EXIT_CODE ')]
            if exits:
                last = exits[-1]
                if lines[last] != 'RUN_EXIT_CODE 0' or any(
                    line.startswith('SOURCE_OR_BINARY_CHANGED ')
                    for line in lines[last + 1:]
                ):
                    raise ValueError(f'earlier evidence reports a failed or unstable run: {name}')
            elif any(line.startswith('SOURCE_OR_BINARY_CHANGED ') for line in lines):
                raise ValueError(f'earlier evidence reports an unstable run: {name}')
            if (lines and lines[0].startswith('EVIDENCE_RUN_START ') and
                not completed_runner_log(lines)):
                raise ValueError(f'earlier evidence runner log is incomplete: {name}')
        if kind in ('binary', 'source'):
            lines = log_text.splitlines()
            if not completed_runner_log(lines):
                raise ValueError(f'run-time source stability attestation missing or incomplete: {name}')
            try:
                begin = lines.index('TESTED_SOURCES_BEGIN')
                end = lines.index('TESTED_SOURCES_END', begin + 1)
                recorded = dict(line.split('  ', 1)[::-1] for line in lines[begin + 1:end])
            except (ValueError, IndexError):
                raise ValueError(f'tested source block invalid: {name}')
            header_lines, trailer_lines, scoped_output = enclosing_evidence_sections(lines, end)
            protocol_v2 = begin == 5 and lines[4] == 'EVIDENCE_RUN_PROTOCOL 2'
            if protocol_v2 and not scoped_output:
                raise ValueError(f'runner output boundaries are missing: {name}')
            if end - begin - 1 != len(recorded) or any(
                not re.fullmatch(r'[0-9a-f]{64}', digest) or f'SOURCE_AFTER {digest}  {path}' not in trailer_lines
                for path, digest in recorded.items()
            ):
                raise ValueError(f'tested source hashes are incomplete or unstable: {name}')
        if kind == 'binary':
            target = check['binary']
            if target not in binaries or check['tested_sha256'] != binaries[target]['sha256']:
                raise ValueError(f'tested binary is not a candidate pin: {name}')
            if (f"SHA256 {check['tested_sha256']}" not in header_lines or
                f"BINARY_AFTER {check['tested_sha256']}  {Path(binaries[target]['path']).resolve()}" not in trailer_lines):
                raise ValueError(f'tested binary hash missing from evidence log: {name}')
            if not directly_runs_binary(lines, binaries[target]['path']):
                raise ValueError(f'evidence does not directly execute candidate binary: {name}')
            legacy_note = ('' if protocol_v2 and scoped_output else
                           '; legacy undelimited log, publisher binary comparison remains binding')
            groups[kind].append(f"  - {name} (binary {target}, SHA256 {check['tested_sha256']}{legacy_note})")
        elif kind == 'source':
            if 'SOURCE_EXECUTION_ATTESTED' not in header_lines:
                raise ValueError(f'source execution attestation missing: {name}')
            snapshot = check['tested_source_hashes']
            if not snapshot:
                raise ValueError(f'no tested sources recorded: {name}')
            if recorded != snapshot:
                raise ValueError(f'tested source block does not match evidence: {name}')
            if not protocol_v2 or not scoped_output:
                commit = check.get('source_commit', '')
                if not re.fullmatch(r'[0-9a-f]{40}', commit):
                    raise ValueError(f'legacy source evidence needs a full earlier commit: {name}')
                groups['earlier'].append(f'  - {name} (earlier result, source commit {commit}; legacy log lacks scoped execution provenance)')
                groups['earlier'].extend(f'      {digest}  {path}' for path, digest in sorted(snapshot.items()))
                continue
            changed = {path: digest for path, digest in snapshot.items() if pins.get(path) != digest}
            if changed:
                commit = check.get('source_commit', '')
                if not re.fullmatch(r'[0-9a-f]{40}', commit):
                    raise ValueError(f'pre-integration result needs a full base commit: {name}')
                groups['earlier'].append(f'  - {name} (pre-integration, working tree based on commit {commit}; tested source hashes differ from or are absent from shipped pins)')
                groups['earlier'].extend(f'      {digest}  {path}' for path, digest in sorted(changed.items()))
            else:
                groups[kind].append(f"  - {name} (ran on this build's sources before integration; source hashes = pins, {len(snapshot)} tested files)")
        else:
            commit = check['source_commit']
            if not re.fullmatch(r'[0-9a-f]{40}', commit):
                raise ValueError(f'earlier result needs a full commit: {name}')
            groups[kind].append(f'  - {name} (earlier result, source commit {commit})')
    if not seen:
        raise ValueError('no checks supplied')
    out = [f'CHECKS ON THIS EXACT BUILD (source commit {source_commit})']
    for kind, label in [('binary', 'Exact binary checks'), ('source', 'Source-pinned component checks')]:
        if groups[kind]:
            out += [label + ':', *groups[kind]]
    if not groups['binary'] and not groups['source']:
        out.append('  No checks with matching binary or source pins were supplied.')
    if groups['earlier']:
        out += ['', 'EARLIER RESULTS (not exact-build checks):', *groups['earlier']]
    return '\n'.join(out) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('evidence_json')
    args = parser.parse_args()
    print(render(json.loads(Path(args.evidence_json).read_text())), end='')


if __name__ == '__main__':
    main()
