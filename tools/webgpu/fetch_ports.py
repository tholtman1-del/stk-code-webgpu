#!/usr/bin/env python3
"""Fills the Emscripten port cache from git clones of the ports' tags.

Some environments (e.g. cloud sessions) can git-clone public GitHub
repositories but cannot download GitHub archives or release assets, which is
how Emscripten fetches its ports. Each port is cloned at the tag its archive
is made from and marked as unpacked, so emcc and embuilder use it as is.
"""
import os
import re
import shutil
import subprocess
import sys

PORTS = sys.argv[1:] or ["sdl2", "zlib", "freetype", "harfbuzz", "ogg", "vorbis"]

emscripten = os.path.dirname(shutil.which("emcc") or sys.exit("emcc not in PATH"))
sys.path.insert(0, emscripten)
from tools import cache  # noqa: E402
from tools.ports import ports_by_name  # noqa: E402


class GotURL(Exception):
    pass


class RecordingPorts:
    def fetch_project(self, name, url, sha512hash=None):
        raise GotURL(url)


def port_url(name):
    try:
        ports_by_name[name].get(RecordingPorts(), None, None)
    except GotURL as e:
        return str(e)
    sys.exit(f"{name}: could not determine download URL")


def clone_spec(url):
    """Returns (repository, tag, top-level directory of the archive)."""
    m = re.match(r"https://github\.com/([^/]+/([^/]+))/releases/download/([^/]+)/(.+?)\.(zip|tar\.\w+)$", url)
    if m:
        return m.group(1), m.group(3), m.group(4)
    m = re.match(r"https://github\.com/([^/]+/([^/]+))/archive/(?:refs/tags/)?(.+?)\.(zip|tar\.\w+)$", url)
    if m:
        tag = m.group(3)
        # GitHub drops the "v" of version tags in the archive's directory name
        version = tag[1:] if re.match(r"v\d", tag) else tag
        return m.group(1), tag, f"{m.group(2)}-{version}"
    sys.exit(f"unsupported port URL: {url}")


for name in PORTS:
    url = port_url(name)
    port_dir = os.path.join(cache.get_path("ports"), name)
    marker = os.path.join(port_dir, ".emscripten_url")
    if os.path.exists(marker) and open(marker).read().strip() == url:
        print(f"{name}: up to date")
        continue
    repo, tag, subdir = clone_spec(url)
    print(f"{name}: cloning {repo} at {tag}")
    tmp = port_dir + ".tmp"
    shutil.rmtree(tmp, ignore_errors=True)
    subprocess.run(["git", "-c", "advice.detachedHead=false", "clone", "-q",
                    "--depth", "1", "--branch", tag,
                    f"https://github.com/{repo}", os.path.join(tmp, subdir)],
                   check=True)
    shutil.rmtree(os.path.join(tmp, subdir, ".git"))
    with open(os.path.join(tmp, ".emscripten_url"), "w") as f:
        f.write(url + "\n")
    shutil.rmtree(port_dir, ignore_errors=True)
    os.replace(tmp, port_dir)
    shutil.rmtree(os.path.join(cache.get_path("ports-builds"), name),
                  ignore_errors=True)
