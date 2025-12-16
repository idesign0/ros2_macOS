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
echo "⏳ Fetching tap information for all packages (may take a moment)..."
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

  # --- OPTIMIZED TAP DETECTION ---
  # Search the preloaded info for the current package's details
  # Adding || true to prevent the script from exiting if grep/tail fails unexpectedly
  package_info_snippet=$(echo "$FORMULA_INFO_ALL" | grep -E -A 1 "^$pkg: " | tail -n 1 || true) 

  # Check if the snippet contains the 'From:' pattern
  if [[ "$package_info_snippet" =~ "From: ([^[:space:]]+)" ]]; then
    tap="${BASH_REMATCH[1]%/Formula/*}"
    
    # Check if tap is already present. Adding || true here just in case.
    if ! brew tap | grep -qx "$tap" || true; then 
      echo "🔗 Adding tap: $tap for $pkg"
      
      if brew tap "$tap"; then
        echo "   -> Tap added successfully."
      else
        echo "❌ Warning: Failed to add tap $tap. Installation of $pkg may fail. Continuing..."
      fi
    fi
  fi

  packages+=("$pkg")
done

# Install all remaining packages at once
if [ "${#packages[@]}" -gt 0 ]; then
  echo "⬇️  Installing ${#packages[@]} formulas: ${packages[*]}"
  brew install --formula "${packages[@]}"
else
  # --- ADDED BLOCK FOR CLARITY ---
  echo "---"
  echo "🎉 SUCCESS! All ${PACKAGE_COUNT} formulas from the list were already installed."
  echo "---"
fi

echo "✅ Done"