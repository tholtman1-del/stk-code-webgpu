#!/usr/bin/env python3
"""Assembles a directory that any static web host can serve.

Usage:
    python3 tools/webgpu/make_dist.py [--build build-web/stk/bin] [--out build-web/dist]

Run after building supertuxkart and package_data.py. The output holds the
page (index.html, coi-sw.js), the engine (supertuxkart.js/.wasm), the core
data parts, stk-files/ and stk-bundles/ (hard links where possible), plus:
- _headers: COOP/COEP and cache headers for Netlify and Cloudflare Pages
- .nojekyll: serve the files as they are on GitHub Pages
Hosts that ignore _headers (GitHub Pages) get the COOP/COEP headers from the
coi-sw.js service worker. Every file is below 25 MiB (Cloudflare's limit).
Upload the whole directory; the page must be served over https.
"""
import argparse
import os
import shutil
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
WEB = os.path.join(ROOT, "tools", "webgpu", "web")

HEADERS = """/*
  Cross-Origin-Opener-Policy: same-origin
  Cross-Origin-Embedder-Policy: require-corp
  Cross-Origin-Resource-Policy: same-origin

/stk-files/*
  Cache-Control: public, max-age=604800

/stk-bundles/*
  Cache-Control: public, max-age=604800
"""


def link_or_copy(src, dst):
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    try:
        os.link(src, dst)
    except OSError:
        shutil.copyfile(src, dst)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--build", default=os.path.join(ROOT, "build-web", "stk", "bin"))
    parser.add_argument("--out", default=os.path.join(ROOT, "build-web", "dist"))
    args = parser.parse_args()

    needed = ["supertuxkart.js", "supertuxkart.wasm", "stk-data.json"]
    for name in needed:
        if not os.path.isfile(os.path.join(args.build, name)):
            sys.exit(f"Missing {name} in {args.build}: build supertuxkart and "
                     "run package_data.py first")

    shutil.rmtree(args.out, ignore_errors=True)
    os.makedirs(args.out)
    for name in ("index.html", "coi-sw.js"):
        shutil.copyfile(os.path.join(WEB, name), os.path.join(args.out, name))
    count = 0
    biggest = 0
    for dirpath, _, filenames in os.walk(args.build):
        for name in filenames:
            src = os.path.join(dirpath, name)
            rel = os.path.relpath(src, args.build)
            # Only what the page loads (no worker or map leftovers)
            if not (rel.startswith("stk-files" + os.sep) or
                    rel.startswith("stk-bundles" + os.sep) or
                    name in needed or (name.startswith("stk-data.") and
                    name.endswith(".bin"))):
                continue
            link_or_copy(src, os.path.join(args.out, rel))
            count += 1
            biggest = max(biggest, os.path.getsize(src))
    with open(os.path.join(args.out, "_headers"), "w") as f:
        f.write(HEADERS)
    open(os.path.join(args.out, ".nojekyll"), "w").close()
    print(f"{count + 4} files in {args.out}, largest {biggest / 1048576:.1f} MB")


if __name__ == "__main__":
    main()
