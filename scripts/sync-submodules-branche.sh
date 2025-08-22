#!/bin/bash
set -e

# Ensure submodules are initialized and updated
git submodule update --init --recursive

# Loop through each submodule path from .gitmodules
git config -f .gitmodules --get-regexp path | while read -r key path; do
    # Get the branch name for this submodule
    branch=$(git config -f .gitmodules submodule."$path".branch)

    if [ -z "$branch" ]; then
        echo "⚠️  No branch set for submodule '$path' (skipping)"
        continue
    fi

    echo "📂 Submodule: $path → Branch: $branch"

    # Enter the submodule directory
    pushd "$path" > /dev/null

    # Fetch and checkout the correct branch
    git fetch origin "$branch"
    git checkout "$branch"

    # Ensure it's up to date
    git pull origin "$branch"

    popd > /dev/null
done

