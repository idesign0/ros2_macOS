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

# A few entries are legitimately not installable here and nothing in the workspace needs them;
# those are listed in allow_install_failures.txt. Anything else is fatal. Failing now costs one
# job; letting it through costs a whole run of phantom failures somewhere else entirely, because
# a missing formula only resurfaces as somebody else's find_package error in a later job.
ALLOW_FILE="$SCRIPT_DIR/allow_install_failures.txt"
is_allowed_failure() {
    local pkg="$1" line
    [ -f "$ALLOW_FILE" ] || return 1
    while IFS= read -r line; do
        line="${line%%#*}"
        line="$(echo "$line" | awk '{print $1}')"
        [ -z "$line" ] && continue
        [ "$line" = "$pkg" ] && return 0
    done < "$ALLOW_FILE"
    return 1
}

if [ ${#FAILED_PKGS[@]:-0} -gt 0 ]; then
    UNEXPECTED=()
    for pkg in "${FAILED_PKGS[@]}"; do
        if is_allowed_failure "$pkg"; then
            echo "   (tolerated) $pkg is listed in allow_install_failures.txt"
        else
            UNEXPECTED+=("$pkg")
        fi
    done

    if [ -n "${GITHUB_STEP_SUMMARY:-}" ]; then
        {
            echo "### Homebrew formulae that failed to install"
            echo
            for pkg in "${FAILED_PKGS[@]}"; do
                if is_allowed_failure "$pkg"; then
                    echo "- \`${pkg}\` (tolerated)"
                else
                    echo "- \`${pkg}\` **UNEXPECTED**"
                fi
            done
        } >> "$GITHUB_STEP_SUMMARY"
    fi

    if [ ${#UNEXPECTED[@]:-0} -gt 0 ]; then
        echo "::error::brew: ${#UNEXPECTED[@]} formula(e) failed to install and are not in allow_install_failures.txt: ${UNEXPECTED[*]}"
        echo "Refusing to build against an incomplete Homebrew prefix. Re-run this job;"
        echo "if the formula is genuinely unavailable, add it to allow_install_failures.txt."
        exit 1
    fi

    echo "All install failures are in allow_install_failures.txt; continuing."
fi


# ---------------------------------------------------------------------------------------
# Record the Homebrew versions this job ACTUALLY installed.
#
# matched_packages.txt supplies the formula NAMES; `brew install "$pkg"` above passes no
# version, so what lands is whatever homebrew-core currently says -- the versions written
# beside the names in that file are a record, not a constraint. A record can be stale: it
# said abseil 20260107.1 (soname 2601) while jobs were installing 20260817.0 (2608).
#
# The gz-install cache key hashes this file, so the key reflects what the job really has
# rather than what some earlier commit claimed. Without it, a cached gz tree built against
# an older brew is restored into a job that installed a newer one, and nothing notices.
# That is how gz and protobuf ended up linked to abseil 2601 after abseil DELETED
# libabsl_crc_cpu_detect and libabsl_borrowed_fixup_buffer: 236 dylibs and 5 executables in
# the shipped ros2-jazzy keg cannot be dlopen'd at all.
#
# Narrowed to the formulae this workspace names. Hashing all of `brew list --versions`
# would also cover the runner image's preinstalls, whose churn would rebuild gz on every
# image bump for no reason.
ROOT="${GITHUB_WORKSPACE:-$(cd "$SCRIPT_DIR/.." && pwd)}"
awk '{print $1}' "$INPUT_FILE" | sort -u > /tmp/brew_want.txt
brew list --formula --versions \
  | awk 'NR==FNR{w[$1];next} ($1 in w)' /tmp/brew_want.txt - \
  | sort > "$ROOT/brew-state.txt"
BREW_STATE_N=$(wc -l < "$ROOT/brew-state.txt" | tr -d ' ')
echo "brew-state.txt: $BREW_STATE_N formulae (feeds the gz cache key)"
# An empty or near-empty file hashes to a constant, which would silently validate ANY
# cached gz tree -- the same shape of trap as packing a partial install tree.
if [ "${BREW_STATE_N:-0}" -lt 50 ]; then
    echo "::error::brew-state.txt has only ${BREW_STATE_N:-0} entries; refusing to let the gz cache key be keyed on it"
    exit 1
fi
grep -E '^(abseil|protobuf|qt@5|qtbase|boost|openssl@3|opencv@4|dartsim) ' "$ROOT/brew-state.txt" || true

echo "✅ Done"
