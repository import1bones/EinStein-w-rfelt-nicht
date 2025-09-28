#!/usr/bin/env python3
"""Map uncovered lines (from coverage_uncovered.py) to functions in a source file.

Usage: ./tools/map_uncovered_to_functions.py --file src/utils/GameSnapshot.cpp [--top N]

This script is heuristic-based: it finds function start/end by locating an opening
brace for a likely function signature and then matching braces. It then counts
how many uncovered lines fall inside each function span and outputs JSON with
functions sorted by uncovered counts.
"""
import argparse
import subprocess
import shlex
import json
import re
import sys

def run(cmd):
    p = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE,
                       stderr=subprocess.STDOUT, text=True)
    return p.returncode, p.stdout

def get_uncovered(file_path, cov_script='./tools/coverage_uncovered.py'):
    cmd = f'{shlex.quote(cov_script)} --file {shlex.quote(file_path)} --json'
    code, out = run(cmd)
    # coverage_uncovered prints the command it runs (lines starting with '> ')
    # before dumping JSON, so try to extract the JSON substring robustly.
    if not out:
        raise RuntimeError('coverage_uncovered produced no output')

    # Find first '{' which likely starts the JSON object
    first_brace = out.find('{')
    if first_brace == -1:
        raise RuntimeError(f'coverage_uncovered did not print JSON:\n{out}')

    json_text = out[first_brace:]
    try:
        return json.loads(json_text)
    except Exception:
        # Try to find the last '}' and trim
        last_brace = json_text.rfind('}')
        if last_brace == -1:
            raise RuntimeError(f'coverage_uncovered JSON parse failed:\n{out}')
        try:
            return json.loads(json_text[:last_brace+1])
        except Exception as e:
            raise RuntimeError(f'coverage_uncovered JSON parse failed: {e}\nFull output:\n{out}')

def find_functions(source_lines):
    # Heuristic: find lines that look like function signatures and an opening brace.
    # We'll look for '(' and ')' and not end with ';' and not start with keywords like if/for/while/switch/catch.
    funcs = []  # list of (name, start_line, open_brace_line, end_line)
    sig_rx = re.compile(r"([\w:\~<>]+)\s*\(")
    control_kw = {'if','for','while','switch','catch','else'}

    i = 0
    n = len(source_lines)
    while i < n:
        line = source_lines[i]
        stripped = line.strip()
        # skip preprocessor and comments
        if stripped.startswith('#') or stripped.startswith('//'):
            i += 1
            continue

        if '(' in line and ')' in line and not stripped.endswith(';'):
            # avoid control statements
            first_word = stripped.split()[0] if stripped.split() else ''
            if first_word in control_kw:
                i += 1
                continue

            # find opening brace on this or following lines
            brace_line = None
            if '{' in line:
                brace_line = i
            else:
                # look ahead a few lines (max 5) for '{'
                for j in range(i+1, min(i+6, n)):
                    if '{' in source_lines[j]:
                        brace_line = j
                        break

            if brace_line is not None:
                # try to extract a function name from the signature (token before '(')
                m = sig_rx.search(line)
                fname = None
                if m:
                    # token may include namespace or class::name; take last segment
                    token = m.group(1)
                    fname = token.split('::')[-1]
                else:
                    # fallback: try to take the word before '('
                    before = line.split('(')[0]
                    parts = before.strip().split()
                    fname = parts[-1] if parts else 'unknown'

                # now match braces from brace_line
                brace_count = 0
                end_line = brace_line
                for k in range(brace_line, n):
                    brace_count += source_lines[k].count('{')
                    brace_count -= source_lines[k].count('}')
                    end_line = k
                    if brace_count == 0:
                        break

                funcs.append({'name': fname, 'start': i+1, 'body_start': brace_line+1, 'end': end_line+1})
                i = end_line + 1
                continue

        i += 1

    return funcs

def map_uncovered_to_functions(uncovered_lines, funcs):
    # uncovered_lines is a list of integers
    func_map = {}
    for f in funcs:
        func_map[f['name']] = { 'start': f['start'], 'end': f['end'], 'uncovered_lines': [] }

    others = []
    for ln in uncovered_lines:
        matched = False
        for f in funcs:
            if ln >= f['start'] and ln <= f['end']:
                func_map[f['name']]['uncovered_lines'].append(ln)
                matched = True
                break
        if not matched:
            others.append(ln)

    # convert to list with counts
    result = []
    for name, data in func_map.items():
        lines = sorted(data['uncovered_lines'])
        result.append({
            'name': name,
            'start': data['start'],
            'end': data['end'],
            'uncovered_count': len(lines),
            'uncovered_lines': lines,
        })

    return result, sorted(others)

def compress_ranges(lines):
    if not lines:
        return []
    lines = sorted(set(lines))
    ranges = []
    start = prev = lines[0]
    for n in lines[1:]:
        if n == prev + 1:
            prev = n
            continue
        ranges.append([start, prev])
        start = prev = n
    ranges.append([start, prev])
    return ranges

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--file', required=True)
    parser.add_argument('--top', type=int, default=10, help='Top N functions to print')
    args = parser.parse_args()

    data = get_uncovered(args.file)
    uncovered_lines = data.get('uncovered_lines', [])

    with open(args.file, 'r') as f:
        src_lines = f.readlines()

    funcs = find_functions(src_lines)
    mapped, others = map_uncovered_to_functions(uncovered_lines, funcs)

    # sort by uncovered_count desc
    mapped_sorted = sorted(mapped, key=lambda x: x['uncovered_count'], reverse=True)

    out = {
        'file': args.file,
        'total_uncovered_lines': len(uncovered_lines),
        'functions': mapped_sorted,
        'others_uncovered_lines': others,
    }

    # print top N in human readable form and then full JSON
    print(f"Top {args.top} functions by uncovered lines:")
    for fn in mapped_sorted[:args.top]:
        print(f"- {fn['name']} ({fn['start']}..{fn['end']}): {fn['uncovered_count']} lines")

    print('\nJSON output:')
    print(json.dumps(out, indent=2))

    return 0

if __name__ == '__main__':
    sys.exit(main())
