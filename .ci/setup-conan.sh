#!/bin/bash
set -e

# --- Configuration ---
# Default build settings
BUILD_TYPE=${BUILD_TYPE:-"Release"}
COMPILER=${COMPILER:-"clang"}
USE_LIBCPP=${USE_LIBCPP:-false}
GENERATOR=${GENERATOR:-"Ninja"}
VERBOSE=${VERBOSE:-false}
NUM_CORES=${NUM_CORES:-$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 2)}
INSTALL_ONLY=${INSTALL_ONLY:-false}
FORCE_INSTALL=${FORCE_INSTALL:-false}

# --- Helper Functions ---
print_status() {
  echo -e "\033[1;32m[CONAN]\033[0m $1"
}

print_warning() {
  echo -e "\033[1;33m[WARNING]\033[0m $1"
}

print_error() {
  echo -e "\033[1;31m[ERROR]\033[0m $1"
}

print_verbose(){
  if [ "$VERBOSE" = true ]; then
    echo -e "\033[1;36m[VERBOSE]\033[0m $1"
  fi
}

detect_system_type() {
  if [ -f "/etc/arch-release" ]; then
    SYSTEM_TYPE="arch"
  elif [ -f "/etc/debian_version" ]; then
    SYSTEM_TYPE="debian"
  elif [ "$(uname)" = "Darwin" ]; then
    SYSTEM_TYPE="macos"
  else
    SYSTEM_TYPE="unknown"
  fi
  print_verbose "Detected system type: $SYSTEM_TYPE"
  echo "$SYSTEM_TYPE"
}

# --- Install Dependencies with Conan ---
install_dependencies() {
  local project_dir=$1
  local build_dir=$2
  local build_type=$3
  local verbose=$4
  local force_install=$5
  
  print_status "Installing dependencies with Conan (build type: $build_type)..."
  
  # Determine the appropriate build directory based on build type
  local type_build_dir="$build_dir/$build_type"
  mkdir -p "$type_build_dir"
  
  # First check if conanfile.py exists in the project directory
  if [ ! -f "$project_dir/conanfile.py" ]; then
    print_error "conanfile.py not found in $project_dir"
    print_warning "Checking for conanfile.txt instead..."
    
    if [ ! -f "$project_dir/conanfile.txt" ]; then
      print_error "No conanfile.txt or conanfile.py found in $project_dir"
      print_error "Please make sure your project has a valid Conan file at the root directory."
      exit 1
    else
      print_status "Found conanfile.txt"
    fi
  else
    print_verbose "Found conanfile.py"
  fi
  
  # Determine if we need to run Conan install
  local need_install=false
  if [ "$force_install" = true ]; then
    need_install=true
    print_status "Forced install requested"
  elif [ ! -d "$type_build_dir/generators" ]; then
    need_install=true
    print_status "No generators found, running Conan install"
  elif [ ! -f "$type_build_dir/generators/conan_toolchain.cmake" ]; then
    need_install=true
    print_status "Toolchain not found, running Conan install"
  elif [ ! -f "$type_build_dir/conan_deps_installed" ]; then
    need_install=true
    print_status "First-time install or incomplete previous install"
  fi
  
  if [ "$need_install" = true ]; then
    # Set verbosity flag for Conan
    if [ "$verbose" = true ]; then
      CONAN_VERBOSE="--verbose"
    else
      CONAN_VERBOSE=""
    fi
    
    # Create the build directory if it doesn't exist
    print_verbose "Creating build directory: $type_build_dir"
    mkdir -p "$type_build_dir"
    
    # Go to the build directory
    cd "$type_build_dir"
    
    print_status "Running Conan install (this might take a while)..."
    
    # Use the default profile but override build_type
    print_verbose "Command: conan install \"$project_dir\" --build=missing -g CMakeDeps -g CMakeToolchain -s build_type=$build_type $CONAN_VERBOSE"
    
    set +e  # Temporarily disable exit on error to handle Conan errors
    conan install "$project_dir" --build=missing -g CMakeDeps -g CMakeToolchain -s build_type=$build_type $CONAN_VERBOSE
    CONAN_EXIT_CODE=$?
    set -e  # Re-enable exit on error
    
    if [ $CONAN_EXIT_CODE -ne 0 ]; then
      print_error "Conan install failed with exit code $CONAN_EXIT_CODE"
      print_error "Try running with --verbose for more information"
      exit $CONAN_EXIT_CODE
    fi
    
    # Create a marker file to indicate successful installation
    touch "$type_build_dir/conan_deps_installed"
    
    print_status "Dependencies installed successfully"
  else
    print_status "Dependencies appear to be already installed, skipping. Use --force-install to reinstall."
  fi
  
  # Verify toolchain exists
  local toolchain_path="$type_build_dir/generators/conan_toolchain.cmake"
  if [ ! -f "$toolchain_path" ]; then
    print_warning "Conan toolchain file not found at expected location: $toolchain_path"
    print_status "Searching for toolchain file..."
    toolchain_path=$(find "$build_dir" -name "conan_toolchain.cmake" | head -n 1)
    
    if [ -z "$toolchain_path" ]; then
      print_error "Could not find any toolchain file. Conan install may have failed."
      exit 1
    else
      print_status "Found toolchain at: $toolchain_path"
    fi
  fi
  
  print_verbose "Toolchain file: $toolchain_path"
  echo "$toolchain_path"
}
# --- Export Include Paths ---
export_include_paths() {
  local vulkan_headers_path=$1
  
  if [ -n "$vulkan_headers_path" ] && [ -d "$vulkan_headers_path" ]; then
    print_status "Exporting Vulkan include paths..."
    
    # Avoid prepending to empty variables which can lead to path issues with leading colons
    if [ -n "$CPATH" ]; then
      export CPATH="$vulkan_headers_path:$CPATH"
    else
      export CPATH="$vulkan_headers_path"
    fi
    
    if [ -n "$C_INCLUDE_PATH" ]; then
      export C_INCLUDE_PATH="$vulkan_headers_path:$C_INCLUDE_PATH"
    else
      export C_INCLUDE_PATH="$vulkan_headers_path"
    fi
    
    if [ -n "$CPLUS_INCLUDE_PATH" ]; then
      export CPLUS_INCLUDE_PATH="$vulkan_headers_path:$CPLUS_INCLUDE_PATH"
    else
      export CPLUS_INCLUDE_PATH="$vulkan_headers_path"
    fi
    
    # For CMake
    if [ -n "$CMAKE_INCLUDE_PATH" ]; then
      export CMAKE_INCLUDE_PATH="$vulkan_headers_path:$CMAKE_INCLUDE_PATH"
    else
      export CMAKE_INCLUDE_PATH="$vulkan_headers_path"
    fi
    
    local prefix_path=$(dirname "$(dirname "$vulkan_headers_path")")
    if [ -n "$CMAKE_PREFIX_PATH" ]; then
      export CMAKE_PREFIX_PATH="$prefix_path:$CMAKE_PREFIX_PATH"
    else
      export CMAKE_PREFIX_PATH="$prefix_path"
    fi
    
    print_verbose "Updated include paths:"
    print_verbose "CPATH=$CPATH"
    print_verbose "C_INCLUDE_PATH=$C_INCLUDE_PATH"
    print_verbose "CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH"
    print_verbose "CMAKE_INCLUDE_PATH=$CMAKE_INCLUDE_PATH"
    print_verbose "CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH"
  else
    print_warning "Skipping include path export: No valid Vulkan headers path provided"
  fi
}

