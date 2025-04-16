#!/bin/bash

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

print_status() {
  echo -e "\033[1;34m[CLEAN]\033[0m $1"
}

print_status "Cleaning build directory"

if [ -d "$BUILD_DIR" ]; then
  print_status "Removing build directory: $BUILD_DIR"
  rm -rf "$BUILD_DIR"
else
  print_status "Build directory doesn't exist. Nothing to clean."
fi

print_status "Clean completed successfully"
