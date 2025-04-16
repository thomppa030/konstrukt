#!/bin/bash
set -e

# --- Configuration ---
# Default build settings
BUILD_TYPE=${BUILD_TYPE:-"Release"}
COMPILER=${COMPILER:-"clang"} # Options: clang, gcc
CPPSTD=${CPPSTD:-"20"}
USE_LIBCPP=${USE_LIBCPP:-false} # Set to true to use libc++ with clang
GENERATOR=${GENERATOR:-"Ninja"}
VERBOSE=${VERBOSE:-true}
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

detect_system_info() {
  local os_type
  local arch_type

  # Detect OS
  if [ -f "/etc/arch-release" ] || [ -f "/etc/debian_version" ] || [ -f "/etc/fedora-release" ] || [ -f "/etc/redhat-release" ] || [ -f "/etc/SuSE-release" ]; then
    os_type="Linux"
  elif [ "$(uname)" = "Darwin" ]; then
    os_type="Macos"
  elif [ "$(uname -o)" = "Msys" ] || [ "$(uname -o)" = "Cygwin" ] || [[ "$(uname -s)" == *"MINGW64"* ]]; then
    os_type="Windows" # Basic detection for Windows via Git Bash/MSYS2/Cygwin
  else
    os_type="Unknown"
    print_warning "Could not reliably detect OS type. Conan might fail."
  fi

  # Detect Architecture
  arch_type=$(uname -m)
  case "$arch_type" in
    x86_64 | amd64)
      arch_type="x86_64"
      ;;
    arm64 | aarch64)
      arch_type="armv8" # Common Conan value for arm64
      ;;
    *)
      print_warning "Detected architecture '$arch_type' might not map directly to Conan settings. Using as is."
      ;;
  esac

  echo "$os_type $arch_type" # Return both values
}

