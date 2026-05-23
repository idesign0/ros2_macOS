#!/usr/bin/env bash
# ------------------------------------------------------------------
# Script: update_forked_submodules.sh
# Purpose: Rebase all forked submodules on their upstream branch
#          and force-push to origin if write access exists.
# Usage: ./update_forked_submodules.sh
# ------------------------------------------------------------------

set -euo pipefail

# Raise per-user process limit — recursive submodule foreach spawns many git
# processes across hundreds of submodules and can hit macOS's default cap (~709)
ulimit -u unlimited 2>/dev/null || ulimit -u 4096 2>/dev/null || true

UPDATED=$(mktemp)
SKIPPED=$(mktemp)
MANUAL=$(mktemp)
trap 'rm -f "$UPDATED" "$SKIPPED" "$MANUAL"' EXIT

echo "🔹 Starting recursive submodule update..."
echo ""

# NOTE: double-quoted so outer shell expands $UPDATED/$SKIPPED/$MANUAL,
# while \$name / \$LOCAL_BRANCH are expanded by the inner subshell.
git submodule foreach --recursive "
    set +e

    if ! git remote get-url upstream >/dev/null 2>&1; then
        echo \"➡️  \$name: no upstream remote — skipped\"
        echo \"\$name||\$LOCAL_BRANCH set to n/a|no upstream remote\" >> \"$SKIPPED\"
        exit 0
    fi

    LOCAL_BRANCH=\$(git symbolic-ref --short HEAD 2>/dev/null || true)
    if [ -z \"\$LOCAL_BRANCH\" ]; then
        echo \"⚠️  \$name: detached HEAD — skipped\"
        echo \"\$name||n/a|detached HEAD\" >> \"$SKIPPED\"
        exit 0
    fi

    if ! git ls-remote --heads upstream \"\$LOCAL_BRANCH\" | grep -q \"\$LOCAL_BRANCH\"; then
        echo \"➡️  \$name [\$LOCAL_BRANCH]: branch not found on upstream — skipped\"
        echo \"\$name||\$LOCAL_BRANCH|branch not found on upstream\" >> \"$SKIPPED\"
        exit 0
    fi

    echo \"🔹 \$name [\$LOCAL_BRANCH]: fetching upstream and origin...\"

    # Fetch upstream
    git fetch upstream \"\$LOCAL_BRANCH\" || {
        echo \"⚠️  \$name [\$LOCAL_BRANCH]: fetch from upstream failed\"
        echo \"\$name||\$LOCAL_BRANCH|upstream fetch failed\" >> \"$MANUAL\"
        exit 0
    }

    if ! git remote get-url origin >/dev/null 2>&1; then
        echo \"➡️  \$name [\$LOCAL_BRANCH]: no origin to push\"
        echo \"\$name||\$LOCAL_BRANCH|no origin\" >> \"$SKIPPED\"
        exit 0
    fi

    # Fetch origin — refreshes lease ref AND lets us detect commits we're missing
    git fetch origin \"\$LOCAL_BRANCH\" 2>/dev/null || true

    # If origin has commits local doesn't have, pull them in first via rebase
    # so they aren't silently dropped when we force-push later
    ORIGIN_AHEAD=\$(git log HEAD..\"origin/\$LOCAL_BRANCH\" --oneline 2>/dev/null | wc -l | tr -d ' ')
    if [ \"\$ORIGIN_AHEAD\" -gt 0 ]; then
        echo \"   origin is \$ORIGIN_AHEAD commit(s) ahead — incorporating before upstream rebase\"
        if ! git rebase \"origin/\$LOCAL_BRANCH\"; then
            echo \"⚠️  \$name [\$LOCAL_BRANCH]: conflict merging origin — needs manual fix\"
            git rebase --abort 2>/dev/null || true
            echo \"\$name||\$LOCAL_BRANCH|conflict pulling origin commits\" >> \"$MANUAL\"
            exit 0
        fi
    fi

    # Try rebase on upstream first (clean linear history).
    # If rebase conflicts, abort and DRY-RUN the merge to check if it's conflict-free
    # before committing anything — only merge if it's guaranteed clean.
    echo \"   rebasing onto upstream/\$LOCAL_BRANCH...\"
    if ! git rebase \"upstream/\$LOCAL_BRANCH\"; then
        git rebase --abort 2>/dev/null || true
        echo \"   rebase had conflicts — checking if merge is conflict-free...\"

        # --no-commit --no-ff: stages the merge but does NOT commit.
        # Exit code 0 = no conflicts. Exit code non-zero = conflicts exist.
        if git merge --no-commit --no-ff \"upstream/\$LOCAL_BRANCH\" >/dev/null 2>&1; then
            # Merge is clean — commit it (equivalent to GitHub 'Update branch')
            git commit --no-edit -m \"Merge upstream/\$LOCAL_BRANCH into \$LOCAL_BRANCH\"
            echo \"   clean merge committed\"
        else
            # Conflicts exist — abort and leave for manual resolution
            git merge --abort 2>/dev/null || true
            echo \"⚠️  \$name [\$LOCAL_BRANCH]: has real conflicts — needs manual fix\"
            echo \"\$name||\$LOCAL_BRANCH|real conflict (needs manual resolution)\" >> \"$MANUAL\"
            exit 0
        fi
    fi

    if git push --force-with-lease origin \"\$LOCAL_BRANCH\"; then
        echo \"✅ \$name [\$LOCAL_BRANCH]: pushed\"
        echo \"\$name||\$LOCAL_BRANCH|\" >> \"$UPDATED\"
    else
        echo \"⚠️  \$name [\$LOCAL_BRANCH]: push failed — needs manual fix\"
        echo \"\$name||\$LOCAL_BRANCH|push failed\" >> \"$MANUAL\"
    fi
"

# ── Summary ────────────────────────────────────────────────────────
echo ""
echo "════════════════════════════════════════════════════════"
echo "📋  SUMMARY"
echo "════════════════════════════════════════════════════════"

count_lines() { grep -c '' "$1" 2>/dev/null || echo 0; }

UPDATED_N=$(count_lines "$UPDATED")
SKIPPED_N=$(count_lines "$SKIPPED")
MANUAL_N=$(count_lines "$MANUAL")

print_table() {
    local file="$1"
    while IFS='|' read -r sub _ branch reason; do
        printf "      %-48s %-15s %s\n" "${sub# }" "[${branch# }]" "${reason# }"
    done < "$file"
}

if [ "$UPDATED_N" -gt 0 ]; then
    echo ""
    echo "✅  Updated & pushed ($UPDATED_N):"
    print_table "$UPDATED"
fi

if [ "$SKIPPED_N" -gt 0 ]; then
    echo ""
    echo "➡️   Skipped ($SKIPPED_N):"
    print_table "$SKIPPED"
fi

if [ "$MANUAL_N" -gt 0 ]; then
    echo ""
    echo "⚠️   NEEDS MANUAL ATTENTION ($MANUAL_N):"
    print_table "$MANUAL"
    echo ""
    echo "  For each, run:"
    echo "    cd <workspace>/<submodule-path>"
    echo "    git fetch upstream <branch>"
    echo "    git rebase upstream/<branch>"
    echo "    # resolve conflicts, then:"
    echo "    git rebase --continue"
    echo "    git fetch origin <branch>"
    echo "    git push --force-with-lease origin <branch>"
fi

echo ""
echo "════════════════════════════════════════════════════════"
if [ "$MANUAL_N" -gt 0 ]; then
    echo "⚠️   Done — $MANUAL_N submodule(s) need manual attention (see above)."
else
    echo "✅  Done — all submodules processed."
fi
