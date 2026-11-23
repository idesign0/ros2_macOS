#!/usr/bin/env bash
# ------------------------------------------------------------------
# add_upstream_remotes.sh
#
# Gives each forked submodule the `upstream` remote that
# update_forked_submodules.sh requires, reading it from
# ci/fork-upstreams.tsv.
#
# Needed because that remote normally lives in .git/modules/<path>/config,
# which is never committed. On a developer machine it is there because someone
# added it by hand; in CI every submodule is a fresh clone with only `origin`,
# so the updater skipped all 648 of them with "no upstream remote" and had never
# updated anything. This script is the missing piece.
#
# Anything listed in ci/fork-no-autoupdate.txt is deliberately LEFT WITHOUT an
# upstream remote, which is what makes it invisible to the updater. Those are
# pins; rebasing one would undo it.
#
# Idempotent: an existing upstream remote is corrected if it points elsewhere,
# left alone if it already matches.
# ------------------------------------------------------------------

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MAP="$ROOT/ci/fork-upstreams.tsv"
SKIP="$ROOT/ci/fork-no-autoupdate.txt"

if [ ! -f "$MAP" ]; then
    echo "::error::$MAP not found"
    exit 1
fi

# Read the skip list into a newline-delimited blob we can grep exactly.
SKIP_LIST=""
if [ -f "$SKIP" ]; then
    SKIP_LIST=$(grep -v '^[[:space:]]*#' "$SKIP" | grep -v '^[[:space:]]*$' || true)
fi

added=0; fixed=0; already=0; skipped=0; absent=0

while IFS=$'\t' read -r path upstream; do
    case "$path" in ''|'#'*) continue ;; esac
    [ -n "${upstream:-}" ] || continue

    if [ -n "$SKIP_LIST" ] && printf '%s\n' "$SKIP_LIST" | grep -qxF "$path"; then
        # Positively REMOVE any upstream remote on a pinned submodule rather than
        # merely declining to add one. On a developer machine the remote is often
        # already there by hand, and leaving it would let the updater see the pin
        # and rebase it -- undoing the pin. Absence of the remote is the mechanism
        # that protects it, so make the absence true.
        if [ -d "$ROOT/$path/.git" ] || [ -f "$ROOT/$path/.git" ]; then
            if git -C "$ROOT/$path" remote get-url upstream >/dev/null 2>&1; then
                git -C "$ROOT/$path" remote remove upstream
                echo "  - upstream REMOVED (pinned): $path"
            else
                echo "  pinned, no upstream remote: $path"
            fi
        else
            echo "  pinned, not checked out: $path"
        fi
        skipped=$((skipped+1))
        continue
    fi

    if [ ! -d "$ROOT/$path/.git" ] && [ ! -f "$ROOT/$path/.git" ]; then
        echo "  not checked out, skipping: $path"
        absent=$((absent+1))
        continue
    fi

    current=$(git -C "$ROOT/$path" remote get-url upstream 2>/dev/null || true)
    if [ -z "$current" ]; then
        git -C "$ROOT/$path" remote add upstream "$upstream"
        echo "  + upstream $upstream  ($path)"
        added=$((added+1))
    elif [ "$current" != "$upstream" ]; then
        git -C "$ROOT/$path" remote set-url upstream "$upstream"
        echo "  ~ upstream corrected to $upstream  ($path)"
        fixed=$((fixed+1))
    else
        already=$((already+1))
    fi
done < "$MAP"

echo ""
echo "upstream remotes: $added added, $fixed corrected, $already already correct,"
echo "                  $skipped skipped as pinned, $absent not checked out"