# --- Generate Custom Conan Profile ---
generate_custom_profile() {
  local profile_path=$1
  local build_type=$2
  local compiler_type=$3
  local use_libcpp=$4
  local generator=$5

  local os_info
  local os_name
  local arch_name

  # Call detect_system_info and capture its output properly
  os_info=$(detect_system_info)
  os_name=$(echo "$os_info" | cut -d' ' -f1)
  arch_name=$(echo "$os_info" | cut -d' ' -f2)

  print_status "Generating custom Conan profile at: $profile_path"

  # --- Compiler Version Detection ---
  local compiler_version=""
  local conan_compiler_version="" # Version string compatible with Conan settings
  local version_output="" # Store the raw version output
  print_verbose "Attempting to detect version for compiler: $compiler_type"

  if ! command -v "$compiler_type" > /dev/null; then
      print_error "Compiler '$compiler_type' not found in PATH. Cannot determine version."
      return 1 # Signal failure
  fi

  set +e # Temporarily disable exit on error for version check commands
  if [ "$compiler_type" = "clang" ]; then
      version_output=$(clang --version 2>&1)
      # Regex attempts to find 'version X.Y.Z' or 'version X.Y '
      compiler_version=$(echo "$version_output" | grep -oP 'version \K[0-9]+\.[0-9]+(\.[0-9]+)?' | head -n 1)
      # Fallback parsing if regex fails
      if [ -z "$compiler_version" ]; then
          # Try another common clang format before giving up
          compiler_version=$(echo "$version_output" | head -n 1 | awk '{for(i=1;i<=NF;i++) if($i ~ /^[0-9]+\.[0-9]+/) {print $i; exit}}')
      fi
  elif [ "$compiler_type" = "gcc" ]; then
      version_output=$(gcc --version 2>&1)
      # Regex attempts to find ') X.Y.Z' or ') X.Y '
      compiler_version=$(echo "$version_output" | grep -oP '\) \K[0-9]+\.[0-9]+(\.[0-9]+)?' | head -n 1)
      # Fallback parsing
      if [ -z "$compiler_version" ]; then
           compiler_version=$(echo "$version_output" | head -n 1 | awk '{print $NF}') # Often the last field
      fi
  fi
  local detection_exit_code=$?
  set -e # Re-enable exit on error

  # Basic check if we got *any* version string that looks like X.Y...
  if [ $detection_exit_code -ne 0 ] || [[ ! "$compiler_version" =~ ^[0-9]+\.[0-9]+ ]]; then
      print_warning "Could not reliably parse $compiler_type version starting with X.Y from output:"
      # Only print output if verbose, might be noisy otherwise
      if [ "$VERBOSE" = true ]; then print_warning "$version_output"; fi
      compiler_version="" # Ensure it's empty if parsing failed
  fi

  if [ -z "$compiler_version" ]; then
      print_error "Failed to automatically detect compiler version for '$compiler_type'."
      print_error "Please ensure the compiler is installed and works, or manually specify compiler.version in your Conan profile."
      if [ "$VERBOSE" = true ]; then print_error "Raw version output:\n$version_output"; fi
      return 1 # Signal failure
  else
      print_verbose "Detected full $compiler_type version: $compiler_version"

      # --- Truncate version for Conan settings compatibility ---
      local major_version=$(echo "$compiler_version" | cut -d '.' -f 1)

      if [ "$compiler_type" = "clang" ]; then
          if [[ "$major_version" =~ ^[0-9]+$ ]] && [ "$major_version" -ge 9 ]; then
              print_verbose "Clang version >= 9 detected. Using Major version ('$major_version') for Conan profile (heuristic)."
              conan_compiler_version=$major_version
          else
              # Includes Clang < 9 and cases where major_version isn't purely numeric (shouldn't happen here)
              print_verbose "Clang version < 9 detected. Using Major.Minor version for Conan profile."
              conan_compiler_version=$(echo "$compiler_version" | cut -d '.' -f 1,2)
          fi
      elif [ "$compiler_type" = "gcc" ]; then
          print_verbose "GCC detected. Using Major.Minor version for Conan profile."
          conan_compiler_version=$(echo "$compiler_version" | cut -d '.' -f 1,2)
      else
          # Fallback for unknown compiler types (shouldn't happen with current script logic)
          conan_compiler_version=$compiler_version
      fi

      if [ -z "$conan_compiler_version" ]; then
          print_error "Failed to truncate compiler version '$compiler_version' for Conan."
          return 1
      fi
      print_verbose "Using Conan-compatible version: $conan_compiler_version"
      # --- End Version Truncation ---
  fi

  # --- End Compiler Version Detection ---
  if [ "$USE_LIBCPP" = true ]; then
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

  # Create the directory for the profile if it doesn't exist
  mkdir -p "$(dirname "$profile_path")"
  # Start writing the profile
  {
    echo "# Auto-generated Conan profile by setup-conan.sh"
    echo ""
    echo "[settings]"o
    echo "os=$os_name"
    echo "arch=$arch_name"
    echo "compiler=$compiler_type"
    if [ -n "$conan_compiler_version" ]; then
        echo "compiler.version=$conan_compiler_version"
    fi
    echo "compiler.cppstd=20"
    echo "build_type=$build_type"

    echo "$STDLIB_SETTING"

    echo ""
    echo "[conf]"
    echo "tools.build:compiler_executables={'c': '$CC', 'cpp': '$CXX'}"
    echo "tools.cmake.cmaketoolchain:generator=$generator"
    echo "tools.build:jobs=$NUM_CORES"
    echo "tools.system.package_manager:mode=install"
    echo "tools.system.package_manager:sudo=True"
  } > "$profile_path"

  print_status "Custom profile generated successfully."
  return 0 # Signal success
}

# --- Other Functions ---
# install_dependencies, setup_conan_environment, show_usage, etc. remain the same
# Make sure they properly check the return code of generate_custom_profile

