#!/bin/bash

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
EXEC_NAME="konstrukt"


BUILD_TYPE="Release"
COMPILER="clang"
GENERATOR="Ninja"
CLEAN_BUILD=false
USE_LIBCPP=false
NUM_CORES=$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 2)
VERBOSE=false

while [[$# -gt 0]]; do
  case $1 in
    --debug)
      BUILD_TYPE="Debug"
      shift
      ;;
    --release)
      BUILD_TYPE="Release"
      shift
      ;;
    --build-type=*)
      BUILD_TYPE="${1#*=}"
      shift
      ;;
    --compiler=*)
      COMPILER="${1#*=}"
      shift
      ;;
    --gcc)
      COMPILER="gcc"
      shift
      ;;
    --clang)
      COMPILER="clang"
      shift
      ;;
    --ninja)
      GENERATOR="Ninja"
      shift
      ;;
    --makefiles)
      COMPILER="Unix Makefiles"
      shift
      ;;
    --clean)
      CLEAN_BUILD=true
      shift
      ;;
    --libstdc++)
      USE_LIBCPP=false
      shift
      ;;
    --libc++)
      USE_LIBCPP=true
      shift
      ;;
    --verbose)
      VERBOSE=true
      shift
      ;;
    --cores=*)
      COMPILER="${1#*=}"
      shift
      ;;
    *)
      ARGS="$ARGS $1"
      shift
      ;;
  esac
done

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

if [ -f "/etc/arch-release" ]; then
    SYSTEM_TYPE="arch"
elif [ -f "/etc/debian_version" ]; then
    SYSTEM_TYPE="debian"
else
    SYSTEM_TYPE="unknown"
fi

check_dependencies() {
    print_status "Checking build dependencies on $SYSTEM_TYPE system..."
    
    if [ "$COMPILER" = "clang" ]; then
        if ! command -v clang &> /dev/null; then
            print_error "Clang compiler not found."
            
            if [ "$SYSTEM_TYPE" = "arch" ]; then
                print_error "Please install clang with: sudo pacman -S clang"
            elif [ "$SYSTEM_TYPE" = "debian" ]; then
                print_error "Please install clang with: sudo apt install clang"
            else
                print_error "Please install clang using your system's package manager."
            fi
            
            exit 1
        fi
        
        # Check for libc++ if we're using it
        if [ "$USE_LIBCPP" = true ]; then
            if [ "$SYSTEM_TYPE" = "arch" ]; then
                if ! pacman -Q libc++ &> /dev/null; then
                    print_warning "libc++ may not be installed. If build fails, install libc++ with: sudo pacman -S libc++"
                fi
            elif [ "$SYSTEM_TYPE" = "debian" ]; then
                if ! dpkg -l | grep -q libc++-dev; then
                    print_warning "libc++ headers may not be installed. If build fails, install with: sudo apt install libc++-dev libc++abi-dev"
                fi
            else
                if ! ls /usr/include/c++/v1/vector &> /dev/null && ! ls /usr/local/include/c++/v1/vector &> /dev/null; then
                    print_warning "libc++ headers may not be installed. Please install them using your system's package manager."
                fi
            fi
        fi
    elif [ "$COMPILER" = "gcc" ]; then
        if ! command -v g++ &> /dev/null; then
            print_error "GCC compiler not found."
            
            if [ "$SYSTEM_TYPE" = "arch" ]; then
                print_error "Please install gcc with: sudo pacman -S gcc"
            elif [ "$SYSTEM_TYPE" = "debian" ]; then
                print_error "Please install g++ with: sudo apt install g++"
            else
                print_error "Please install g++ using your system's package manager."
            fi
            
            exit 1
        fi
    fi
    
    if ! command -v cmake &> /dev/null; then
        print_error "CMake not found."
        
        if [ "$SYSTEM_TYPE" = "arch" ]; then
            print_error "Please install cmake with: sudo pacman -S cmake"
        elif [ "$SYSTEM_TYPE" = "debian" ]; then
            print_error "Please install cmake with: sudo apt install cmake"
        else
            print_error "Please install cmake using your system's package manager."
        fi
        
        exit 1
    fi
    
    if ! command -v conan &> /dev/null; then
        print_error "Conan package manager not found."
        print_error "Please install conan with: pip install conan"
        exit 1
    fi
    
    # Check Conan version - must be Conan 2.x
    CONAN_VERSION=$(conan --version | grep -oP '(?<=Conan version )[0-9]+\.[0-9]+' | cut -d. -f1)
    if [ "$CONAN_VERSION" != "2" ]; then
        print_warning "You appear to be using Conan version $CONAN_VERSION.x, but Conan 2.x is recommended."
        print_warning "Consider upgrading with: pip install --upgrade conan"
    fi
}

