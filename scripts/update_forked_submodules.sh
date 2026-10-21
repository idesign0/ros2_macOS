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
# Pushes that succeeded but sent nothing, kept apart from UPDATED. Conflating the two
# made a run that changed almost nothing look like a run that refreshed 185 forks: on
# 2026-10-20, run 39429097901 reported 185 "pushed" of which 179 were "Everything
# up-to-date" and only 6 carried commits.
NOOP=$(mktemp)
trap 'rm -f "$UPDATED" "$SKIPPED" "$MANUAL" "$NOOP"' EXIT

# Forks that must never be rebased (pins). Belt-and-braces: add_upstream_remotes.sh
# already withholds/removes their `upstream` remote, which alone keeps them out of this
# loop, but a hand-added remote on a developer machine would defeat that. Enforce the
# invariant here too, where it actually matters.
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CI_FORK_SKIP=""
if [ -f "$REPO_ROOT/ci/fork-no-autoupdate.txt" ]; then
    CI_FORK_SKIP=$(grep -v '^[[:space:]]*#' "$REPO_ROOT/ci/fork-no-autoupdate.txt" \
                   | grep -v '^[[:space:]]*$' || true)
fi
export CI_FORK_SKIP

# DRY_RUN=1 does everything except the force-push, so a run can be inspected first.
# Default 0 keeps the existing local behaviour unchanged.
DRY_RUN="${DRY_RUN:-0}"
export DRY_RUN
if [ "$DRY_RUN" = "1" ]; then
    echo "DRY RUN: rebases will be computed, nothing will be pushed"
fi

echo "🔹 Starting recursive submodule update..."
echo ""

# NOTE: double-quoted so outer shell expands $UPDATED/$SKIPPED/$MANUAL,
# while \$name / \$LOCAL_BRANCH are expanded by the inner subshell.
git submodule foreach --recursive "
    set +e

    if [ -n \"\$CI_FORK_SKIP\" ] && printf '%s\\n' \"\$CI_FORK_SKIP\" | grep -qxF \"\$name\"; then
        echo \"PINNED \$name: in ci/fork-no-autoupdate.txt — refusing to rebase\"
        echo \"\$name||n/a|pinned (ci/fork-no-autoupdate.txt)\" >> \"$SKIPPED\"
        exit 0
    fi

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

    if [ \"\$DRY_RUN\" = \"1\" ]; then
        echo \"   DRY RUN: would push \$name [\$LOCAL_BRANCH]\"
        echo \"\$name||\$LOCAL_BRANCH|dry run, not pushed\" >> \"$SKIPPED\"
        exit 0
    fi

    # Captured rather than streamed, because the outcome has to be classified:
    # \"Everything up-to-date\" is the difference between a fork that was refreshed and
    # one that was already current. The output is echoed back verbatim, so the log reads
    # exactly as before.
    PUSH_OUT=\$(git push --force-with-lease origin \"\$LOCAL_BRANCH\" 2>&1)
    PUSH_RC=\$?
    printf '%s\\n' \"\$PUSH_OUT\"
    if [ \"\$PUSH_RC\" = 0 ]; then
        case \"\$PUSH_OUT\" in
          *'Everything up-to-date'*)
            echo \"➡️  \$name [\$LOCAL_BRANCH]: already current, nothing sent\"
            echo \"\$name||\$LOCAL_BRANCH|already up to date\" >> \"$NOOP\"
            ;;
          *)
            echo \"✅ \$name [\$LOCAL_BRANCH]: pushed\"
            echo \"\$name||\$LOCAL_BRANCH|\" >> \"$UPDATED\"
            ;;
        esac
    else
        # Name the cause. The 47 rejected pushes of run 39429097901 split 34/13 between
        # the first two buckets, and each needs a different remedy -- a bare
        # \"push failed\" sent us hunting a redirect bug when 13 were a token-scope problem.
        why='push failed'
        case \"\$PUSH_OUT\" in
          *'could not read Username'*) why='push failed: credential lost after redirect';;
          *workflow*scope*)            why='push failed: token lacks workflow scope';;
          *'stale info'*|*'fetch first'*|*'non-fast-forward'*) why='push failed: stale lease, rerun';;
          *'remote rejected'*)         why='push failed: remote rejected';;
          *'Permission'*denied*)       why='push failed: no write permission';;
        esac
        echo \"⚠️  \$name [\$LOCAL_BRANCH]: \$why — needs manual fix\"
        echo \"\$name||\$LOCAL_BRANCH|\$why\" >> \"$MANUAL\"
    fi
"

# ── Summary ────────────────────────────────────────────────────────
echo ""
echo "════════════════════════════════════════════════════════"
echo "📋  SUMMARY"
echo "════════════════════════════════════════════════════════"

# grep -c '' prints 0 AND exits 1 on an empty file, so the old
# `grep -c '' "$1" || echo 0` emitted "0\n0" and every later [ "$N" -gt 0 ] test
# died with "integer expression expected". Count only when there is something to count.
count_lines() { if [ -s "$1" ]; then grep -c '' "$1"; else echo 0; fi; }

