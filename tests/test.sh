#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
CMD_FILE="$ROOT_DIR/tests/commands.txt"
OUT_DIR="$ROOT_DIR/tests/out"
REF_OUT="$OUT_DIR/bash.out"
GSHELL_OUT="$OUT_DIR/gshell.out"

mkdir -p "$OUT_DIR"

echo "Building gshell..."
make -C "$ROOT_DIR"

echo "Running reference (bash) ..."
bash "$CMD_FILE" > "$REF_OUT" 2>&1 || true

echo "Running gshell..."
"$ROOT_DIR/gshell" < "$CMD_FILE" > "$GSHELL_OUT.raw" 2>&1 || true

echo "Filtering gshell prompts..."
# Remove lines that start with '@' (gshell prompt) and lines that start with '$ ' (command echo)
# Also normalize PIDs (replace any number-only line with <PID>)
sed '/^@/d; /^\$ /d' "$GSHELL_OUT.raw" | sed 's/^[0-9]\+$/<PID>/' > "$GSHELL_OUT"

echo "Filtering bash output..."
# Normalize PIDs in bash output too
sed 's/^[0-9]\+$/<PID>/' "$REF_OUT" > "$REF_OUT.normalized"

echo "Diffing outputs..."
if diff -u "$REF_OUT.normalized" "$GSHELL_OUT"; then
  echo "OK: outputs match"
  exit 0
else
  echo "FAIL: outputs differ. See $OUT_DIR for artifacts"
  exit 2
fi
