# RISC-V Assembly Test Program: ALU Operations Test
# This program tests various ALU operations and data dependencies

.text
.globl _start

_start:
    # Initialize registers
    li x1, 42          # Base value
    li x2, 7           # Operand
    li x3, 0           # Result accumulator
    li x4, 100         # Loop count
    li x5, 0           # Loop counter

    # Test 1: Arithmetic operations with immediate data dependencies
loop1:
    addi x6, x1, 5     # Add immediate
    addi x7, x6, -2    # Subtract immediate (using addi with negative value)
    add x8, x6, x7     # Add - data dependency on x6 and x7
    sub x9, x8, x2     # Subtract - data dependency on x8
    # Replace multiply with shift and add to simulate multiplication by 7
    slli x10, x9, 3    # x9 * 8
    sub x10, x10, x9   # x9 * 8 - x9 = x9 * 7
    add x3, x3, x10    # Accumulate result
    addi x5, x5, 1     # Increment counter
    blt x5, x4, loop1  # Loop
    
    # Reset counter
    li x5, 0

    # Test 2: Logical operations with data dependencies
loop2:
    xori x6, x1, 0xFF  # XOR immediate
    ori x7, x6, 0xF0   # OR immediate - dep on x6
    and x8, x7, x2     # AND - deps on x7 and x2
    slli x9, x8, 2     # Shift left logical - dep on x8
    srli x10, x9, 1    # Shift right logical - dep on x9
    srai x11, x10, 1   # Shift right arithmetic - dep on x10
    xor x12, x11, x1   # XOR - deps on x11 and x1
    add x3, x3, x12    # Accumulate result
    addi x5, x5, 1     # Increment counter
    blt x5, x4, loop2  # Loop

    # Reset counter
    li x5, 0

    # Test 3: Comparison operations
loop3:
    slt x6, x1, x2     # Set if less than
    slti x7, x1, 50    # Set if less than immediate
    sltu x8, x1, x2    # Set if less than unsigned
    sltiu x9, x1, 30   # Set if less than immediate unsigned
    add x10, x6, x7    # Combine results
    add x10, x10, x8   
    add x10, x10, x9
    add x3, x3, x10    # Accumulate result
    addi x5, x5, 1     # Increment counter
    blt x5, x4, loop3  # Loop

    # Store final result to memory
    li x20, 0x2000     # Memory address
    sw x3, 0(x20)      # Store result

    # End of program
    j end

end:
    j end  # Infinite loop to halt
