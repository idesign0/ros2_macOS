#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INPUT_FILE="$SCRIPT_DIR/matched_packages.txt"

while IFS= read -r line || [[ -n "$line" ]]; do
  # Skip empty or comment lines
  [[ -z "$line" || "$line" =~ ^# ]] && continue

  # First column = formula name
  pkg="${line%% *}"

  echo "🔍 Processing: $pkg"

  # Skip if formula does not exist
  if ! brew info "$pkg" >/dev/null 2>&1; then
    echo "⚠️  Unknown formula, skipping: $pkg"
    continue
  fi

  # Install if missing
  if brew list --formula | grep -qx "$pkg"; then
    echo "✔️  $pkg already installed"
  else
    echo "⬇️  Installing $pkg"
    brew install "$pkg"
  fi

done < "$INPUT_FILE"

echo "✅ Homebrew package processing complete"