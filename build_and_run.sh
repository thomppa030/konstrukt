#!/bin/bash

set -e

# --- Configuration ---
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build" 
EXEC_NAME="konstrukt"
NUM_CORES=$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 2)

# --- Default Build Settings ---
BUILD_TYPE="Release" # Default build type
COMPILER="clang"     # Default compiler
GENERATOR="Ninja"    # Default CMake generator
CLEAN_BUILD=false    # Perform a clean build?
INSTALL_CONAN=true   # Setup conan and install packages?
USE_LIBCPP=false     # Use libc++ with Clang?
VERBOSE=false        # Show verbose output?
RUN_BINARY=true      # Run the executable after building?
RUN_TESTS=false      # Run tests after building?
GENERATE_COVERAGE=false # Generate code coverage report?
APP_ARGS=""          # Arguments to pass to the application

# --- Argument Parsing ---
while [ $# -gt 0 ]; do
  case $1 in
    -d|--debug)
      BUILD_TYPE="Debug"
      shift
      ;;
    -r|--release)
      BUILD_TYPE="Release"
      shift
      ;;
    --build-type=*)
      BUILD_TYPE="${1#*=}"
      shift
      ;;
    -c|--coverage)
      BUILD_TYPE="Debug" 
      GENERATE_COVERAGE=true
      RUN_TESTS=true 
      shift
      ;;
    --skip-conan)
      INSTALL_CONAN=false
      shift
      ;;
    # Compiler Options
    --compiler=*)
      COMPILER="${1#*=}"
      shift
      ;;
    --gcc)
      COMPILER="gcc"
      USE_LIBCPP=false 
      shift
      ;;
    --clang)
      COMPILER="clang"
      shift
      ;;

    # Standard Library Options (Clang only)
    --libstdc++)
      USE_LIBCPP=false
      shift
      ;;
    --libc++)
      USE_LIBCPP=true
      shift
      ;;

    # Generator Options
    --generator=*)
      GENERATOR="${1#*=}"
      shift
      ;;
    --ninja)
      GENERATOR="Ninja"
      shift
      ;;
    --makefiles)
      GENERATOR="Unix Makefiles"
      shift
      ;;

    # Build Process Options
    --clean)
      CLEAN_BUILD=true
      shift
      ;;
    --verbose)
      VERBOSE=true
      shift
      ;;
    --cores=*)
      NUM_CORES="${1#*=}"
      shift
      ;;
    --no-run)
      RUN_BINARY=false
      shift
      ;;
    -t|--test)
      RUN_TESTS=true
      shift
      ;;

    # Help
    -h|--help)
      echo "Usage: $0 [options] [-- app_arg1 app_arg2 ...]"
      echo "Options:"
      echo "  Build Type:"
      echo "    -d, --debug              Build in Debug mode"
      echo "    -r, --release            Build in Release mode (default)"
      echo "    --build-type=<Type>      Specify CMake build type"
      echo "    -c, --coverage           Build with coverage reporting"
      echo "  Compiler:"
      echo "    --compiler=<gcc|clang>   Specify compiler (default: clang)"
      echo "    --gcc                    Use GCC compiler"
      echo "    --clang                  Use Clang compiler"
      echo "  Standard Library (Clang only):"
      echo "    --libc++                 Use libc++ with Clang"
      echo "    --libstdc++              Use libstdc++ with Clang (default)"
      echo "  Generator:"
      echo "    --generator=<Gen>        Specify CMake generator"
      echo "    --ninja                  Use Ninja generator"
      echo "    --makefiles              Use Makefiles generator"
      echo "  Build Process:"
      echo "    --clean                  Clean build directory before build"
      echo "    --cores=<N>              Number of cores for parallel build"
      echo "    --verbose                Enable verbose output"
      echo "  Execution:"
      echo "    --no-run                 Don't run the binary after building"
      echo "    -t, --test               Run tests after building"
      echo "  Help:"
      echo "    -h, --help               Show this help message"
      echo "Arguments after '--' are passed directly to the application."
      exit 0
      ;;

    # Separator for application arguments
    --)
      shift # Remove the '--'
      APP_ARGS="$@" # Assign remaining arguments to APP_ARGS
      break # Stop processing options
      ;;

    # Unknown option
    *)
      echo "Unknown option: $1" >&2
      echo "Use -h or --help for usage information." >&2
      exit 1
      ;;
  esac
done

