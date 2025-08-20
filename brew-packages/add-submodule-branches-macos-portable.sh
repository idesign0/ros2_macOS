#!/usr/bin/env bash
set -euo pipefail

git submodule update --init --recursive || true

lines=$(git config --file .gitmodules --get-regexp '^submodule\..*\.path$' 2>/dev/null || true)
if [ -z "$lines" ]; then
  echo "No submodules found in .gitmodules"
  exit 0
fi

changed=0

while IFS= read -r line; do
  key=${line%% *}
  path=${line#* }
  name=${key#submodule.}
  name=${name%.path}

  if [ ! -d "$path" ]; then
    echo "Skipping $name ($path): directory not present. Run 'git submodule update --init' first if needed."
    continue
  fi

  pushd "$path" > /dev/null

  branch=$(git symbolic-ref --quiet --short HEAD 2>/dev/null || true)

  if [ -z "$branch" ]; then
    found=""
    while IFS= read -r rline; do
      [ -z "$rline" ] && continue
      case "$rline" in
        *'->'* ) continue ;;
      esac
      # trim leading whitespace
      trimmed="${rline#"${rline%%[![:space:]]*}"}"
      if [[ "$trimmed" == */* ]]; then
        short=${trimmed#*/}
      else
        short="$trimmed"
      fi
      found="$short"
      break
    done < <(git branch -r --contains HEAD 2>/dev/null || true)
    branch="$found"
  fi

  if [ -z "$branch" ]; then
    if git show-ref --verify --quiet refs/remotes/origin/main; then
      branch="main"
    elif git show-ref --verify --quiet refs/remotes/origin/master; then
      branch="master"
    fi
  fi

  popd > /dev/null

  if [ -n "$branch" ]; then
    git config -f .gitmodules "submodule.$name.branch" "$branch"
    echo "Set submodule.$name.branch = $branch (path: $path)"
    changed=1
  else
    echo "Could not determine branch for submodule $name ($path) — left unchanged."
  fi
done <<< "$lines"

if [ "$changed" -ne 0 ]; then
  git add .gitmodules
  git commit -m "Add branch entries to .gitmodules for submodules"
  echo "Committed .gitmodules changes."
else
  echo "No changes made to .gitmodules."
fi

