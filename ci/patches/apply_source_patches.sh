#!/bin/bash
# Build-time source patches for submodule packages — applied in the composite
# setup AFTER submodule checkout, BEFORE colcon build. No forks needed; edits
# the submodule working tree in place. Idempotent (safe to re-run per job).
#
# Only put CLEAN, verified transforms here. Packages needing a real port
# (io_service::work -> executor_work_guard, resolver::iterator -> range resolve,
# etc.) get their own reviewed patch, not a blind sed.
set -u
ROOT="${1:-.}"

# BSD (macOS) vs GNU sed in-place flag
if sed --version >/dev/null 2>&1; then SEDI=(-i); else SEDI=(-i ''); fi

_pkg_dir() {
  find "$ROOT" -type d -name "$1" -not -path '*/build/*' -not -path '*/install/*' 2>/dev/null | head -1
}

# --- Lane 2 (Boost 1.90): boost::asio::io_service was removed (== io_context).
#     Only for packages that use the bare TYPE (no ::work / resolver::iterator). ---
patch_io_service_typeonly() {
  local dir; dir="$(_pkg_dir "$1")"; [ -z "$dir" ] && { echo "skip $1 (absent)"; return 0; }
  # bail if this package also uses the removed ::work API — needs a real port
  if grep -rq 'io_service::work' "$dir" 2>/dev/null; then
    echo "skip $1 (uses io_service::work — needs full port, not this sed)"; return 0
  fi
  local files; files="$(grep -rl 'io_service' "$dir" \
      --include='*.h' --include='*.hpp' --include='*.ipp' --include='*.cpp' --include='*.cc' 2>/dev/null)"
  [ -z "$files" ] && return 0
  echo "$files" | while IFS= read -r f; do
    sed "${SEDI[@]}" 's/asio::io_service/asio::io_context/g' "$f"
  done
  echo "patched io_service -> io_context in $1 ($dir)"
}

patch_io_service_typeonly hls_lfcd_lds_driver

echo "source patches applied."
