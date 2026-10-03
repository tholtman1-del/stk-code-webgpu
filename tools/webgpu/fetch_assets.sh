#!/bin/bash
# Exports the game assets (karts, tracks, textures, music...) from the
# official SVN repository into ../stk-assets, where package_data.py and the
# native build look for them. About 750 MB.
#
# svn ignores HTTPS_PROXY. Behind a proxy (e.g. a cloud session), set
# http-proxy-host/http-proxy-port (and ssl-authority-files for a custom CA)
# in the [global] section of ~/.subversion/servers.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
ASSETS="${STK_ASSETS:-$ROOT/../stk-assets}"
SVN=https://svn.code.sf.net/p/supertuxkart/code/stk-assets

mkdir -p "$ASSETS"
for dir in karts library models music sfx textures tracks; do
    if [ ! -d "$ASSETS/$dir" ]; then
        echo "Exporting $dir"
        svn export -q "$SVN/$dir" "$ASSETS/$dir.tmp"
        mv "$ASSETS/$dir.tmp" "$ASSETS/$dir"
    fi
done
echo "Assets ready in $ASSETS"
