#!/usr/bin/env bash
# Build AETHER 3.0 User Manual PDF from Markdown.
# Requires: Node.js and npx (md-to-pdf will be downloaded on first run).

set -e
cd "$(dirname "$0")"
echo "Building AETHER_3.0_User_Manual.pdf ..."
npx --yes md-to-pdf AETHER_3.0_User_Manual.md
echo "Done. Output: $(pwd)/AETHER_3.0_User_Manual.pdf"
