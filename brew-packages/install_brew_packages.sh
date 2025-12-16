#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
INPUT_FILE="$SCRIPT_DIR/matched_packages.txt"

echo "📦 Installing Homebrew formulas..."

packages=()
while IFS= read -r pkg; do
  [[ -z "$pkg" || "$pkg" =~ ^# ]] && continue

  # Check if formula is already installed
  if brew list --formula "$pkg" >/dev/null 2>&1; then
    echo "✔️  $pkg already installed"
    continue
  fi

  # Detect which tap (if any) provides this formula
  formula_info=$(brew info "$pkg" 2>/dev/null || true)
  if [[ "$formula_info" =~ "From: ([^[:space:]]+)" ]]; then
    tap="${BASH_REMATCH[1]%/Formula/*}"
    # Add tap if not already tapped
    if ! brew tap | grep -qx "$tap"; then
      echo "🔗 Adding tap: $tap"
      brew tap "$tap"
    fi
  fi

  packages+=("$pkg")
done < "$INPUT_FILE"

# Install all remaining packages at once
if [ "${#packages[@]}" -gt 0 ]; then
  echo "⬇️  Installing ${#packages[@]} formulas..."
  brew install --formula "${packages[@]}"
fi

echo "✅ Done"