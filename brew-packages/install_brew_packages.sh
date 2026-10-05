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

# --- install with retries -------------------------------------------------------------------
# A bottle fetch can fail transiently -- a DNS blip on ghcr.io took libzip out of one humble
# base job ("Failed to download resource libzip" / "Could not resolve host: ghcr.io" x4), and
# because that was swallowed with a warning it silently cost 19 packages downstream:
# gz-fuel_tools' find_package(ZIP) failed -> gz_ros2_control + ros_gz_sim (+ ouster_ros, which
# has its own Findlibzip) -> 16 cascades through ign_ros2_control / ros_ign_gazebo.
# brew's own curl retries fire immediately, so they all land inside the same outage window;
# back off for tens of seconds instead, and say loudly what is still missing at the end.
install_with_retry() {
    local pkg="$1"
    local attempt
    for attempt in 1 2 3; do
        if brew install "$pkg"; then
            return 0
        fi
        echo "   $pkg: install attempt ${attempt}/3 failed"
        if [ "$attempt" -lt 3 ]; then
            sleep $(( attempt * 20 ))
        fi
    done
    return 1
}

FAILED_PKGS=()

# Only enter the loop if there is actually something in the array
if [ ${#packages[@]:-0} -gt 0 ]; then
    for pkg in "${packages[@]}"; do
        echo "🔹 Installing $pkg..."
        install_with_retry "$pkg" || {
            echo "⚠️ Warning: Failed to install $pkg after 3 attempts, continuing..."
            FAILED_PKGS+=("$pkg")
        }
    done
else
    echo "✅ No new packages to install."
fi

# Staying non-fatal is deliberate: some entries are legitimately unavailable on this runner and
# nothing in the workspace needs them. But a missing formula must never again be discoverable
# only as someone else's find_package failure 40 000 log lines later.
if [ ${#FAILED_PKGS[@]:-0} -gt 0 ]; then
    echo "::error::brew: ${#FAILED_PKGS[@]} formula(e) did not install after 3 attempts: ${FAILED_PKGS[*]}"
    echo "Not installed: ${FAILED_PKGS[*]}"
    if [ -n "${GITHUB_STEP_SUMMARY:-}" ]; then
        {
            echo "### Homebrew formulae that failed to install"
            echo
            for pkg in "${FAILED_PKGS[@]}"; do
                echo "- \`${pkg}\`"
            done
            echo
            echo "Any package whose CMake asks for one of these will fail with a"
            echo "\"Could NOT find ...\" that has nothing to do with its own source."
        } >> "$GITHUB_STEP_SUMMARY"
    fi
fi


echo "✅ Done"