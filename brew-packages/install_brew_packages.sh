#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INPUT_FILE="$SCRIPT_DIR/matched_packages.txt"

if [[ ! -f $INPUT_FILE ]]; then
  echo "File '$INPUT_FILE' not found!"
  exit 1
fi

MATCHED_COUNT=$(wc -l < "$INPUT_FILE")
echo "🚀 Starting Homebrew install for $MATCHED_COUNT packages from $INPUT_FILE..."
echo "💡 Press CTRL+C anytime to cancel."

trap 'echo -e "\n❌ Installation interrupted. Exiting..."; exit 1' SIGINT

while read -r line; do
  # Extract package name and optional version
  pkg=$(echo "$line" | awk '{print $1}')
  version=$(echo "$line" | awk '{print $2}')
  
  if brew list --versions "$pkg" > /dev/null 2>&1; then
    echo "✔️ $pkg is already installed."
  else
    echo "⬇️ Installing $pkg ..."
    brew install "$pkg"
  fi
done < "$INPUT_FILE"

echo "✅ All packages processed."
