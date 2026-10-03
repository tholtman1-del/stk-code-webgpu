#!/usr/bin/env python3
"""Writes tools/webgpu/core_files.txt from a boot_test.mjs log.

The files streamed while the game starts are better downloaded with the core
package than fetched one by one. Boot to the main menu with streaming on and
pass the log:

    node tools/webgpu/web/boot_test.mjs http://localhost:8080/ 40000 boot.png \\
        | python3 tools/webgpu/update_core_list.py
    python3 tools/webgpu/package_data.py
"""
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
OUT = os.path.join(ROOT, "tools", "webgpu", "core_files.txt")


def main():
    paths = set()
    for line in sys.stdin:
        m = re.search(r"Streamed /stk/(\S+) \(", line)
        if m:
            paths.add(m.group(1))
    old = set()
    if os.path.exists(OUT):
        with open(OUT) as f:
            old = {l.strip() for l in f if l.strip() and not l.startswith("#")}
    paths |= old
    with open(OUT, "w") as f:
        f.write("# Streamed files read before the main menu shows, packed in the\n"
                "# core package by package_data.py (update_core_list.py)\n")
        for p in sorted(paths):
            f.write(p + "\n")
    print(f"{len(paths)} files ({len(paths - old)} new) in {OUT}")


if __name__ == "__main__":
    main()
