#!/bin/bash

# Usage:
# ./remove_submodule.sh path1 path2 "ros-drivers/*"

set -e

if [ "$#" -eq 0 ]; then
    echo "Usage: ./remove_submodule.sh <submodule_path1> [submodule_path2] ..."
    exit 1
fi

# Get all submodule paths from .gitmodules
ALL_SUBMODULES=$(git config -f .gitmodules --get-regexp path | awk '{print $2}')

remove_one() {
    local SUBMODULE_PATH="$1"

    echo "----------------------------------------"
    echo "Removing: $SUBMODULE_PATH"

    git submodule deinit -f "$SUBMODULE_PATH" || true
    git rm -f "$SUBMODULE_PATH" || true
    rm -rf ".git/modules/$SUBMODULE_PATH"

    echo "✅ Removed $SUBMODULE_PATH"
}

for INPUT in "$@"
do
    # If wildcard detected
    if [[ "$INPUT" == *"*"* ]]; then
        echo "Wildcard detected: $INPUT"

        MATCHED=$(echo "$ALL_SUBMODULES" | grep -E "^${INPUT//\*/.*}$" || true)

        if [ -z "$MATCHED" ]; then
            echo "⚠️  No submodules match $INPUT"
            continue
        fi

        for SUBMODULE_PATH in $MATCHED
        do
            remove_one "$SUBMODULE_PATH"
        done
    else
        if echo "$ALL_SUBMODULES" | grep -qx "$INPUT"; then
            remove_one "$INPUT"
        else
            echo "⚠️  '$INPUT' not found in .gitmodules"
        fi
    fi
done

echo "----------------------------------------"
echo "Done."
echo "Commit with:"
echo "git commit -m \"Remove selected submodules\""