#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
NANOPB_DIR="$ROOT_DIR/.pio/libdeps/t-deck-tft/Nanopb"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

if [[ ! -f "$NANOPB_DIR/pb_decode.c" ]]; then
    echo "Missing pinned Nanopb runtime. Run: pio run -e t-deck-tft" >&2
    exit 1
fi

for source in pb_common.c pb_decode.c pb_encode.c; do
    cc -std=c11 -I"$NANOPB_DIR" -c "$NANOPB_DIR/$source" -o "$TMP_DIR/${source%.c}.o"
done

c++ -std=c++17 -Wall -Wextra -Werror -I"$ROOT_DIR/src" -I"$ROOT_DIR/src/mesh/generated" -I"$NANOPB_DIR" \
    "$ROOT_DIR/bin/friendmesh-protocol-check.cpp" \
    "$ROOT_DIR/src/friendmesh/generated/friendmesh.pb.cpp" \
    "$ROOT_DIR/src/friendmesh/protocol/FriendMeshProtocol.cpp" \
    "$ROOT_DIR/src/friendmesh/protocol/FriendMeshReplayWindow.cpp" \
    "$TMP_DIR"/*.o -o "$TMP_DIR/friendmesh-protocol-check"

"$TMP_DIR/friendmesh-protocol-check"
