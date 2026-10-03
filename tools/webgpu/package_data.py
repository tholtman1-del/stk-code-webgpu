#!/usr/bin/env python3
"""Packs the game data for the browser build.

Usage:
    python3 tools/webgpu/package_data.py [--assets ../stk-assets] [--out build-web/stk/bin]

Writes stk-data.bin (all files concatenated) and stk-data.json (a manifest of
[path, offset, size] entries) to the output directory. index.html downloads
both and writes the files into the in-memory file system before main():
data/ goes to /stk/data and the assets to /stk/assets.
"""
import argparse
import json
import os
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))

# Not needed in the browser: desktop packaging files
EXCLUDE_DIRS = {"data/po/.git"}
EXCLUDE_FILES = {"data/supertuxkart.icns", "data/SuperTuxKart-Info.plist",
                 "data/SuperTuxKart-Info-iOS.plist", "data/supertuxkart.desktop",
                 "data/net.supertuxkart.SuperTuxKart.metainfo.xml",
                 "data/optimize_data.sh"}
ASSET_DIRS = ["karts", "library", "models", "music", "sfx", "textures",
              "tracks"]


def walk(src, rel_prefix):
    for dirpath, dirnames, filenames in os.walk(src):
        rel_dir = os.path.relpath(dirpath, src)
        rel_dir = rel_prefix if rel_dir == "." else f"{rel_prefix}/{rel_dir}"
        dirnames[:] = sorted(d for d in dirnames if not d.startswith(".")
                             and f"{rel_dir}/{d}" not in EXCLUDE_DIRS)
        for name in sorted(filenames):
            rel = f"{rel_dir}/{name}"
            if not name.startswith(".") and rel not in EXCLUDE_FILES:
                yield os.path.join(dirpath, name), rel


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--assets", default=os.path.join(ROOT, "..", "stk-assets"))
    parser.add_argument("--out", default=os.path.join(ROOT, "build-web", "stk", "bin"))
    args = parser.parse_args()

    sources = list(walk(os.path.join(ROOT, "data"), "data"))
    for d in ASSET_DIRS:
        path = os.path.join(args.assets, d)
        if not os.path.isdir(path):
            sys.exit(f"Missing assets directory {path}")
        sources += walk(path, f"assets/{d}")

    os.makedirs(args.out, exist_ok=True)
    files = []
    offset = 0
    with open(os.path.join(args.out, "stk-data.bin"), "wb") as out:
        for src, rel in sources:
            with open(src, "rb") as f:
                data = f.read()
            out.write(data)
            files.append([rel, offset, len(data)])
            offset += len(data)
    with open(os.path.join(args.out, "stk-data.json"), "w") as f:
        json.dump({"size": offset, "files": files}, f, separators=(",", ":"))
    print(f"{len(files)} files, {offset / 1048576:.0f} MB in {args.out}/stk-data.bin")


if __name__ == "__main__":
    main()
