#!/usr/bin/env python3
"""Packs the game data for the browser build.

Usage:
    python3 tools/webgpu/package_data.py [--assets ../stk-assets] [--out build-web/stk/bin]

Writes stk-data.bin (the core files concatenated) and stk-data.json (a
manifest of [path, offset, size] core entries and [path, size] streamed
entries) to the output directory. index.html downloads both and writes the
files into the in-memory file system before main(): data/ goes to /stk/data
and the assets to /stk/assets.

Streamed files (big textures, models, music, translations, replays) are put
in stk-files/ as single files and only fetched when the game opens them, so
the game starts after downloading the core (~40 MB) instead of everything.
--no-streaming puts everything in the core package.
"""
import argparse
import json
import os
import shutil
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

# Read at startup when STK scans karts, tracks and music: always in the core
CORE_EXTENSIONS = {".xml", ".music", ".txt"}
# Smaller files are not worth a request of their own
STREAM_MIN_SIZE = 4096
# Only these data/ directories are streamed, the rest is needed for the menus
STREAMED_DATA_DIRS = ("data/po/", "data/replay/")
# Read completely at startup (all karts are loaded for the kart selection)
CORE_ASSET_DIRS = ("assets/karts/", "assets/models/", "assets/sfx/")
# Other files read before the main menu shows (shared textures of karts and
# powerups, track screenshots), see update_core_list.py
CORE_LIST = os.path.join(ROOT, "tools", "webgpu", "core_files.txt")
LAZY_DIR = "stk-files"


def is_streamed(rel, size, core_list):
    if size < STREAM_MIN_SIZE or rel in core_list:
        return False
    if rel.startswith(CORE_ASSET_DIRS):
        return False
    if os.path.splitext(rel)[1].lower() in CORE_EXTENSIONS:
        return False
    if rel.startswith("data/"):
        return rel.startswith(STREAMED_DATA_DIRS)
    return True


def link_or_copy(src, dst):
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    try:
        os.link(src, dst)
    except OSError:
        shutil.copyfile(src, dst)


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
    parser.add_argument("--no-streaming", action="store_true",
                        help="put every file in stk-data.bin")
    args = parser.parse_args()

    sources = list(walk(os.path.join(ROOT, "data"), "data"))
    for d in ASSET_DIRS:
        path = os.path.join(args.assets, d)
        if not os.path.isdir(path):
            sys.exit(f"Missing assets directory {path}")
        sources += walk(path, f"assets/{d}")

    core_list = set()
    if os.path.exists(CORE_LIST):
        with open(CORE_LIST) as f:
            core_list = {l.strip() for l in f
                         if l.strip() and not l.startswith("#")}
    os.makedirs(args.out, exist_ok=True)
    lazy_root = os.path.join(args.out, LAZY_DIR)
    shutil.rmtree(lazy_root, ignore_errors=True)
    files = []
    lazy = []
    offset = 0
    lazy_size = 0
    with open(os.path.join(args.out, "stk-data.bin"), "wb") as out:
        for src, rel in sources:
            size = os.path.getsize(src)
            if not args.no_streaming and is_streamed(rel, size, core_list):
                link_or_copy(src, os.path.join(lazy_root, rel))
                lazy.append([rel, size])
                lazy_size += size
                continue
            with open(src, "rb") as f:
                data = f.read()
            out.write(data)
            files.append([rel, offset, len(data)])
            offset += len(data)
    with open(os.path.join(args.out, "stk-data.json"), "w") as f:
        json.dump({"size": offset, "files": files, "lazyBase": LAZY_DIR + "/",
                   "lazy": lazy}, f, separators=(",", ":"))
    print(f"{len(files)} files, {offset / 1048576:.0f} MB in "
          f"{args.out}/stk-data.bin")
    if lazy:
        print(f"{len(lazy)} streamed files, {lazy_size / 1048576:.0f} MB in "
              f"{lazy_root}/")


if __name__ == "__main__":
    main()