# --- Install Dependencies with Conan ---
install_dependencies() {
  local project_dir=$1
  local build_dir=$2
  local build_type=$3
  local compiler=$4
  local use_libcpp=$5
  local generator=$6
  local verbose=$7
  local force_install=$8

  print_status "Installing dependencies with Conan (build type: $build_type, compiler: $compiler)..."

  local type_build_dir="$build_dir/$build_type"

  # Check for Conanfile
  local conanfile_path=""
  if [ -f "$project_dir/conanfile.py" ]; then
    conanfile_path="$project_dir/conanfile.py"
    print_verbose "Found conanfile.py"
  elif [ -f "$project_dir/conanfile.txt" ]; then
    conanfile_path="$project_dir/conanfile.txt"
    print_status "Found conanfile.txt"
  else
    print_error "No conanfile.txt or conanfile.py found in $project_dir"
    print_error "Please make sure your project has a valid Conan file at the root directory."
    return 1 # Use return code to signal failure
  fi
  print_verbose "Using Conan file: $conanfile_path"

  # --- Profile Generation ---
  local profile_name="custom_$(echo "$compiler" | tr '[:upper:]' '[:lower:]')_$(echo "$build_type" | tr '[:upper:]' '[:lower:]')"
  local profile_path="$type_build_dir/$profile_name.conanprofile"

  # Detect OS and Arch
  read -r detected_os detected_arch <<< "$(detect_system_info)"
  if [ "$detected_os" = "Unknown" ]; then
      print_error "Cannot proceed without a detected OS for the Conan profile."
      return 1 # Signal failure
  fi

  # Generate the profile, checking for failure (e.g., if version detection/truncation fails)
  if ! generate_custom_profile "$profile_path" "$build_type" "$compiler" "$use_libcpp" "$generator" "$detected_os" "$detected_arch"; then
      print_error "Failed to generate Conan profile."
      return 1 # Signal failure
  fi
  # --- End Profile Generation ---

  # Determine if we need to run Conan install
  local need_install=false
  if [ "$force_install" = true ]; then
    need_install=true
    print_status "Forced install requested"
  elif [ ! -d "$type_build_dir/generators" ]; then
    need_install=true
    print_status "No generators directory found, running Conan install"
  elif [ ! -f "$type_build_dir/generators/conan_toolchain.cmake" ]; then
    need_install=true
    print_status "Toolchain not found, running Conan install"
  elif [ ! -f "$type_build_dir/conan_deps_installed" ]; then
    need_install=true
    print_status "Marker file not found, running Conan install"
  fi

  if [ "$need_install" = true ]; then
    # Set verbosity flag for Conan
    local conan_verbose_flag=""
    if [ "$verbose" = true ]; then
       # Conan uses -v, -vv, -vvv for verbosity
       conan_verbose_flag="-v" # Or -v, adjust as needed
       print_verbose "Using Conan verbose flag: $conan_verbose_flag"
    fi

    print_status "Running Conan install (this might take a while)..."

    print_verbose "Command: conan install \"$conanfile_path\" --profile:host=\"$profile_path\" --profile:build=\"$profile_path\" --build=missing $conan_verbose_flag"

    set +e  # Temporarily disable exit on error to handle Conan errors
    conan install "$conanfile_path" \
        --profile:host="$profile_path" \
        --profile:build="$profile_path" \
        --build=missing \
        $conan_verbose_flag
    CONAN_EXIT_CODE=$?
    set -e  # Re-enable exit on error

    if [ $CONAN_EXIT_CODE -ne 0 ]; then
      print_error "Conan install failed with exit code $CONAN_EXIT_CODE"
      return $CONAN_EXIT_CODE
    fi

    # Create a marker file to indicate successful installation
    touch "$type_build_dir/conan_deps_installed"

    print_status "Dependencies installed successfully using custom profile"
  else
    print_status "Dependencies appear to be already installed with a compatible setup, skipping. Use --force-install to reinstall."
    print_status "Using profile: $profile_path (ensure it matches previous install if not forcing)"
  fi

  # Verify toolchain exists in the correct output folder
  local toolchain_path="$type_build_dir/generators/conan_toolchain.cmake"
  if [ ! -f "$toolchain_path" ]; then
    # This check might be redundant if Conan install succeeded, but good for sanity.
    print_error "Conan toolchain file not found at expected location after install: $toolchain_path"
    print_error "Conan install may have failed silently or placed files elsewhere."
    return 1 # Signal failure
  fi

  print_verbose "Toolchain file confirmed at: $toolchain_path"
  echo "$toolchain_path" # Return the path to the toolchain (stdout)
  return 0 # Signal success
}