# --- Helper Functions ---
print_status() {
  echo -e "\033[1;32m[BUILD]\033[0m $1"
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

# --- Dependency Checks ---
check_command() {
  command -v "$1" &> /dev/null
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
}

check_dependencies() {
  print_status "Checking build dependencies..."
  local error_found=false

  # Check Compiler
  if [ "$COMPILER" = "clang" ]; then
    if ! check_command clang || ! check_command clang++; then
      print_error "Clang compiler (clang/clang++) not found."
      case "$SYSTEM_TYPE" in
        arch) print_error "  Install with: sudo pacman -S clang" ;;
        debian) print_error "  Install with: sudo apt install clang" ;;
        macos) print_error "  Install with: brew install llvm" ;;
        *) print_error "  Please install clang/clang++ using your system's package manager." ;;
      esac
      error_found=true
    fi
    
    if [ "$USE_LIBCPP" = true ]; then
      print_verbose "Checking for libc++ headers..."
      if ! ls /usr/include/c++/v1/vector &> /dev/null && \
         ! ls /usr/local/include/c++/v1/vector &> /dev/null; then
        print_warning "libc++ headers might not be installed in standard locations."
        case "$SYSTEM_TYPE" in
          arch) print_warning "  If build fails, try: sudo pacman -S libc++" ;;
          debian) print_warning "  If build fails, try: sudo apt install libc++-dev libc++abi-dev" ;;
          macos) print_warning "  Ensure LLVM is installed: brew install llvm" ;;
          *) print_warning "  Please ensure libc++ headers are installed." ;;
        esac
      fi
    fi
  elif [ "$COMPILER" = "gcc" ]; then
    if ! check_command gcc || ! check_command g++; then
      print_error "GCC compiler (gcc/g++) not found."
      case "$SYSTEM_TYPE" in
        arch) print_error "  Install with: sudo pacman -S gcc" ;;
        debian) print_error "  Install with: sudo apt install build-essential" ;; 
        macos) print_error "  Install with: brew install gcc" ;;
        *) print_error "  Please install gcc/g++ using your system's package manager." ;;
      esac
      error_found=true
    fi
  else
    print_error "Unsupported compiler specified: $COMPILER"
    error_found=true
  fi

  # Check CMake
  if ! check_command cmake; then
    print_error "CMake not found."
    case "$SYSTEM_TYPE" in
      arch) print_error "  Install with: sudo pacman -S cmake" ;;
      debian) print_error "  Install with: sudo apt install cmake" ;;
      macos) print_error "  Install with: brew install cmake" ;;
      *) print_error "  Please install cmake using your system's package manager." ;;
    esac
    error_found=true
  fi

  # Check Conan
  if ! check_command conan; then
    print_error "Conan package manager not found."
    print_error "  Install with: pip install conan"
    error_found=true
  else
    # Check Conan version
    CONAN_VERSION_MAJOR=$(conan --version | grep -oP '(?<=Conan version )[0-9]+\.[0-9]+' | cut -d. -f1)
    if [ "$CONAN_VERSION_MAJOR" != "2" ]; then
      print_warning "You appear to be using Conan version $CONAN_VERSION_MAJOR.x, but Conan 2.x is recommended."
      print_warning "Consider upgrading with: pip install --upgrade conan"
    fi
  fi

  # Check Ninja if selected
  if [ "$GENERATOR" = "Ninja" ] && ! check_command ninja; then
    print_error "Ninja build tool not found (selected generator)."
    case "$SYSTEM_TYPE" in
      arch) print_error "  Install with: sudo pacman -S ninja" ;;
      debian) print_error "  Install with: sudo apt install ninja-build" ;;
      macos) print_error "  Install with: brew install ninja" ;;
      *) print_error "  Please install ninja using your system's package manager." ;;
    esac
    error_found=true
  fi

  # Check Coverage Tools if needed
  if [ "$GENERATE_COVERAGE" = true ]; then
    if ! check_command llvm-profdata || ! check_command llvm-cov; then
      print_error "Coverage tools (llvm-profdata, llvm-cov) not found, but coverage requested."
      case "$SYSTEM_TYPE" in
        arch) print_error "  Install with: sudo pacman -S llvm" ;;
        debian) print_error "  Install with: sudo apt install llvm" ;; 
        macos) print_error "  Install with: brew install llvm" ;;
        *) print_error "  Please install llvm-profdata and llvm-cov." ;;
      esac
      error_found=true
    elif [ "$COMPILER" != "clang" ]; then
      print_warning "Coverage generation is configured for Clang. Results with GCC might be inaccurate."
    fi
  fi

  if [ "$error_found" = true ]; then
    print_error "Please install the missing dependencies and try again."
    exit 1
  fi
  print_status "Dependency check passed."
}

