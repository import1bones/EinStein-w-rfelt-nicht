#!/usr/bin/env python3
"""Print uncovered line ranges and coverage metrics for a single source file.

Usage: ./tools/coverage_uncovered.py --file src/utils/GameSnapshot.cpp

This script runs llvm-cov show to get annotated source with execution counts,
parses lines with 0 executions and compresses them into human-readable ranges.
It also calls llvm-cov report and extracts the line/branch coverage numbers for
the file.
"""
import argparse
import subprocess
import shlex
import sys
import re
import os
import json

def run(cmd):
    print(f"> {cmd}")
    p = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE,
                       stderr=subprocess.STDOUT, text=True, check=False)
    return p.returncode, p.stdout

def parse_annotated(output):
    # capture lines like: '   75|      5|std::string...'
    uncovered = []
    line_rx = re.compile(r"^\s*(\d+)\|\s*([0-9]+)?\|.*$")
    for ln in output.splitlines():
        m = line_rx.match(ln)
        if not m:
            continue
        lineno = int(m.group(1))
        count = m.group(2)
        if count is None:
            # non-executable or not instrumented; skip
            continue
        if count == '0':
            uncovered.append(lineno)
    return uncovered

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
        ranges.append((start, prev))
        start = prev = n
    ranges.append((start, prev))
    return ranges

def extract_report_for_file(report_output, filename):
    # The llvm-cov report prints a table with the filename in the first column.
    # We'll find the line that starts with the filename (or contains it) and
    # parse the columns: Cover (Lines %), Functions, Branches etc.
    lines = report_output.splitlines()
    for line in lines:
        if filename in line:
            # Try to find percentages in the line
            pct_rx = re.compile(r"(\d+\.\d+)%")

            pcts = pct_rx.findall(line)
            # Return the raw line and any percentages found
            return line, pcts
    return None, []


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--file', required=True,
        help='Source file to inspect (relative to repo root)'
    )
    parser.add_argument(
        '--binary',
        default='build-coverage/EinStein_Game_tests',

        help='Instrumented test binary'
    )
    parser.add_argument(
        '--profdata',
        default='/tmp/EinStein_merged.profdata',
        help='Merged profdata file'
    )
    parser.add_argument(
        '--obj',
        default='build-coverage/libeinstein_lib.a',
        help='Instrumented object/library'
    )
    parser.add_argument(
        '--json', action='store_true', help='Output machine-readable JSON'

    )
    args = parser.parse_args()

    if not os.path.exists(args.profdata):
        print(
            f'profdata {args.profdata} not found; '
            'run the instrumented tests and merge first'
        )
        return 2

    # Run llvm-cov show to get annotated source
    cmd = (
        f'xcrun llvm-cov show {shlex.quote(args.binary)} '
        f'-instr-profile={shlex.quote(args.profdata)} '
        f'-object {shlex.quote(args.obj)} '
        f'-format=text {shlex.quote(args.file)}'
    )
    code, out = run(cmd)
    if code != 0:
        print('llvm-cov show failed or returned non-zero; output below:')
        print(out)
        return 3
    if code != 0:
        if args.json:
            result = {
                'file': args.file,
                'error': 'llvm-cov show failed',
                'stdout': out,
            }
            print(json.dumps(result))
            return 3
        else:
            print('llvm-cov show failed or returned non-zero; output below:')
            print(out)
            return 3

    uncovered_lines = parse_annotated(out)
    ranges = compress_ranges(uncovered_lines)

    print('\nCoverage summary for:', args.file)
    if not uncovered_lines:
        print(
            'All instrumented lines appear covered '
            '(no zero-hit lines found).'
        )
    else:
        total_uncovered = len(uncovered_lines)
        print(f'Total uncovered lines: {total_uncovered}')
        print('Uncovered ranges:')
        for s, e in ranges:
            if s == e:
                print(f'  {s}')
            else:
                print(f'  {s}-{e}')
    total_uncovered = len(uncovered_lines)
    # Prepare structured result
    result = {
        'file': args.file,
        'total_uncovered_lines': total_uncovered,
        'uncovered_ranges': [[s, e] for s, e in ranges],
        'uncovered_lines': uncovered_lines,
    }
    if not args.json:
        print('\nCoverage summary for:', args.file)
        if not uncovered_lines:
            print(
                'All instrumented lines appear covered '
                '(no zero-hit lines found).'
            )
        else:
            print(f'Total uncovered lines: {total_uncovered}')
            print('Uncovered ranges:')
            for s, e in ranges:
                if s == e:
                    print(f'  {s}')
                else:
                    print(f'  {s}-{e}')

    # Also get the report line for this file
    rcmd = (
        f'xcrun llvm-cov report {shlex.quote(args.binary)} '
        f'-instr-profile={shlex.quote(args.profdata)} '
        f'-object {shlex.quote(args.obj)}'
    )
    _, report_out = run(rcmd)
    rep_line, pcts = extract_report_for_file(report_out, args.file)
    print('\nllvm-cov report line for file:')
    if rep_line:
        print(rep_line)
        if pcts:
            print('Detected percent values in report:', ', '.join(pcts))
    else:
        print('No report line found for file (check path equivalence)')
    result['report_line'] = rep_line
    result['report_percentages'] = pcts

    if not args.json:
        print('\nllvm-cov report line for file:')
        if rep_line:
            print(rep_line)
            if pcts:
                print('Detected percent values in report:', ', '.join(pcts))
        else:
            print('No report line found for file (check path equivalence)')
    else:
        # JSON output only
        print(json.dumps(result, indent=2))

    return 0


if __name__ == '__main__':
    sys.exit(main())
