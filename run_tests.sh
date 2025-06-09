#!/bin/bash
# Test script for CakemuOoO processor
# This script compiles and runs the RISC-V test programs

set -e  # Exit on any error

# Directory setup
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_DIR="${SCRIPT_DIR}/tests"
BUILD_DIR="${SCRIPT_DIR}/build"
BIN_DIR="${SCRIPT_DIR}/bin"

# Create binary directory if it doesn't exist
mkdir -p "${BIN_DIR}"

# Check for RISC-V toolchain
if ! command -v riscv64-unknown-elf-gcc &> /dev/null; then
    echo "RISC-V toolchain not found. Please install it first."
    exit 1
fi

# Function to compile a test program
compile_test() {
    local test_name="$1"
    echo "Compiling ${test_name}.s..."
    
    # Compile assembly to object file
    riscv64-unknown-elf-gcc -march=rv32i -mabi=ilp32 -nostdlib -nostartfiles -T "${SCRIPT_DIR}/tests/link.ld" \
        "${TEST_DIR}/${test_name}.s" -o "${BIN_DIR}/${test_name}.elf"
    
    # Extract binary
    riscv64-unknown-elf-objcopy -O binary "${BIN_DIR}/${test_name}.elf" "${BIN_DIR}/${test_name}.bin"
    
    echo "Compiled ${test_name}.bin successfully."
}

# Function to run a test
run_test() {
    local test_name="$1"
    local predictor_type="$2"
    local simulation_time="$3"
    local generate_report="$4"
    
    echo "Running test ${test_name} with predictor ${predictor_type}..."
    
    # Build command
    cmd="${BUILD_DIR}/cakemu_ooo -f ${BIN_DIR}/${test_name}.bin -t ${simulation_time} -p ${predictor_type}"
    
    # Add report generation if requested
    if [ "${generate_report}" = "true" ]; then
        cmd="${cmd} -r -o ${BIN_DIR}/${test_name}_${predictor_type}_report.txt -c ${BIN_DIR}/${test_name}_${predictor_type}_data.csv"
    fi
    
    # Run the simulation
    echo "Command: ${cmd}"
    eval "${cmd}"
    
    echo "Test ${test_name} completed."
    echo "----------------------------"
}

# Check if the simulator is built
if [ ! -f "${BUILD_DIR}/cakemu_ooo" ]; then
    echo "Simulator not found. Building..."
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    cmake ..
    make
    cd "${SCRIPT_DIR}"
fi

# Create linker script if it doesn't exist
if [ ! -f "${TEST_DIR}/link.ld" ]; then
    cat > "${TEST_DIR}/link.ld" << 'EOF'
OUTPUT_ARCH( "riscv" )
ENTRY(_start)

SECTIONS
{
  . = 0x00000000;
  .text : { *(.text) }
  . = 0x00001000;
  .data : { *(.data) }
  .bss : { *(.bss) }
}
EOF
fi

# Compile all test programs
echo "Compiling test programs..."
compile_test "branch_heavy_test"
compile_test "memory_test"
compile_test "alu_test"
compile_test "comprehensive_test"

# Run tests with different branch predictors
echo "Running tests with different branch predictors..."

# Array of predictor types to test
predictors=("always_not_taken" "always_taken" "static_btfn" "one_bit" "two_bit" "gshare" "tournament")

# Test each program with each predictor
for test in "branch_heavy_test" "memory_test" "alu_test" "comprehensive_test"; do
    echo "===== Testing ${test} ====="
    for predictor in "${predictors[@]}"; do
        run_test "${test}" "${predictor}" 10000 true
    done
done

echo "All tests completed."