# --- Relink compile_commands.json ---
relink_compile_commands() {
  print_status "Relinking compile_commands.json..."
  
  # Remove the old compile_commands.json if it exists
  if [ -f "$SCRIPT_DIR/compile_commands.json" ]; then
    rm "$SCRIPT_DIR/compile_commands.json"
  fi
  
  # Create a symbolic link to the new compile_commands.json
  local compile_commands_path=""
  
  # Check different possible locations for compile_commands.json
  if [ -f "$BUILD_DIR/compile_commands.json" ]; then
    compile_commands_path="$BUILD_DIR/compile_commands.json"
  elif [ -f "$BUILD_DIR/$BUILD_TYPE/compile_commands.json" ]; then
    compile_commands_path="$BUILD_DIR/$BUILD_TYPE/compile_commands.json"
  elif [ -f "$BUILD_DIR/coverage/compile_commands.json" ] && [ "$GENERATE_COVERAGE" = true ]; then
    compile_commands_path="$BUILD_DIR/coverage/compile_commands.json"
  fi
  
  if [ -n "$compile_commands_path" ]; then
    ln -sf "$compile_commands_path" "$SCRIPT_DIR/compile_commands.json"
    print_status "Relinked compile_commands.json successfully from $compile_commands_path"
  else
    print_warning "compile_commands.json not found in any build directory."
    # Perform a find to locate it
    local found_path=$(find "$BUILD_DIR" -name "compile_commands.json" -type f | head -n 1)
    if [ -n "$found_path" ]; then
      ln -sf "$found_path" "$SCRIPT_DIR/compile_commands.json"
      print_status "Found and relinked compile_commands.json from $found_path"
    fi
  fi
}

# --- Main Build Logic ---

# 0. Detect System
detect_system_type

# 1. Clean Build Directory if requested
if [ "$CLEAN_BUILD" = true ]; then
  print_status "Cleaning build directory..."
  if [ -f "$SCRIPT_DIR/clean.sh" ]; then
    source $SCRIPT_DIR/clean.sh
  else
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
  fi
fi

# 2. Create Build Directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
  print_status "Creating build directory..."
  mkdir -p "$BUILD_DIR"
fi

# 3. Check Dependencies
check_dependencies

# 4. Setup Conan using the module
if [ "$INSTALL_CONAN" = true ]; then
if [ -f "$SCRIPT_DIR/scripts/setup-conan.sh" ]; then
  print_status "Setting up Conan environment using module..."
  CONAN_ARGS=""
  [ "$VERBOSE" = true ] && CONAN_ARGS="$CONAN_ARGS --verbose"
  [ "$USE_LIBCPP" = true ] && CONAN_ARGS="$CONAN_ARGS --use-libcpp"
  # Add this to your build_and_run.sh
  bash scripts/setup-conan.sh "$SCRIPT_DIR" "$BUILD_DIR" --build-type="$BUILD_TYPE"
else
  print_error "Conan setup module not found at scripts/setup-conan.sh"
  print_warning "Please ensure the scripts directory exists with the setup-conan.sh script"
  exit 1
fi
fi

# 5. CMake Configuration and Build
cd "$SCRIPT_DIR"

if [ "$GENERATE_COVERAGE" = true ]; then
  print_status "Running CMake configuration with coverage enabled..."
  cmake --preset=coverage
  cmake --build --preset=coverage
  relink_compile_commands
elif [ "$BUILD_TYPE" = "Debug" ]; then
  print_status "Running CMake configuration in Debug mode..."
  cmake --preset=debug
  cmake --build --preset=debug
  relink_compile_commands
else
  print_status "Running CMake configuration in Release mode..."
  cmake --preset=release
  cmake --build --preset=release
  relink_compile_commands
fi

