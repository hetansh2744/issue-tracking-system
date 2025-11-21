#!/usr/bin/env bash
set -euo pipefail

# Where the sqlite source lives (relative to repo root)
SQLITE_SRC_DIR="third_party/sqlite-src"
# Where we will install it locally (no root)
SQLITE_PREFIX="$PWD/third_party/sqlite-build"

mkdir -p "$SQLITE_PREFIX"

cd "$SQLITE_SRC_DIR"

# Configure to install under third_party/sqlite-build
./configure --prefix="$SQLITE_PREFIX" --disable-tcl

# Build and install
make -j"$(nproc || echo 2)"
make install
