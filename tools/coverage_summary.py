#!/usr/bin/env python3
"""Generate a compact coverage summary using llvm-profdata + llvm-cov.

Usage: ./tools/coverage_summary.py [--profdata /tmp/EinStein.profdata] [--binary build-coverage/EinStein_Game_tests]

The script will look for /tmp/EinStein_*.profraw if no profdata provided.
"""
import argparse
import glob
import os
import shlex
import subprocess
import sys

def run(cmd):
    print(f"> {cmd}")
    p = subprocess.run(
        cmd,
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )
    return p.returncode, p.stdout

def find_profraws(tmp_pattern='/tmp/EinStein_*.profraw'):
    return glob.glob(tmp_pattern)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--profdata', help='Path to existing .profdata file')
    parser.add_argument(
        '--binary',
        help='Path to instrumented test binary',
        default='build-coverage/EinStein_Game_tests'
    )
    parser.add_argument(
        '--obj',
        help='Path to instrumented object/library (libeinstein_lib.a)',
        default='build-coverage/libeinstein_lib.a'
    )
    parser.add_argument(
        '--sources',
        nargs='*',
        help='Source files or directories to report (default: src/)',
        default=['src']
    )
    args = parser.parse_args()

    profdata = args.profdata
    if not profdata:
        profraws = find_profraws()
        if not profraws:
            print(
                'No .profraw files found under /tmp. '
                'Run the instrumented tests first.'
            )
            return 2
        profdata = '/tmp/EinStein_merged.profdata'
        profraw_list = ' '.join(shlex.quote(p) for p in profraws)
        cmd = (
            f'xcrun llvm-profdata merge -sparse {profraw_list} '
            f'-o {shlex.quote(profdata)}'
        )
        code, out = run(cmd)
        if code != 0:
            print('Failed to merge profraw files:')
            print(out)
            return 3

    # Build file list for llvm-cov show
    file_args = []
    for s in args.sources:
        if os.path.isdir(s):
            for root, _, files in os.walk(s):
                for f in files:
                    if (
                        f.endswith('.cpp')
                        or f.endswith('.c')
                        or f.endswith('.h')
                        or f.endswith('.hpp')
                    ):
                        file_args.append(os.path.join(root, f))
        elif os.path.isfile(s):
            file_args.append(s)

    files_str = ' '.join(shlex.quote(p) for p in file_args)
    if not files_str:
        print('No source files found to report on.')
        return 4

    # Run llvm-cov report (text) to get per-file summary
    cmd = (
        f'xcrun llvm-cov report {shlex.quote(args.binary)} '
        f'-instr-profile={shlex.quote(profdata)} '
        f'-object {shlex.quote(args.obj)} '
        f'-path-equivalence="{os.getcwd()},{os.getcwd()}"'
    )
    code, out = run(cmd)
    if code != 0:
        print('llvm-cov report failed:')
        print(out)
        return 5

    print('\n=== llvm-cov report output ===\n')
    print(out)

    # Also show annotated coverage for files under src/
    # to help locate missing lines
    print('\n=== Per-file annotated coverage (first 80 lines) ===\n')
    for f in file_args:
        if f.startswith('src'):
            cmd = (
                f'xcrun llvm-cov show {shlex.quote(args.binary)} '
                f'-instr-profile={shlex.quote(profdata)} '
                f'-object {shlex.quote(args.obj)} '
                f'-format=text {shlex.quote(f)}'
            )
            code, out = run(cmd)
            if code == 0:
                print(f'--- {f} ---')
                # print only first 2000 chars to keep output reasonable
                print(out[:2000])
            else:
                print(f'Failed to show coverage for {f}:')
                print(out)

    return 0


if __name__ == '__main__':
    sys.exit(main())
