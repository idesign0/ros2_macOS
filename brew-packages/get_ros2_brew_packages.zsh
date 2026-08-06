#!/bin/bash

ROS2_SRC="$HOME/humble-ros2/src"

echo "Extracting find_package names from ROS2 source..."
grep -rho 'find_package([^)]*)' "$ROS2_SRC" | sed 's/find_package(//; s/)//' | sort -u > $HOME/humble-ros2/src/brew-packages/found_packages.txt
echo "Found $(wc -l < $HOME/humble-ros2/src/brew-packages/found_packages.txt) unique packages."

echo "Getting installed Homebrew formula packages with versions..."
brew list --formula --versions > $HOME/humble-ros2/src/brew-packages/brew_packages_versions.txt
echo "Homebrew packages list saved."

echo "Matching ROS2 packages with Homebrew packages..."
grep -Ff $HOME/humble-ros2/src/brew-packages/found_packages.txt $HOME/humble-ros2/src/brew-packages/brew_packages_versions.txt > $HOME/humble-ros2/src/brew-packages/matched_packages.txt
echo "Matched packages saved to matched_packages.txt."

# Count matched lines
matched_count=$(wc -l < $HOME/humble-ros2/src/brew-packages/matched_packages.txt)
echo "Number of matched packages: $matched_count"
