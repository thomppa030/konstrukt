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

# --- Setup Conan ---
setup_conan() {
  local build_dir=$1
  local compiler=$2
  local build_type=$3
  local use_libcpp=$4
  local generator=$5
  local num_cores=$6
  local verbose=$7

  print_status "Configuring Conan..."
  
  # Configure compiler for Conan profile
  if [ "$compiler" = "clang" ]; then
    COMP_SETTINGS="compiler=clang"
    COMP_VERSION=$(clang --version | grep -oP 'version \K[0-9]+\.[0-9]+\.[0-9]+' | cut -d. -f1)
    
    if [ -z "$COMP_VERSION" ]; then
      COMP_VERSION=$(clang --version | head -n1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | cut -d. -f1)
    fi
    
    if [ -z "$COMP_VERSION" ]; then
      print_warning "Could not detect Clang version, assuming version 15"
      COMP_VERSION="15"
    fi
    
    print_verbose "Detected Clang version: $COMP_VERSION"
    
    if [ "$use_libcpp" = true ]; then
      print_status "Configuring Clang with libc++"
      STDLIB_SETTING="compiler.libcxx=libc++"
      CONAN_FLAGS="-stdlib=libc++"
    else
      print_status "Configuring Clang with libstdc++11"
      STDLIB_SETTING="compiler.libcxx=libstdc++11"
      CONAN_FLAGS=""
    fi
    
    export CC=clang
    export CXX=clang++
    export CXXFLAGS="$CONAN_FLAGS"
    export LDFLAGS="$CONAN_FLAGS"
    
  elif [ "$compiler" = "gcc" ]; then
    COMP_SETTINGS="compiler=gcc"
    COMP_VERSION=$(g++ -dumpversion | cut -d. -f1)
    STDLIB_SETTING="compiler.libcxx=libstdc++11"
    
    print_verbose "Detected GCC version: $COMP_VERSION"
    
    export CC=gcc
    export CXX=g++
  else
    print_error "Unsupported compiler: $compiler"
    exit 1
  fi
  
  if [ "$(uname)" = "Linux" ]; then
    DETECTED_OS="Linux"
  elif [ "$(uname)" = "Darwin" ]; then
    DETECTED_OS="Macos" # Conan uses "Macos" not "Darwin"
  else
    DETECTED_OS="$(uname)"
  fi
  
  PROFILE_FILE="$build_dir/custom_profile"
  mkdir -p "$build_dir"
  
  # Create the profile with proper formatting
  cat > "$PROFILE_FILE" << EOF
[settings]
arch=x86_64
build_type=$build_type
$COMP_SETTINGS
compiler.version=$COMP_VERSION
$STDLIB_SETTING
compiler.cppstd=20
os=$DETECTED_OS

[conf]
tools.build:compiler_executables={"c": "$CC", "cpp": "$CXX"}
tools.cmake.cmaketoolchain:generator=$generator
tools.build:jobs=$num_cores
tools.system.package_manager:mode=install
tools.system.package_manager:sudo=True
EOF

  print_verbose "Created Conan profile:"
  print_verbose "$(cat $PROFILE_FILE)"
  
  # Configure Conan global settings
  conan config set core.verbose_build=$verbose
  
  echo "$PROFILE_FILE"
}

# --- Run Conan Install ---
run_conan_install() {
  local project_dir=$1
  local build_dir=$2
  local profile_path=$3
  local build_type=$4
  local verbose=$5
  
  print_status "Running Conan to install dependencies (build type: $build_type)..."
  
  if [ "$verbose" = true ]; then
    CONAN_VERBOSE="--verbose"
  else
    CONAN_VERBOSE=""
  fi
  
  cd "$build_dir"
  conan install "$project_dir" --build=missing -g CMakeDeps -g CMakeToolchain -pr="$profile_path" -s build_type=$build_type $CONAN_VERBOSE
  
  TOOLCHAIN_PATH="$build_dir/$build_type/generators/conan_toolchain.cmake"
  if [ ! -f "$TOOLCHAIN_PATH" ]; then
    print_warning "Conan toolchain file not found at expected location: $TOOLCHAIN_PATH"
    print_status "Searching for toolchain file..."
    TOOLCHAIN_PATH=$(find "$build_dir" -name "conan_toolchain.cmake" | head -n 1)
    
    if [ -z "$TOOLCHAIN_PATH" ]; then
      print_error "Could not find any toolchain file."
      exit 1
    else
      print_status "Found toolchain at: $TOOLCHAIN_PATH"
    fi
  fi
  
  echo "$TOOLCHAIN_PATH"
}