show_usage() {
    echo "Usage: source setup-conan.sh <project_dir> <build_dir> [options]"
    echo "       ./setup-conan.sh <project_dir> <build_dir> [options]"
    echo ""
    echo "Sets up the build environment using Conan, generating a custom profile."
    echo ""
    echo "Required Arguments:"
    echo "  <project_dir>   Path to the project root (containing conanfile.py/txt)."
    echo "  <build_dir>     Path to the directory where build artifacts will be stored."
    echo ""
    echo "Options:"
    echo "  --build-type=<Type>  Specify the build type (e.g., Release, Debug). Default: Release."
    echo "  --compiler=<Comp>    Specify the compiler (e.g., clang, gcc). Default: clang."
    echo "  --use-libcpp         Use libc++ standard library (primarily for clang). Default: false."
    echo "  --generator=<Gen>    Specify the CMake generator (e.g., Ninja, \"Unix Makefiles\"). Default: Ninja."
    echo "  --verbose            Enable verbose output."
    echo "  --install-only       Only install Conan dependencies, do not perform other setup steps."
    echo "  --force-install      Force re-installation of Conan dependencies even if they seem up-to-date."
    echo "  -h, --help           Show this help message."
    echo ""
    echo "Environment Variables:"
    echo "  BUILD_TYPE, COMPILER, USE_LIBCPP, GENERATOR, VERBOSE, INSTALL_ONLY, FORCE_INSTALL can be set to override defaults."
}


