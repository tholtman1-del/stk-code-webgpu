#!/usr/bin/env python3
"""Packs the game data for the browser build.

Usage:
    python3 tools/webgpu/package_data.py [--assets ../stk-assets] [--out build-web/stk/bin]

Writes stk-data.N.bin (the core files concatenated, split in parts of at
most 24 MiB for static hosts with file size limits, downloaded in parallel)
and stk-data.json (a manifest of the parts, [path, offset, size] core entries
and [path, size] streamed entries) to the output directory. index.html downloads both and writes the
files into the in-memory file system before main(): data/ goes to /stk/data
and the assets to /stk/assets.

Streamed files (big textures, models, music, translations, replays) are put
in stk-files/ as single files and only fetched when the game opens them, so
the game starts after downloading the core (~40 MB) instead of everything.
--no-streaming puts everything in the core package.

Track bundles: when the game reads the first streamed file of a track, the
page fetches stk-bundles/TRACK.N.bin instead, with every streamed file of
the track directory and the shared files (textures, library objects, music)
it read when recorded by record_track_deps.py (track_deps.json) that few
other tracks read, so loading a track takes a few requests instead of about
a hundred. Shared files most tracks read are in the core package. The single
files stay in stk-files/ for everything else.
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
BUNDLE_DIR = "stk-bundles"
TRACK_DEPS = os.path.join(ROOT, "tools", "webgpu", "track_deps.json")
# Shared files read by this many tracks go in the core package, the ones read
# by at most BUNDLE_MAX_TRACKS are copied into each of those track bundles,
# the others stay single files (limits the duplication)
CORE_MIN_TRACKS = 10
BUNDLE_MAX_TRACKS = 4


def load_track_deps():
    """{track: [shared files]} and {shared file: number of tracks}"""
    deps = {}
    if os.path.exists(TRACK_DEPS):
        with open(TRACK_DEPS) as f:
            deps = json.load(f)
    users = {}
    for paths in deps.values():
        for path in paths:
            users[path] = users.get(path, 0) + 1
    return deps, users
# Cloudflare Pages allows 25 MiB per file, GitHub Pages 100 MB
PART_SIZE = 24 * 1024 * 1024


def is_streamed(rel, size, core_list, track_users):
    if size < STREAM_MIN_SIZE or rel in core_list:
        return False
    if track_users.get(rel, 0) >= CORE_MIN_TRACKS:
        return False
    if rel.startswith(CORE_ASSET_DIRS):
        return False
    if os.path.splitext(rel)[1].lower() in CORE_EXTENSIONS:
        return False
    if rel.startswith("data/"):
        return rel.startswith(STREAMED_DATA_DIRS)
    return True


def write_bundles(out, lazy, sources, deps, users):
    """Writes the track bundles, returns their manifest entries:
    {track: {"parts": [...], "files": [[path, part, offset, size], ...]}}"""
    bundle_root = os.path.join(out, BUNDLE_DIR)
    shutil.rmtree(bundle_root, ignore_errors=True)
    lazy_paths = {path for path, _ in lazy}
    tracks = {}
    for path in sorted(lazy_paths):
        parts = path.split("/")
        if parts[:2] == ["assets", "tracks"] and len(parts) > 3:
            tracks.setdefault(parts[2], []).append(path)
    for track, paths in deps.items():
        if track in tracks:
            tracks[track] += [p for p in paths if p in lazy_paths and
                              users[p] <= BUNDLE_MAX_TRACKS]
    if not tracks:
        return {}
    os.makedirs(bundle_root)
    manifest = {}
    total = 0
    for track, paths in sorted(tracks.items()):
        entries = []
        part_names = []
        part = None
        part_size = 0
        for path in paths:
            size = os.path.getsize(sources[path])
            if part is None or part_size + size > PART_SIZE:
                if part:
                    part.close()
                name = f"{BUNDLE_DIR}/{track}.{len(part_names)}.bin"
                part = open(os.path.join(out, name), "wb")
                part_names.append(name)
                part_size = 0
            with open(sources[path], "rb") as f:
                part.write(f.read())
            entries.append([path, len(part_names) - 1, part_size, size])
            part_size += size
            total += size
        part.close()
        manifest[track] = {"parts": part_names, "files": entries}
    print(f"{len(manifest)} track bundles, {total / 1048576:.0f} MB in "
          f"{bundle_root}/")
    return manifest


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
                        help="put every file in the core package")
    args = parser.parse_args()

    sources = list(walk(os.path.join(ROOT, "data"), "data"))
    for d in ASSET_DIRS:
        path = os.path.join(args.assets, d)
        if not os.path.isdir(path):
            sys.exit(f"Missing assets directory {path}")
        sources += walk(path, f"assets/{d}")

    deps, track_users = load_track_deps()
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
    for old in os.listdir(args.out):
        if old.startswith("stk-data.") and old.endswith(".bin"):
            os.remove(os.path.join(args.out, old))
    blob = bytearray()
    for src, rel in sources:
        size = os.path.getsize(src)
        if not args.no_streaming and is_streamed(rel, size, core_list,
                                                track_users):
            link_or_copy(src, os.path.join(lazy_root, rel))
            lazy.append([rel, size])
            lazy_size += size
            continue
        with open(src, "rb") as f:
            data = f.read()
        blob += data
        files.append([rel, offset, len(data)])
        offset += len(data)
    parts = []
    for i, start in enumerate(range(0, max(len(blob), 1), PART_SIZE)):
        name = f"stk-data.{i}.bin"
        with open(os.path.join(args.out, name), "wb") as out:
            out.write(blob[start:start + PART_SIZE])
        parts.append(name)
    bundles = {} if args.no_streaming else write_bundles(
        args.out, lazy, {rel: src for src, rel in sources}, deps, track_users)
    with open(os.path.join(args.out, "stk-data.json"), "w") as f:
        json.dump({"size": offset, "parts": parts, "partSize": PART_SIZE,
                   "files": files, "lazyBase": LAZY_DIR + "/",
                   "lazy": lazy, "bundles": bundles}, f,
                  separators=(",", ":"))
    print(f"{len(files)} files, {offset / 1048576:.0f} MB in "
          f"{len(parts)} parts (stk-data.N.bin)")
    if lazy:
        print(f"{len(lazy)} streamed files, {lazy_size / 1048576:.0f} MB in "
              f"{lazy_root}/")


if __name__ == "__main__":
    main()
