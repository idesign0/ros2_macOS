#!/usr/bin/env bash
# ------------------------------------------------------------------
# Script: update_forked_submodules.sh
# Purpose: Rebase all forked submodules on their upstream branch
#          and force-push to origin if write access exists.
# Usage: ./update_forked_submodules.sh
# ------------------------------------------------------------------

set -euo pipefail

echo "🔹 Starting recursive submodule update..."

git submodule foreach --recursive '
    # Only proceed if upstream remote exists
    if git remote get-url upstream >/dev/null 2>&1; then
        LOCAL_BRANCH=$(git symbolic-ref --short HEAD)
        echo "🔹 Submodule $name: on local branch $LOCAL_BRANCH"

        # Check if the upstream has the same branch
        if git ls-remote --heads upstream "$LOCAL_BRANCH" | grep -q "$LOCAL_BRANCH"; then
            echo "✅ Pulling and rebasing $LOCAL_BRANCH from upstream..."
            
            # Pull with rebase
            git pull --rebase upstream "$LOCAL_BRANCH" || {
                echo "⚠️ Conflict or error in $name on branch $LOCAL_BRANCH, aborting rebase."
                git rebase --abort 2>/dev/null || true
                continue
            }

            # Check if origin exists
            if git remote get-url origin >/dev/null 2>&1; then
                ORIGIN_URL=$(git remote get-url origin)

                # Test write access using a dry-run push
                if git push --dry-run "$ORIGIN_URL" "$LOCAL_BRANCH" >/dev/null 2>&1; then
                    echo "🚀 Pushing rebased branch $LOCAL_BRANCH to origin with --force-with-lease..."
                    git push --force-with-lease origin "$LOCAL_BRANCH" || {
                        echo "⚠️ Push failed for $name/$LOCAL_BRANCH"
                    }
                else
                    echo "⚠️ Skipping push for $name/$LOCAL_BRANCH: no write access to origin"
                fi
            else
                echo "➡️ Skipping push: origin remote not found."
            fi
        else
            echo "➡️ Skipping $name: upstream branch $LOCAL_BRANCH does not exist."
        fi
    else
        echo "➡️ Skipping $name: upstream remote not found."
    fi
'

echo "✅ Submodule update script finished."
