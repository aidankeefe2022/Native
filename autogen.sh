#!/bin/sh
# Regenerate the build system after a fresh checkout.
set -e
cd "$(dirname "$0")"
autoreconf --install --force --warnings=all
echo "Now run ./configure && make"