# --- Main Function ---
# This is the main entry point when the script is sourced or executed
setup_conan_environment() {
  local project_dir_arg=$1
  local build_dir_arg=$2

  # Use defaults from environment or script header
  local build_type=${BUILD_TYPE:-"Release"}
  local compiler=${COMPILER:-"clang"}
  local use_libcpp=${USE_LIBCPP:-false}
  local generator=${GENERATOR:-"Ninja"}
  local verbose=${VERBOSE:-false}
  local install_only=${INSTALL_ONLY:-false}
  local force_install=${FORCE_INSTALL:-false}

  # Check that we have the necessary positional parameters
  if [ -z "$project_dir_arg" ] || [ -z "$build_dir_arg" ]; then
    print_error "Missing required parameters: project_dir and build_dir"
    show_usage
    return 1
  fi

  # Process optional arguments (--key=value)
  shift 2
  while [ $# -gt 0 ]; do
    case $1 in
      --build-type=*)
        build_type="${1#*=}"
        ;;
      --compiler=*)
        compiler="${1#*=}"
        ;;
      --use-libcpp)
        use_libcpp=true
        ;;
      --generator=*)
        generator="${1#*=}"
        ;;
      --verbose)
        verbose=true
        # Propagate verbosity setting for helper functions
        VERBOSE=true
        ;;
      --install-only)
        install_only=true
        ;;
      --force-install)
        force_install=true
        ;;
      *)
        print_error "Unknown option: $1"
        show_usage
        return 1
        ;;
    esac
    shift
  done

   # --- Parameter Validation ---
   if [[ "$compiler" != "clang" && "$compiler" != "gcc" ]]; then
      print_error "Unsupported compiler specified: '$compiler'. Use 'clang' or 'gcc'."
      return 1
   fi
   if [[ "$use_libcpp" = true && "$compiler" != "clang" ]]; then
      print_warning "--use-libcpp is typically used with clang. Ignoring for compiler '$compiler'."
      # Don't force it to false, let the profile generator handle default libcxx for gcc
   fi
   # --- End Parameter Validation ---

  # Convert to absolute paths with proper normalization
  # Use subshell for pwd to avoid changing script's CWD
  local current_pwd=$(pwd)
  local project_dir
  local build_dir

  # Handle relative paths robustly
  if [[ "$project_dir_arg" == /* ]]; then
      project_dir="$project_dir_arg"
  else
      project_dir="$current_pwd/$project_dir_arg"
  fi
  # Use realpath only if it exists and the directory exists
  if [ -d "$project_dir" ] && command -v realpath > /dev/null; then
     project_dir=$(realpath "$project_dir")
  elif [ ! -d "$project_dir" ]; then
     print_error "Project directory does not exist: $project_dir_arg (resolved to $project_dir)"
     return 1
  fi


  if [[ "$build_dir_arg" == /* ]]; then
      build_dir="$build_dir_arg"
  else
      build_dir="$current_pwd/$build_dir_arg"
  fi
  # Create the build directory early if it doesn't exist
  mkdir -p "$build_dir"
  if command -v realpath > /dev/null; then
     build_dir=$(realpath "$build_dir") # Normalize path
  fi

  print_verbose "Using project directory: $project_dir"
  print_verbose "Using build directory: $build_dir"
  print_verbose "Build Type: $build_type"
  print_verbose "Compiler: $compiler"
  print_verbose "Use libc++: $use_libcpp"
  print_verbose "Generator: $generator"
  print_verbose "Verbose: $verbose"
  print_verbose "Install Only: $install_only"
  print_verbose "Force Install: $force_install"

  # Install dependencies using custom profile
  local toolchain_path=""

  print_status "Starting dependency installation..."
  
  # Call the function directly instead of capturing output
  install_dependencies \
      "$project_dir" \
      "$build_dir" \
      "$build_type" \
      "$compiler" \
      "$use_libcpp" \
      "$generator" \
      "$verbose" \
      "$force_install"
  
  local install_exit_code=$?

  if [ "$install_exit_code" -ne 0 ]; then
      print_error "Dependency installation failed with exit code $install_exit_code."
      return "$install_exit_code"
  fi

  # If install-only flag is set, exit after installation
  if [ "$install_only" = true ]; then
    print_status "Install-only mode: dependencies installed, exiting without further setup."
    return 0
  fi

  print_status "Conan environment setup complete"

  # Directly define the expected toolchain path
  local toolchain_path="$build_dir/$build_type/generators/conan_toolchain.cmake"
  
  # Check if the toolchain file exists
  if [ ! -f "$toolchain_path" ]; then
      print_warning "Toolchain file not found at expected location: $toolchain_path"
      # Attempt to find it via fallback
      local found_toolchain
      found_toolchain=$(find "$build_dir/$build_type/generators" -name "conan_toolchain.cmake" 2>/dev/null | head -n 1)
      
      if [ -z "$found_toolchain" ]; then
         print_error "Critical: Toolchain file not found after successful install."
         return 1
      fi
      
      toolchain_path="$found_toolchain"
      print_status "Found toolchain via fallback: $toolchain_path"
  fi

  print_status "CMake Toolchain File: $toolchain_path"
  
  # Export the toolchain path for subsequent CMake commands
  export CONAN_CMAKE_TOOLCHAIN="$toolchain_path"
  print_status "Toolchain path exported as CONAN_CMAKE_TOOLCHAIN"

  return 0
}

# --- Script Execution Handling ---
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
  # Script is being executed directly
  if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    show_usage
    exit 0
  fi

  if [ $# -lt 2 ]; then
    print_error "Missing required parameters: project_dir and build_dir"
    show_usage
    exit 1
  fi

  # Call the main function, passing all arguments
  setup_conan_environment "$@"
  exit $? # Exit with the return code of the main function
else
  # Script is being sourced
  # Check for help flag when sourcing (optional, but can be useful)
   if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
     show_usage
     # Can't exit when sourced, just return
     return 0
   fi
   # If not asking for help, the user intends to source the setup function.
   # They should call `setup_conan_environment <proj> <build> [opts]` themselves after sourcing.
fi
