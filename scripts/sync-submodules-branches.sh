#!/bin/bash
set -e
set -o pipefail

MISSING_BRANCHES=()

# --- 0. Determine main repo root ---
MAIN_ROOT=$(git rev-parse --show-toplevel)

# --- 1. Initialization ---
echo "Initializing and updating submodules..."
git submodule update --init --recursive
echo "-------------------------------------------------"

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

    origin_owner=$(echo "$ORIGIN_URL" | grep -oE 'github.com/([^/]+)/' | cut -d'/' -f2)
    upstream_owner=$(echo "$UPSTREAM_URL" | grep -oE 'github.com/([^/]+)/' | cut -d'/' -f2)

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

    # --- Nested submodules ---
    # Step 1's `submodule update --init --recursive` ran against the gitlink the
    # superproject records, so it only materialised the nested submodules that existed
    # at THAT commit. The checkout above may have moved this submodule to a branch tip
    # whose .gitmodules adds, removes or repoints nested submodules, and git does not
    # touch a submodule's own submodules on checkout -- so those land empty or stale.
    #
    # Measured, not theorised: kroshu restructured kuka_drivers into a thin wrapper
    # whose sources moved into the nested `upstream/kuka_drivers`, while the
    # superproject gitlink still pinned the pre-restructure flat commit. After the
    # checkout above, upstream/kuka_drivers was an empty directory (0 files) and every
    # kuka package lost its sources entirely.
    #
    # Run from INSIDE the submodule. A `git submodule update` at $MAIN_ROOT would reset
    # every submodule back to the superproject's recorded gitlink and undo this script's
    # entire purpose.
    if [ -f .gitmodules ]; then
        echo "   └─ refreshing nested submodules for the checked-out tip"
        git submodule sync --recursive >/dev/null 2>&1 || true
        git submodule update --init --recursive \
            || echo "⚠️ Could not update nested submodules in '$path'"
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
