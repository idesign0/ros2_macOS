#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
INPUT_FILE="$SCRIPT_DIR/matched_packages.txt"

echo "🔄 Preloading Homebrew status for faster checks..."

# 1. Fetch the list of ALL currently installed formulas ONCE
INSTALLED_FORMULAS=$(brew list --formula 2>/dev/null || echo "")

# --- 2. Clean up package names and provide progress feedback ---
PACKAGE_NAMES_ARRAY=()
while IFS= read -r line; do
    pkg=$(echo "$line" | awk '{print $1}')
    [[ -z "$pkg" || "$pkg" =~ ^# ]] && continue
    PACKAGE_NAMES_ARRAY+=("$pkg")
done < "$INPUT_FILE"

PACKAGE_COUNT=${#PACKAGE_NAMES_ARRAY[@]}

echo "---"
echo "🔍 Found ${PACKAGE_COUNT} unique formulas to process."
echo "---"

PACKAGE_NAMES_ONLY="${PACKAGE_NAMES_ARRAY[*]}"

# 3. Fetch the tap info for ALL packages ONCE
echo "⏳ Fetching information for all packages (may take a moment)..."
FORMULA_INFO_ALL=$(brew info $PACKAGE_NAMES_ONLY 2>/dev/null || true)

echo "✅ Homebrew status preloaded."
echo "--------------------------------------------------------"

echo "📦 Starting formula check and installation queue..."

packages=()
for pkg in "${PACKAGE_NAMES_ARRAY[@]}"; do
  
  # --- OPTIMIZED CHECK: Check if formula is already installed ---
  # CRITICAL: This grep must not fail the script if the package isn't found in the list.
  if echo "$INSTALLED_FORMULAS" | grep -w -q "$pkg"; then
    echo "✔️  $pkg already installed"
    continue
  fi
  packages+=("$pkg")
done

# Only enter the loop if there is actually something in the array
if [ ${#packages[@]:-0} -gt 0 ]; then
    for pkg in "${packages[@]}"; do
        echo "🔹 Installing $pkg..."
        brew install "$pkg" || {
            echo "⚠️ Warning: Failed to install $pkg, continuing..."
        }
    done
else
    echo "✅ No new packages to install."
fi

echo "✅ Done"