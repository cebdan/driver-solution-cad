#!/bin/bash
# Quick install script for dependencies
# Run this in terminal: bash QUICK_INSTALL.sh

echo "Installing dependencies..."

# Fix permissions (requires sudo)
echo "Fixing permissions..."
sudo chown -R $(whoami) /usr/local/var/log
chmod u+w /usr/local/var/log

# Install packages
echo "Installing CMake, Ninja, and Google Test..."
brew install cmake ninja googletest

echo "Done! Now you can build the project:"
echo "  cd build"
echo "  cmake -G Ninja .."
echo "  ninja"
echo "  ctest --output-on-failure"

