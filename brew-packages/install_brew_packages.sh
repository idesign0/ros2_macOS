#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
INPUT_FILE="$SCRIPT_DIR/matched_packages.txt"

echo "📦 Installing Homebrew formulas..."

brew install --formula $(
  awk '!/^#/ && NF {print $1}' "$INPUT_FILE"
)

echo "✅ Done"
