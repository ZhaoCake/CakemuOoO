# RISC-V Assembly Test Program: Branch Heavy Test
# This program tests various branch behaviors and is designed to stress the branch predictor

.text
.globl _start

_start:
    # Initialize registers
    li x1, 0       # Counter
    li x2, 100     # Loop limit
    li x3, 0       # Result register
    li x4, 1       # Constant 1
    li x5, 7       # Constant for prime calculation
    li x6, 13      # Another constant

    # Loop 1: Simple counting loop with predictable branches
    # This should be very predictable for the branch predictor
loop1:
    addi x1, x1, 1     # Increment counter
    add x3, x3, x1     # Add to result
    blt x1, x2, loop1  # Branch if counter < limit

    # Reset counter
    li x1, 0

    # Loop 2: Alternating branch behavior
    # This should be harder to predict
loop2:
    addi x1, x1, 1         # Increment counter
    andi x7, x1, 1         # Check if counter is odd or even
    beqz x7, even_branch   # Branch if counter is even
    add x3, x3, x5         # Odd case: add x5
    j odd_done
even_branch:
    add x3, x3, x6         # Even case: add x6
odd_done:
    blt x1, x2, loop2      # Continue loop

    # Reset counter
    li x1, 0

    # Loop 3: Nested branches
    # This creates more complex branch patterns
loop3:
    addi x1, x1, 1         # Increment counter
    
    # First condition: x1 mod 3 == 0?
    addi x8, x1, 0
mod3_loop:
    addi x8, x8, -3
    blt x8, x0, mod3_end
    j mod3_loop
mod3_end:
    addi x8, x8, 3
    bnez x8, not_div3
    
    # If divisible by 3
    addi x3, x3, 3
    j div_check_done
    
not_div3:
    # Second condition: x1 mod 5 == 0?
    addi x8, x1, 0
mod5_loop:
    addi x8, x8, -5
    blt x8, x0, mod5_end
    j mod5_loop
mod5_end:
    addi x8, x8, 5
    bnez x8, not_div5
    
    # If divisible by 5
    addi x3, x3, 5
    j div_check_done
    
not_div5:
    # Default case: add 1
    addi x3, x3, 1
    
div_check_done:
    blt x1, x2, loop3      # Continue loop

    # Store final result to memory
    li x9, 0x1000          # Memory address
    sw x3, 0(x9)           # Store result

    # End of program
    j end

end:
    j end  # Infinite loop to halt