setup_compiler() {
    # Configure compiler for Conan profile
    if [ "$COMPILER" = "clang" ]; then
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
        
        if [ "$USE_LIBCPP" = true ]; then
            print_status "Configuring Clang with libc++"
            STDLIB_SETTING="compiler.libcxx=libc++"
            CONAN_FLAGS="-stdlib=libc++"
            
            if [ "$SYSTEM_TYPE" = "arch" ]; then
                if [ -d "/usr/include/c++/v1" ]; then
                    print_verbose "Found libc++ headers in /usr/include/c++/v1"
                elif [ -d "/usr/lib/llvm/include/c++/v1" ]; then
                    print_verbose "Found libc++ headers in /usr/lib/llvm/include/c++/v1"
                    # You might need to add include paths here if they're non-standard
                fi
            fi
        else
            print_status "Configuring Clang with libstdc++11"
            STDLIB_SETTING="compiler.libcxx=libstdc++11"
            CONAN_FLAGS=""
        fi
        
        export CC=clang
        export CXX=clang++
        export CXXFLAGS="$CONAN_FLAGS"
        export LDFLAGS="$CONAN_FLAGS"
        
    elif [ "$COMPILER" = "gcc" ]; then
        COMP_SETTINGS="compiler=gcc"
        COMP_VERSION=$(g++ -dumpversion | cut -d. -f1)
        STDLIB_SETTING="compiler.libcxx=libstdc++11"
        
        print_verbose "Detected GCC version: $COMP_VERSION"
        
        export CC=gcc
        export CXX=g++
    else
        print_error "Unsupported compiler: $COMPILER"
        exit 1
    fi
    
    if [ "$(uname)" = "Linux" ]; then
        DETECTED_OS="Linux"
    elif [ "$(uname)" = "Darwin" ]; then
        DETECTED_OS="Macos" # Conan uses "Macos" not "Darwin"
    else
        DETECTED_OS="$(uname)"
    fi
    
    PROFILE_FILE="$BUILD_DIR/custom_profile"
    mkdir -p "$BUILD_DIR"
    
    cat > "$PROFILE_FILE" << EOF
[settings]
arch=x86_64
build_type=$BUILD_TYPE
$COMP_SETTINGS
compiler.version=$COMP_VERSION
$STDLIB_SETTING
compiler.cppstd=20
os=$DETECTED_OS

[conf]
tools.build:compiler_executables={"c": "$CC", "cpp": "$CXX"}
tools.cmake.cmaketoolchain:generator=Ninja
tools.build:jobs=$NUM_CORES
EOF

    print_verbose "Created Conan profile:"
    print_verbose "$(cat $PROFILE_FILE)"
}


if [ "$CLEAN_BUILD" = true ]; then
    print_status "Cleaning build directory..."
    source $SCRIPT_DIR/clean.sh
fi

if [ ! -d "$BUILD_DIR" ]; then
    print_status "Creating build directory..."
    mkdir -p "$BUILD_DIR"
    NEED_CONAN=true
fi

check_dependencies

setup_compiler

cd "$BUILD_DIR"

if [ "$NEED_CONAN" = true ] || [ "$CLEAN_BUILD" = true ] || [ ! -d "$BUILD_DIR/$BUILD_TYPE" ] || [ ! -f "$BUILD_DIR/$BUILD_TYPE/generators/conan_toolchain.cmake" ]; then
    print_status "Running Conan to install dependencies (build type: $BUILD_TYPE, compiler: $COMPILER)..."
    
    if [ "$VERBOSE" = true ]; then
        CONAN_VERBOSE="--verbose"
    else
        CONAN_VERBOSE=""
    fi
    
    conan install .. --build=missing -g CMakeDeps -g CMakeToolchain -pr="$PROFILE_FILE" -s build_type=$BUILD_TYPE $CONAN_VERBOSE
fi


TOOLCHAIN_PATH="$BUILD_DIR/$BUILD_TYPE/generators/conan_toolchain.cmake"
if [ ! -f "$TOOLCHAIN_PATH" ]; then
    print_error "Conan toolchain file not found at expected location: $TOOLCHAIN_PATH"
    print_status "Searching for toolchain file..."
    TOOLCHAIN_PATH=$(find "$BUILD_DIR" -name "conan_toolchain.cmake" | head -n 1)
    
    if [ -z "$TOOLCHAIN_PATH" ]; then
        print_error "Could not find any toolchain file."
        exit 1
    else
        print_status "Found toolchain at: $TOOLCHAIN_PATH"
    fi
fi

cd "$SCRIPT_DIR"

print_status "Running CMake configuration with $COMPILER compiler in $BUILD_TYPE mode..."
if [ "$BUILD_TYPE" = "Debug" ]; then
  cmake --preset=ninja-debug
print_status "Building the project with $NUM_CORES cores..."
  cmake --build --preset debug
else
  cmake --preset=ninja-release
print_status "Building the project with $NUM_CORES cores..."
  cmake --build --preset release
fi

# Create a symlink to compile_commands.json in the project root for tools like clangd 
if [ -f "$BUILD_DIR/$BUILD_TYPE/compile_commands.json" ] && [ ! -f "$SCRIPT_DIR/compile_commands.json" ]; then
    print_status "Creating symlink to compile_commands.json in project root for IDE integration"
    ln -sf "$BUILD_DIR/$BUILD_TYPE/compile_commands.json" "$SCRIPT_DIR/compile_commands.json"
fi

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

if [ -n "$EXECUTABLE" ] && [ -f "$EXECUTABLE" ] && [ -x "$EXECUTABLE" ]; then
    print_status "Build successful! Running the application..."
    echo -e "\033[1;36m------------------------[Application Output]------------------------\033[0m"
    "$EXECUTABLE" $ARGS
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
