#!/usr/bin/env bash

set -euo pipefail

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

SEED="4ccd089b28ff96da9db6c346ec114e0f5b8a319f35aba624da8cf6ed4fb8a6fb"
PUBLIC="3d4017c3e843895a92b70aa74d1b7ebc9c982ccf2ec4968cc0cd55f12af4660c"
MESSAGE="72"
SIGNATURE="92a009a9f0d4cab8720e820b5f642540a2b27b5416503f8fb3762223ebdb69da085ac1e43e15996e458f3613d0f11d8c387b2eaeb4302aeeb00d291612bb0c00"
MALLEATED="92a009a9f0d4cab8720e820b5f642540a2b27b5416503f8fb3762223ebdb69daf52db7415978abc61b2c2eb6aeebfca0387b2eaeb4302aeeb00d291612bb0c10"

printf '%s' "302e020100300506032b657004220420${SEED}" | xxd -r -p >"$TMP_DIR/private.der"
printf '%s' "302a300506032b6570032100${PUBLIC}" | xxd -r -p >"$TMP_DIR/public.der"
printf '%s' "$SIGNATURE" | xxd -r -p >"$TMP_DIR/expected.sig"
printf '%s' "$MALLEATED" | xxd -r -p >"$TMP_DIR/malleated.sig"
printf '%s' "$MESSAGE" | xxd -r -p >"$TMP_DIR/message"

openssl pkeyutl -sign -inkey "$TMP_DIR/private.der" -keyform DER -rawin -in "$TMP_DIR/message" \
    -out "$TMP_DIR/actual.sig"
cmp "$TMP_DIR/expected.sig" "$TMP_DIR/actual.sig"
openssl pkeyutl -verify -pubin -inkey "$TMP_DIR/public.der" -keyform DER -rawin -in "$TMP_DIR/message" \
    -sigfile "$TMP_DIR/expected.sig" >/dev/null

cp "$TMP_DIR/expected.sig" "$TMP_DIR/invalid.sig"
printf '\x00' | dd of="$TMP_DIR/invalid.sig" bs=1 seek=0 conv=notrunc status=none
if openssl pkeyutl -verify -pubin -inkey "$TMP_DIR/public.der" -keyform DER -rawin -in "$TMP_DIR/message" \
    -sigfile "$TMP_DIR/invalid.sig" >/dev/null 2>&1; then
    echo "Mutated Ed25519 signature was accepted" >&2
    exit 1
fi
if openssl pkeyutl -verify -pubin -inkey "$TMP_DIR/public.der" -keyform DER -rawin -in "$TMP_DIR/message" \
    -sigfile "$TMP_DIR/malleated.sig" >/dev/null 2>&1; then
    echo "Noncanonical Ed25519 S + L signature was accepted" >&2
    exit 1
fi

echo "RFC 8032 Ed25519 vector passed"
