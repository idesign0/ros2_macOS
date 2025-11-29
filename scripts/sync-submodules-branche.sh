#!/bin/bash
set -e
set -o pipefail

MISSING_BRANCHES=()

# --- 0. Determine main repo root and set up temporary file ---
MAIN_ROOT=$(git rev-parse --show-toplevel)
TEMP_FILE=$(mktemp)
# Capture submodule list to a temporary file
git config -f "$MAIN_ROOT/.gitmodules" --get-regexp path > "$TEMP_FILE"

# --- 1. Initialization ---
echo "Initializing and updating submodules..."
git submodule update --init --recursive
echo "-------------------------------------------------"

# ----------------------------
# 2. Loop through submodules (macOS-safe)
# ----------------------------
# Input redirection from a file guarantees the while loop runs in the parent shell, 
# ensuring MISSING_BRANCHES array updates are persistent.
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

    # *** FIX 1: Added 'true' to ensure set -e doesn't exit on pushd failure ***
    pushd "$path" > /dev/null || { 
        echo "⚠️  Cannot enter '$path'"; 
        MISSING_BRANCHES+=("$path ($branch)"); 
        continue; 
        true; 
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

    popd > /dev/null

# *** FIX 2: Read from the temporary file ***
done < "$TEMP_FILE"

# Clean up the temporary file
rm -f "$TEMP_FILE"

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