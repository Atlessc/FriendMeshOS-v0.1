#!/usr/bin/env bash

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -pedantic -fsanitize=undefined,bounds \
    -I"$ROOT/src" \
    "$ROOT/bin/friendmesh-storage-check.cpp" \
    "$ROOT/src/friendmesh/storage/FriendMeshRecordCodec.cpp" \
    -o "$TMP_DIR/friendmesh-storage-check"

"$TMP_DIR/friendmesh-storage-check"
