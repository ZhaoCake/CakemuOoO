# RISC-V Assembly Test Program: Memory Access Test
# This program tests memory loads and stores with various patterns

.text
.globl _start

_start:
    # Initialize registers
    li x1, 0x1000      # Base memory address
    li x2, 100         # Loop limit
    li x3, 0           # Counter
    li x4, 0           # Result

    # Loop 1: Sequential memory writes
    # Fill memory with sequential values
loop1:
    sw x3, 0(x1)       # Store counter value to memory
    addi x1, x1, 4     # Move to next word
    addi x3, x3, 1     # Increment counter
    blt x3, x2, loop1  # Continue loop

    # Reset counter and address
    li x3, 0
    li x1, 0x1000

    # Loop 2: Read values back, but in reverse order
loop2:
    addi x5, x2, -1    # Calculate x2-1-x3 (reverse index)
    sub x5, x5, x3
    slli x5, x5, 2     # Multiply by 4 to get byte offset
    add x6, x1, x5     # Calculate address
    lw x7, 0(x6)       # Load value from memory
    add x4, x4, x7     # Add to result
    addi x3, x3, 1     # Increment counter
    blt x3, x2, loop2  # Continue loop

    # Reset counter and address
    li x3, 0
    li x1, 0x1000

    # Loop 3: Strided memory access
    # Access every 16 bytes
loop3:
    slli x5, x3, 4     # Multiply counter by 16 for stride
    add x6, x1, x5     # Calculate address
    lw x7, 0(x6)       # Load value
    add x4, x4, x7     # Add to result
    addi x3, x3, 1     # Increment counter
    addi x8, x2, -75   # Limit at 25
    blt x3, x8, loop3  # Continue loop (only 25 iterations)

    # Store final result
    li x1, 0x2000      # Result address
    sw x4, 0(x1)       # Store result

    # End of program
    j end

end:
    j end  # Infinite loop to halt