# 6. Find Executable
EXECUTABLE="$BUILD_DIR/${EXEC_NAME}"
if [ ! -f "$EXECUTABLE" ]; then
  if [ -f "$BUILD_DIR/$BUILD_TYPE/${EXEC_NAME}" ]; then
    EXECUTABLE="$BUILD_DIR/$BUILD_TYPE/${EXEC_NAME}"
  elif [ -f "$BUILD_DIR/bin/${EXEC_NAME}" ]; then
    EXECUTABLE="$BUILD_DIR/bin/${EXEC_NAME}"
  elif [ -f "$BUILD_DIR/bin/$BUILD_TYPE/${EXEC_NAME}" ]; then
    EXECUTABLE="$BUILD_DIR/bin/$BUILD_TYPE/${EXEC_NAME}"
  else
    EXECUTABLE=$(find "$BUILD_DIR" -name "${EXEC_NAME}" -type f -executable | head -n 1)
  fi
fi

print_status "Build successful!"

# 7. Handle tests and coverage together
if [ "$RUN_TESTS" = true ] || [ "$GENERATE_COVERAGE" = true ]; then
  # Determine the build directory
  if [ "$GENERATE_COVERAGE" = true ]; then
    TEST_DIR="$BUILD_DIR/coverage"
  elif [ "$BUILD_TYPE" = "Debug" ]; then
    TEST_DIR="$BUILD_DIR/Debug"
  else
    TEST_DIR="$BUILD_DIR/Release"
  fi
  
  # Find the test executable
  TEST_EXEC=$(find "$TEST_DIR" -name "konstrukt_tests" -type f -executable)
  if [ -n "$TEST_EXEC" ]; then
    echo "Found test executable at $TEST_EXEC"
    
    # Set up coverage if needed
    if [ "$GENERATE_COVERAGE" = true ]; then
      echo "Generating coverage reports..."
      cd "$TEST_DIR"
      
      # Run tests with coverage instrumentation
      echo "Running tests with coverage instrumentation..."
      LLVM_PROFILE_FILE="$(pwd)/default.profraw" "$TEST_EXEC"
      
      # Check if profile file was generated
      if [ ! -f default.profraw ]; then
          echo "Error: Coverage data was not generated. Check if tests are properly instrumented."
          exit 1
      fi
      
      # Generate coverage data
      llvm-profdata merge -sparse default.profraw -o default.profdata
      
      # Check if profile data was generated
      if [ ! -f default.profdata ]; then
          echo "Error: Failed to merge coverage data."
          exit 1
      fi
      
      # Create coverage report directory
      mkdir -p coverage_report
      
      # Generate HTML report - exclude test files
      llvm-cov show "$TEST_EXEC" -instr-profile=default.profdata \
               -ignore-filename-regex=".*tests/.*" \
               --format=html > coverage_report/index.html
      
      # Export coverage data in a format Codecov can understand - exclude test files
      llvm-cov export "$TEST_EXEC" -instr-profile=default.profdata \
               -ignore-filename-regex=".*tests/.*" \
               -format=lcov > coverage.lcov
      
      # Print coverage summary - exclude test files
      llvm-cov report "$TEST_EXEC" -instr-profile=default.profdata \
               -ignore-filename-regex=".*tests/.*"
      
      echo "Coverage report generated in $TEST_DIR/coverage_report/index.html"
      echo "Coverage data for Codecov generated in $TEST_DIR/coverage.lcov"
      
      # Return to the original directory
      cd "$SCRIPT_DIR"
    else
      # Just run the tests
      echo "Running tests..."
      "$TEST_EXEC"
    fi
  else
    echo "Error: Could not find test executable."
    echo "Searched in $TEST_DIR"
    exit 1
  fi
fi

# 8. Run Application
# Only run if tests/coverage weren't the primary goal
if [ "$RUN_TESTS" = false ] && [ "$GENERATE_COVERAGE" = false ] && [ "$RUN_BINARY" = true ]; then
  if [ -n "$EXECUTABLE" ] && [ -f "$EXECUTABLE" ] && [ -x "$EXECUTABLE" ]; then
    print_status "Running the application..."
    echo -e "\033[1;36m------------------------[Application Output]------------------------\033[0m"
    "$EXECUTABLE" $APP_ARGS
    EXIT_CODE=$?
    echo -e "\033[1;36m--------------------------------------------------------------------\033[0m"
    
    if [ $EXIT_CODE -ne 0 ]; then
      print_error "Application exited with code $EXIT_CODE"
      exit $EXIT_CODE
    fi
  else
    print_error "Executable not found or not executable."
    print_error "Looked for: $EXECUTABLE"
    print_error "Check your CMakeLists.txt to ensure the target name is '${EXEC_NAME}'"
    exit 1
  fi
fi

print_status "Script finished successfully."
exit 0
