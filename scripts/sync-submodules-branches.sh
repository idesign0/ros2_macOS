#!/bin/bash
# Intentionally NOT `set -e` / `set -o pipefail`: this script walks EVERY
# submodule and must tolerate individual failures (non-GitHub URLs where the
# owner `grep` finds no match, missing branches, un-clonable repos) without
# aborting the whole run. Failures are collected in MISSING_BRANCHES and
# reported in the summary instead.
set +e
set +o pipefail

MISSING_BRANCHES=()

# --- 0. Determine main repo root ---
MAIN_ROOT=$(git rev-parse --show-toplevel)

# --- 1. Initialization ---
echo "Initializing and updating submodules..."
git submodule update --init --recursive
echo "-------------------------------------------------"

# Re-apply COLCON_IGNORE for vendored/nested duplicate packages living INSIDE
# submodules (recorded by `ros2pkg dedupe-packages`). These can't be tracked by
# the superproject, so they must be recreated after each fresh submodule checkout
# or colcon aborts with "Duplicate package names not supported".
IGNORE_LIST="$MAIN_ROOT/scripts/ros2pkg/colcon_ignore.list"
if [ -f "$IGNORE_LIST" ]; then
    while read -r ip; do
        [ -z "$ip" ] && continue
        if [ -d "$MAIN_ROOT/$ip" ]; then
            touch "$MAIN_ROOT/$ip/COLCON_IGNORE"
            echo "🚫 COLCON_IGNORE -> $ip"
        fi
    done < "$IGNORE_LIST"
    echo "-------------------------------------------------"
fi

# ----------------------------
# 2. Loop through submodules (macOS-safe)
# ----------------------------
while read -r key path; do
    branch=$(git config -f "$MAIN_ROOT/.gitmodules" submodule."$path".branch)

    echo
    echo "📂 Processing submodule: $path"
    echo "   └─ Required branch: ${branch:-<none>}"

    # --- Directory Check ---
    if [ ! -d "$path" ]; then
        echo "⚠️  Submodule folder '$path' does not exist, skipping"
        MISSING_BRANCHES+=("$path ($branch)")
        continue
    fi

    pushd "$path" > /dev/null || { 
        echo "⚠️  Cannot enter '$path'"; 
        MISSING_BRANCHES+=("$path ($branch)"); 
        continue; 
    }

    # --- Branch Check ---
    if [ -z "$branch" ]; then
        echo "⚠️  No branch defined for '$path', skipping"
        popd > /dev/null
        MISSING_BRANCHES+=("$path (<none>)")
        continue
    fi

    # --- Fetch origin ---
    git fetch origin || echo "⚠️ Could not fetch origin"

    # --- Upstream Detection ---
    UPSTREAM_URL=$(git config -f "$MAIN_ROOT/.gitmodules" --get submodule."$path".url)
    ORIGIN_URL=$(git remote get-url origin)

    origin_owner=$(echo "$ORIGIN_URL" | grep -oE 'github.com/([^/]+)/' | cut -d'/' -f2 || true)
    upstream_owner=$(echo "$UPSTREAM_URL" | grep -oE 'github.com/([^/]+)/' | cut -d'/' -f2 || true)

    if [ -n "$UPSTREAM_URL" ] && [ "$origin_owner" != "$upstream_owner" ]; then
        if ! git remote | grep -q upstream; then
            echo "🔧 Adding upstream remote: $UPSTREAM_URL"
            git remote add upstream "$UPSTREAM_URL" || true
        fi
        git fetch upstream || echo "⚠️ Could not fetch upstream"
    else
        echo "ℹ️ Repository is not a fork → skipping upstream remote"
    fi

    # --- Branch Exists Check and Checkout ---
    branch_exists() { git ls-remote --exit-code "$1" "$branch" &>/dev/null; }

    if branch_exists origin; then
        echo "✔️ Branch '$branch' found on origin → checking out"
        git checkout "$branch" || true
        git pull origin "$branch" || true
    elif branch_exists upstream; then
        echo "✔️ Branch '$branch' found on upstream → creating and checking out"
        git checkout -b "$branch" "upstream/$branch" || true
    else
        echo "❌ Branch '$branch' not found on origin or upstream"
        MISSING_BRANCHES+=("$path ($branch)")
    fi

    popd > /dev/null

# Safe process substitution to loop over submodules
done < <(git config -f "$MAIN_ROOT/.gitmodules" --get-regexp path)

# ----------------------------
# 3. Summary
# ----------------------------
echo
echo "==================== SUMMARY ===================="

if [ ${#MISSING_BRANCHES[@]} -eq 0 ]; then
    echo "🎉 All submodules have the required branches."
else
    echo "⚠️ Missing branches or unreachable submodules:"
    for missing in "${MISSING_BRANCHES[@]}"; do
        echo "   - $missing"
    done
fi

echo "================================================="
