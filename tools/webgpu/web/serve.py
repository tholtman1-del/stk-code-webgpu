#!/usr/bin/env python3
"""Local development server for the browser build of SuperTuxKart.

Usage:
    python3 tools/webgpu/web/serve.py [BUILD_DIR] [--port 8080] [--bind 127.0.0.1]

BUILD_DIR is the directory holding supertuxkart.js and supertuxkart.wasm
(default: build-web/stk/bin). "/" serves the index.html next to this script and
every other path is served from BUILD_DIR. Open http://localhost:8080/ in a
browser with WebGPU; extra game arguments can be passed as
http://localhost:8080/?arg=--log=0&arg=--no-start-screen

Every response carries the COOP/COEP headers that make the page
cross-origin isolated, which pthreads (SharedArrayBuffer) need. WebGPU only
exists in secure contexts: use localhost, not a LAN address over plain http.
"""
import argparse
import functools
import http.server
import os
import sys

WEB_DIR = os.path.dirname(os.path.abspath(__file__))
INDEX = os.path.join(WEB_DIR, "index.html")
SOURCE_ROOT = os.path.normpath(os.path.join(WEB_DIR, "..", "..", ".."))


class Handler(http.server.SimpleHTTPRequestHandler):
    extensions_map = {
        **http.server.SimpleHTTPRequestHandler.extensions_map,
        ".wasm": "application/wasm",
        ".js": "text/javascript",
        ".mjs": "text/javascript",
        ".json": "application/json",
        ".data": "application/octet-stream",
        ".html": "text/html; charset=utf-8",
    }

    def translate_path(self, path):
        clean = path.split("?", 1)[0].split("#", 1)[0]
        if clean in ("/", "/index.html"):
            return INDEX
        return super().translate_path(path)

    def end_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cross-Origin-Resource-Policy", "same-origin")
        # Development server: always revalidate so rebuilds are picked up
        self.send_header("Cache-Control", "no-cache")
        super().end_headers()


def main():
    parser = argparse.ArgumentParser(
        description="Serve the SuperTuxKart browser build with COOP/COEP headers.")
    parser.add_argument("build_dir", nargs="?",
                        default=os.path.join(SOURCE_ROOT, "build-web", "stk", "bin"),
                        help="directory with supertuxkart.js/.wasm "
                             "(default: build-web/stk/bin)")
    parser.add_argument("--port", type=int, default=8080)
    parser.add_argument("--bind", default="127.0.0.1",
                        help="address to listen on (default: 127.0.0.1)")
    args = parser.parse_args()

    build_dir = os.path.abspath(args.build_dir)
    if not os.path.isdir(build_dir):
        sys.exit(f"Build directory not found: {build_dir}")
    for name in ("supertuxkart.js", "supertuxkart.wasm"):
        if not os.path.isfile(os.path.join(build_dir, name)):
            print(f"Warning: {name} not found in {build_dir}", file=sys.stderr)

    handler = functools.partial(Handler, directory=build_dir)
    server = http.server.ThreadingHTTPServer((args.bind, args.port), handler)
    host = "localhost" if args.bind in ("127.0.0.1", "0.0.0.0", "::", "") \
        else args.bind
    print(f"Serving {build_dir}")
    print(f"Open http://{host}:{server.server_address[1]}/")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