UPDATED_N=$(count_lines "$UPDATED")
SKIPPED_N=$(count_lines "$SKIPPED")
MANUAL_N=$(count_lines "$MANUAL")
NOOP_N=$(count_lines "$NOOP")

print_table() {
    local file="$1"
    while IFS='|' read -r sub _ branch reason; do
        printf "      %-48s %-15s %s\n" "${sub# }" "[${branch# }]" "${reason# }"
    done < "$file"
}

# MANUAL FIRST, deliberately. There are ~636 submodules and only ~93 forks, so the
# overwhelming majority land in SKIPPED as "no upstream remote" -- by design, they are
# not forks. Printing SKIPPED first buried the one bucket that needs a human under ~540
# rows of expected noise, and the workflow's `head -300` then cut it off entirely, so the
# conflict list never reached the job summary at all. Order is now: what needs a human,
# what changed, then the noise.
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

if [ "$UPDATED_N" -gt 0 ]; then
    echo ""
    echo "✅  Updated & pushed -- carried commits ($UPDATED_N):"
    print_table "$UPDATED"
fi

# A bare count on purpose. A fork that already matches upstream is the healthy steady
# state; listing every one of them in full is what pushed the real signal off the
# bottom of the job summary.
if [ "$NOOP_N" -gt 0 ]; then
    echo ""
    echo "➡️   Already current, nothing sent ($NOOP_N)."
fi

# Split SKIPPED by reason. "would push" is the interesting half of a dry run and must be
# listed in full; "no upstream remote" is ~540 rows of not-a-fork and is reported as a
# count with a handful of examples, so it can never crowd out anything above it.
if [ "$SKIPPED_N" -gt 0 ]; then
    WOULD=$(mktemp); PINNED=$(mktemp); NOUP=$(mktemp)
    grep -F 'dry run, not pushed'  "$SKIPPED" > "$WOULD"  2>/dev/null || true
    grep -F 'pinned ('             "$SKIPPED" > "$PINNED" 2>/dev/null || true
    grep -F 'no upstream remote'   "$SKIPPED" > "$NOUP"   2>/dev/null || true
    WOULD_N=$(count_lines "$WOULD"); PINNED_N=$(count_lines "$PINNED"); NOUP_N=$(count_lines "$NOUP")

    if [ "$WOULD_N" -gt 0 ]; then
        echo ""
        echo "🔎  DRY RUN -- would rebase and push ($WOULD_N):"
        print_table "$WOULD"
    fi

    if [ "$PINNED_N" -gt 0 ]; then
        echo ""
        echo "📌  Pinned, refused on purpose ($PINNED_N):"
        print_table "$PINNED"
    fi

    if [ "$NOUP_N" -gt 0 ]; then
        echo ""
        echo "➡️   No upstream remote ($NOUP_N) -- not forks, nothing to do. First 5:"
        head -5 "$NOUP" | while IFS='|' read -r sub _ branch reason; do
            printf "      %-48s %s\n" "${sub# }" "${reason# }"
        done
        [ "$NOUP_N" -gt 5 ] && echo "      ... and $((NOUP_N - 5)) more"
    fi

    OTHER_N=$(( SKIPPED_N - WOULD_N - PINNED_N - NOUP_N ))
    if [ "$OTHER_N" -gt 0 ]; then
        echo ""
        echo "➡️   Skipped for other reasons ($OTHER_N):"
        grep -vF -e 'dry run, not pushed' -e 'pinned (' -e 'no upstream remote' "$SKIPPED" \
          | while IFS='|' read -r sub _ branch reason; do
                printf "      %-48s %-15s %s\n" "${sub# }" "[${branch# }]" "${reason# }"
            done
    fi
    rm -f "$WOULD" "$PINNED" "$NOUP"
fi

echo ""
echo "════════════════════════════════════════════════════════"
echo "    pushed with commits: $UPDATED_N    already current: $NOOP_N    needs a human: $MANUAL_N"
if [ "$MANUAL_N" -gt 0 ]; then
    echo "⚠️   Done — $MANUAL_N submodule(s) need manual attention (see above)."
else
    echo "✅  Done — all submodules processed."
fi

# A rejected push is an infrastructure fault, not a fork that needs thought, and it has
# to turn the job red. Run 39429097901 ended `success` with 47 of its 53
# commit-carrying pushes rejected, so the weekly cron could have no-opped indefinitely
# with nothing to show for it. Merge conflicts deliberately stay exit 0: those are
# expected, and want a human rather than an alarm.
if [ -s "$MANUAL" ] && grep -qF 'push failed' "$MANUAL"; then
    PUSHFAIL_N=$(grep -cF 'push failed' "$MANUAL")
    echo ""
    echo "::error::$PUSHFAIL_N fork push(es) rejected -- no commits reached those forks."
    exit 1
fi
