# RISC-V Assembly Test Program: Comprehensive Test
# This program combines ALU operations, memory access patterns, and branch prediction
# to fully test the CakemuOoO processor implementation

.text
.globl _start

_start:
    # Initialize registers
    li x1, 0x1000      # Base memory address
    li x2, 50          # Loop limit
    li x3, 0           # Counter
    li x4, 0           # Result accumulator
    li x5, 7           # Constant 1
    li x6, 13          # Constant 2
    li x7, 42          # Constant 3

    # Phase 1: Initialize memory with values
    # Store x3 * x3 + 7 at each location
loop_init:
    # Compute x3 * x3 using shifts and adds for small values
    addi x8, x0, 0      # Initialize result
    addi x20, x0, 0     # Initialize counter
    addi x21, x3, 0     # Copy x3 to x21
square_loop:
    beq x20, x3, square_done  # If counter == x3, we're done
    add x8, x8, x3      # Add x3 to result
    addi x20, x20, 1    # Increment counter
    blt x20, x21, square_loop  # Continue if counter < x3
square_done:
    add x8, x8, x5     # Add constant
    sw x8, 0(x1)       # Store to memory
    addi x1, x1, 4     # Increment address
    addi x3, x3, 1     # Increment counter
    blt x3, x2, loop_init  # Continue loop

    # Reset counter and address
    li x3, 0
    li x1, 0x1000

    # Phase 2: Fibonacci sequence calculation with memory dependencies
    # This will test both memory operations and computational performance
    li x8, 1           # Fib(1)
    li x9, 1           # Fib(2)
    sw x8, 0(x1)       # Store Fib(1)
    sw x9, 4(x1)       # Store Fib(2)
    addi x1, x1, 8     # Point to next memory location
    addi x3, x3, 2     # We've done 2 Fibonacci numbers

fib_loop:
    addi x10, x1, -4   # Address of Fib(n-1)
    addi x11, x1, -8   # Address of Fib(n-2)
    lw x12, 0(x10)     # Load Fib(n-1)
    lw x13, 0(x11)     # Load Fib(n-2)
    add x14, x12, x13  # Calculate Fib(n)
    sw x14, 0(x1)      # Store Fib(n)
    add x4, x4, x14    # Add to result accumulator
    addi x1, x1, 4     # Increment address
    addi x3, x3, 1     # Increment counter
    addi x15, x2, -10  # Calculate limit (40)
    blt x3, x15, fib_loop  # Continue loop (40 iterations)

    # Reset counter and address
    li x3, 0
    li x1, 0x1000

    # Phase 3: Complex branch pattern with data dependencies
    # This will test branch prediction capabilities
    
    # Initialize factorial
    li x8, 1           # Factorial result
    li x9, 1           # Current number

fact_loop:
    addi x9, x9, 1     # Increment current number
    
    # Multiply x8 by x9 using repeated addition
    addi x30, x0, 0    # Initialize temp result
    addi x31, x0, 0    # Initialize counter
mul_loop:
    beq x31, x9, mul_done  # If counter == x9, we're done
    add x30, x30, x8   # Add x8 to result
    addi x31, x31, 1   # Increment counter
    blt x31, x9, mul_loop  # Continue if counter < x9
mul_done:
    addi x8, x30, 0    # Copy result back to x8
    
    # Check various conditions to create complex branch patterns
    
    # Check if number is divisible by 3
    addi x10, x9, 0    # Copy current number
    li x11, 3          # Divisor
div3_loop:
    sub x10, x10, x11  # Subtract divisor
    bltz x10, not_div3 # Branch if negative (not divisible)
    beqz x10, is_div3  # Branch if zero (exactly divisible)
    j div3_loop        # Continue loop
    
is_div3:
    # If divisible by 3, add number to result
    add x4, x4, x9
    j div_check_done
    
not_div3:
    # If not divisible by 3, check if odd or even
    andi x10, x9, 1    # Check lowest bit
    beqz x10, even_num # Branch if even
    
    # Odd number: square it and add to result
    # Compute x9 * x9 using shifts and adds
    addi x10, x0, 0     # Initialize result
    addi x20, x0, 0     # Initialize counter
    addi x21, x9, 0     # Copy x9 to x21
square_loop2:
    beq x20, x9, square_done2  # If counter == x9, we're done
    add x10, x10, x9    # Add x9 to result
    addi x20, x20, 1    # Increment counter
    blt x20, x21, square_loop2  # Continue if counter < x9
square_done2:
    add x4, x4, x10
    j div_check_done
    
even_num:
    # Even number: add twice the number to result
    add x10, x9, x9
    add x4, x4, x10
    
div_check_done:
    # Check for loop termination
    addi x15, x2, -40  # Limit at 10
    blt x9, x15, fact_loop
    
    # Phase 4: Memory access pattern test
    # Reset counter and use a new memory region
    li x3, 0
    li x1, 0x2000
    
    # Fill memory with increasing values
    li x8, 42          # Starting value
mem_fill_loop:
    sw x8, 0(x1)       # Store value
    addi x8, x8, 1     # Increment value
    addi x1, x1, 4     # Next address
    addi x3, x3, 1     # Increment counter
    addi x15, x2, -30  # Limit at 20
    blt x3, x15, mem_fill_loop
    
    # Access memory in complex pattern and compute checksum
    li x3, 0           # Reset counter
    li x1, 0x2000      # Reset base address
    
mem_access_loop:
    # Calculate address = base + (counter * counter * 4) % (size * 4)
    # Compute counter^2 using shifts and adds
    addi x8, x0, 0     # Initialize result
    addi x20, x0, 0    # Initialize counter
    addi x21, x3, 0    # Copy x3 to x21
square_loop3:
    beq x20, x3, square_done3  # If counter == x3, we're done
    add x8, x8, x3     # Add x3 to result
    addi x20, x20, 1   # Increment counter
    blt x20, x21, square_loop3  # Continue if counter < x3
square_done3:
    slli x8, x8, 2     # counter^2 * 4
    li x9, 80          # size * 4 = 20 * 4
    
    # Compute remainder using repeated subtraction
    addi x20, x0, 0    # Result will be in x8
rem_loop:
    blt x8, x9, rem_done  # If x8 < x9, we're done
    sub x8, x8, x9     # Subtract x9 from x8
    j rem_loop         # Continue
rem_done:
    add x10, x1, x8    # base + offset
    
    lw x11, 0(x10)     # Load value
    add x4, x4, x11    # Add to result
    
    addi x3, x3, 1     # Increment counter
    addi x15, x2, -35  # Limit at 15
    blt x3, x15, mem_access_loop
    
    # Store final result to memory
    li x1, 0x3000      # Result address
    sw x4, 0(x1)       # Store final checksum
    
    # End of program
    j end

end:
    j end  # Infinite loop to halt
