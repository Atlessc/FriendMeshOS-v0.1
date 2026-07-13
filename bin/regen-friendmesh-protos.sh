#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
PINNED_GENERATOR="$ROOT_DIR/nanopb-0.4.9/generator-bin/protoc"
PIO_PROTOC="$HOME/.platformio/penv/bin/python-grpc-tools-protoc"
PIO_PLUGIN="$ROOT_DIR/bin/protoc-gen-nanopb-friendmesh"

if [[ -x "$PINNED_GENERATOR" ]]; then
    GENERATOR="$PINNED_GENERATOR"
    GENERATOR_COMMAND=("$GENERATOR")
elif [[ -x "$PIO_PROTOC" && -x "$PIO_PLUGIN" ]]; then
    if [[ ! -f "$ROOT_DIR/.pio/libdeps/t-deck-tft/Nanopb/generator/nanopb_generator.py" ]]; then
        echo "Missing PlatformIO Nanopb generator. Run: pio run -e t-deck-tft" >&2
        exit 1
    fi
    GENERATOR_COMMAND=("$PIO_PROTOC" "--plugin=protoc-gen-nanopb=$PIO_PLUGIN")
else
    echo "Missing Nanopb 0.4.9 generator. Run: pio run -e t-deck-tft" >&2
    exit 1
fi

mkdir -p "$ROOT_DIR/src/friendmesh/generated"
cd "$ROOT_DIR/protobufs/friendmesh"
"${GENERATOR_COMMAND[@]}" --experimental_allow_proto3_optional \
    "--nanopb_out=-S.cpp -v:$ROOT_DIR/src/friendmesh/generated" \
    -I="$ROOT_DIR/protobufs/friendmesh" friendmesh.proto
