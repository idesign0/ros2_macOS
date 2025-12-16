#!/bin/bash
set -u 

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INPUT_FILE="$SCRIPT_DIR/matched_packages.txt"

while IFS= read -r line || [[ -n "$line" ]]; do
  [[ -z "$line" || "$line" =~ ^# ]] && continue

  pkg="${line%% *}"
  echo "🔍 Processing: $pkg"

  # Check formula existence
  if ! brew info "$pkg" >/dev/null 2>&1; then
    echo "⚠️  Unknown formula, skipping: $pkg"
    continue
  fi

  # Correct install check
  if brew list --formula "$pkg" >/dev/null 2>&1; then
    echo "✔️  $pkg already installed"
  else
    echo "⬇️  Installing $pkg"
    brew install "$pkg" || {
      echo "❌ Failed to install $pkg"
      exit 1
    }
  fi

done < "$INPUT_FILE"

echo "✅ Homebrew package processing complete"