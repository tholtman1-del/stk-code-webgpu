#!/usr/bin/env python3
"""Records which streamed files each track reads, for the track bundles of
package_data.py (tools/webgpu/track_deps.json).

Usage (serve.py running on port 8080, streaming package built):
    python3 tools/webgpu/record_track_deps.py [TRACK...]

Starts a race on every track (or the given ones) in headless Chromium with
boot_test.mjs and keeps the shared files (textures, library objects, music)
that were streamed while it loaded; the track's own directory is always
bundled. Takes about half a minute per track.
"""
import json
import os
import re
import subprocess
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
ASSETS = os.environ.get("STK_ASSETS", os.path.join(ROOT, "..", "stk-assets"))
OUT = os.path.join(ROOT, "tools", "webgpu", "track_deps.json")
BOOT_TEST = os.path.join(ROOT, "tools", "webgpu", "web", "boot_test.mjs")
URL = ("http://localhost:8080/?nobundles&arg=--no-start-screen&arg=--race-now"
       "&arg=--track={}&arg=--numkarts=1&arg=--laps=1")
WAIT_MS = 25000


def record(track):
    shot = os.path.join("/tmp", f"stk-deps-{track}.png")
    try:
        r = subprocess.run(["node", BOOT_TEST, URL.format(track), str(WAIT_MS),
                            shot], capture_output=True, text=True,
                           timeout=WAIT_MS / 1000 + 120)
        log = r.stdout + r.stderr
    except subprocess.TimeoutExpired as e:
        log = (e.stdout or b"").decode() if isinstance(e.stdout, bytes) else (e.stdout or "")
    own = f"assets/tracks/{track}/"
    deps = set()
    for m in re.finditer(r"Streamed /stk/(\S+) \(", log):
        path = m.group(1)
        if not path.startswith(own):
            deps.add(path)
    return sorted(deps)


def main():
    tracks = sys.argv[1:] or sorted(
        d for d in os.listdir(os.path.join(ASSETS, "tracks"))
        if os.path.isfile(os.path.join(ASSETS, "tracks", d, "track.xml")))
    data = {}
    if os.path.exists(OUT):
        with open(OUT) as f:
            data = json.load(f)
    for i, track in enumerate(tracks):
        deps = record(track)
        data[track] = deps
        print(f"[{i + 1}/{len(tracks)}] {track}: {len(deps)} shared files",
              flush=True)
        with open(OUT, "w") as f:
            json.dump(data, f, indent=1, sort_keys=True)
    print(f"Wrote {OUT}")


if __name__ == "__main__":
    main()