# --- Find Vulkan Headers ---
find_vulkan_headers() {
  print_status "Locating Vulkan headers from Conan packages..."
  local vulkan_headers_path=$(find ~/.conan2/p -name "vulkan" -type d | grep include | head -n 1)
  
  if [ -n "$vulkan_headers_path" ] && [ -d "$vulkan_headers_path" ]; then
    print_status "Found Vulkan headers at: $vulkan_headers_path"
    echo "$vulkan_headers_path"
  else
    print_warning "Vulkan headers not found in Conan packages!"
    echo ""
  fi
}

# --- Export Include Paths ---
export_include_paths() {
  local vulkan_headers_path=$1
  
  if [ -n "$vulkan_headers_path" ] && [ -d "$vulkan_headers_path" ]; then
    print_status "Exporting Vulkan include paths..."
    export CPATH="$vulkan_headers_path:$CPATH"
    export C_INCLUDE_PATH="$vulkan_headers_path:$C_INCLUDE_PATH"
    export CPLUS_INCLUDE_PATH="$vulkan_headers_path:$CPLUS_INCLUDE_PATH"
    
    # For CMake
    export CMAKE_INCLUDE_PATH="$vulkan_headers_path:$CMAKE_INCLUDE_PATH"
    export CMAKE_PREFIX_PATH="$(dirname $(dirname $vulkan_headers_path)):$CMAKE_PREFIX_PATH"
    
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
  local compiler=$COMPILER
  local use_libcpp=$USE_LIBCPP
  local generator=$GENERATOR
  local num_cores=$NUM_CORES
  local verbose=$VERBOSE
  
  # Check that we have the necessary parameters
  if [ -z "$project_dir" ] || [ -z "$build_dir" ]; then
    print_error "Missing required parameters: project_dir and build_dir"
    print_error "Usage: source conan-setup.sh <project_dir> <build_dir> [options]"
    return 1
  fi
  
  # Process optional arguments
  shift 2
  while [ $# -gt 0 ]; do
    case $1 in
      --build-type=*)
        build_type="${1#*=}"
        ;;
      --compiler=*)
        compiler="${1#*=}"
        ;;
      --generator=*)
        generator="${1#*=}"
        ;;
      --cores=*)
        num_cores="${1#*=}"
        ;;
      --verbose)
        verbose=true
        ;;
      --use-libcpp)
        use_libcpp=true
        ;;
    esac
    shift
  done
  
  # Detect system
  local system_type=$(detect_system_type)
  
  # Set up Conan profile
  local profile_path=$(setup_conan "$build_dir" "$compiler" "$build_type" "$use_libcpp" "$generator" "$num_cores" "$verbose")
  
  # Run conan install
  local toolchain_path=$(run_conan_install "$project_dir" "$build_dir" "$profile_path" "$build_type" "$verbose")
  
  # Find Vulkan headers
  local vulkan_headers_path=$(find_vulkan_headers)
  
  # Export include paths
  export_include_paths "$vulkan_headers_path"
  
  print_status "Conan environment setup complete"
  echo "$profile_path"
}

# If this script is being executed directly, not sourced
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
  if [ $# -lt 2 ]; then
    print_error "Missing required parameters: project_dir and build_dir"
    print_error "Usage: $0 <project_dir> <build_dir> [options]"
    exit 1
  fi
  
  setup_conan_environment "$@"
fi
