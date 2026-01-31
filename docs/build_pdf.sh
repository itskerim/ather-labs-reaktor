#!/usr/bin/env bash
# Build AETHER 3.0 User Manual PDF from Markdown (AETHER style guide).
# Requires: Node.js and npx (md-to-pdf will be downloaded on first run).
# Output: AETHER_3.0_User_Manual.pdf (NOT .md) in this folder.

set -e
cd "$(dirname "$0")"
INPUT="AETHER_3.0_User_Manual.md"
OUTPUT="AETHER_3.0_User_Manual.pdf"
STYLESHEET="aether-manual.css"

echo "Building PDF (AETHER style, full-bleed black) ..."
echo "  Input:  $INPUT"
echo "  Output: $OUTPUT"
npx --yes md-to-pdf "$INPUT" --stylesheet "$STYLESHEET" --pdf-options '{"margin":{"top":"0in","right":"0in","bottom":"0in","left":"0in"},"printBackground":true}'

if [ -f "$OUTPUT" ]; then
  echo "Done. PDF written to: $(pwd)/$OUTPUT"
else
  echo "ERROR: Expected $OUTPUT was not created. Check that md-to-pdf ran successfully." >&2
  exit 1
fi