# --- Main Function ---
# This is the main entry point when the script is sourced
# It sets up everything and returns the profile path
setup_conan_environment() {
  local project_dir=$1
  local build_dir=$2
  local build_type=$BUILD_TYPE
  local verbose=$VERBOSE
  local install_only=$INSTALL_ONLY
  local force_install=$FORCE_INSTALL
  
  # Check that we have the necessary parameters
  if [ -z "$project_dir" ] || [ -z "$build_dir" ]; then
    print_error "Missing required parameters: project_dir and build_dir"
    print_error "Usage: source setup-conan.sh <project_dir> <build_dir> [options]"
    return 1
  fi
  
  # Convert to absolute paths with proper normalization
  if [[ ! "$project_dir" = /* ]]; then
    if [ "$project_dir" = "." ]; then
      project_dir="$(pwd)"
    else
      project_dir="$(realpath "$project_dir")"
    fi
  fi
  
  if [[ ! "$build_dir" = /* ]]; then
    # Create the directory if it doesn't exist
    mkdir -p "$build_dir"
    build_dir="$(realpath "$build_dir")"
  fi
  
  print_verbose "Using project directory: $project_dir"
  print_verbose "Using build directory: $build_dir"
  
  # Process optional arguments
  shift 2
  while [ $# -gt 0 ]; do
    case $1 in
      --build-type=*)
        build_type="${1#*=}"
        ;;
      --verbose)
        verbose=true
        ;;
      --install-only)
        install_only=true
        ;;
      --force-install)
        force_install=true
        ;;
    esac
    shift
  done
  
  # Install dependencies using default profile
  local toolchain_path=$(install_dependencies "$project_dir" "$build_dir" "$build_type" "$verbose" "$force_install")
  
  # If install-only flag is set, exit after installation
  if [ "$install_only" = true ]; then
    print_status "Install-only mode: dependencies installed, exiting without further setup."
    return 0
  fi
  
  # Find Vulkan headers
  local vulkan_headers_path=$(find_vulkan_headers)
  
  # Export include paths
  export_include_paths "$vulkan_headers_path"
  
  print_status "Conan environment setup complete"
}
# If this script is being executed directly, not sourced
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
  if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    show_usage
    exit 0
  fi

  if [ $# -lt 2 ]; then
    print_error "Missing required parameters: project_dir and build_dir"
    show_usage
    exit 1
  fi
  
  setup_conan_environment "$@"
else
  # If being sourced, check for help flag
  if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    show_usage
    return 0
  fi
fi
