#!/usr/bin/env python3
"""Doxygen input filter: preprocesses cvx facade headers with gcc -E, stripping system header content."""
import subprocess
import sys
import re
import os

filepath = os.path.abspath(sys.argv[1])
project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

result = subprocess.run(
    ["gcc", "-E", "-C", "-I", project_root, filepath],
    capture_output=True,
    text=True,
    cwd=project_root,
)

SYSTEM_PREFIXES = ("/usr", "/opt/homebrew", "/Library", "<built-in>", "<command-line>", "<command line>")

skip = False
for line in result.stdout.splitlines():
    m = re.match(r'^# \d+ "([^"]*)"', line)
    if m:
        fname = m.group(1)
        skip = any(fname.startswith(p) for p in SYSTEM_PREFIXES)
        continue
    if not skip:
        print(line)
