#!/bin/bash

# Use CMakePresets to configure, build and run tests
cmake --preset=coverage
cmake --build --preset=coverage
ctest --preset=coverage

# Generate coverage data
cd build/coverage
llvm-profdata merge -sparse default.profraw -o default.profdata

# Check if profile data was generated
if [ ! -f default.profdata ]; then
    echo "Error: Failed to merge coverage data."
    exit 1
fi

# Create coverage report directory
mkdir -p coverage_report

# Generate HTML report - exclude test files
llvm-cov show ./tests/konstrukt_tests -instr-profile=default.profdata \
         -ignore-filename-regex=".*tests/.*" \
         --format=html > coverage_report/index.html

# Export coverage data in a format Codecov can understand - exclude test files
llvm-cov export ./tests/konstrukt_tests -instr-profile=default.profdata \
         -ignore-filename-regex=".*tests/.*" \
         -format=lcov > coverage.lcov

# Print coverage summary - exclude test files
llvm-cov report ./tests/konstrukt_tests -instr-profile=default.profdata \
         -ignore-filename-regex=".*tests/.*"

echo "Coverage report generated in build/coverage/coverage_report/index.html"
echo "Coverage data for Codecov generated in build/coverage/coverage.lcov" 