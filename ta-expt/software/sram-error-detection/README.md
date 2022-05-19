# Software SRAM error detection
Shize Che

# Summary
This demo provides support for running and testing software SRAM error detection algorithms
on the experiment board (STM32L496).

# Brief walkthrough of the core functions
- init_sram1: initialize first half of SRAM1 (128KB) to random values and replicate the values
to flash from page 128 to page 191 (a flash page is 2KB).
- rand_error_sram1: Simulates the effect of cosmeic radiation on a single byte in SRAM1. It 
takes as input an SRAM1 address, and it has 0.01 probablity of flipping a random bit of the byte
at this SRAM1 address. The function returns true if the byte was flipped, and false 
otherwise.
- induce_error_sram1: Apply rand_error_sram1 to every byte in SRAM1, and stores the affected
addresses (rand_error_sram1 returns true) to flash starting at page 192. The addresses should 
be able to fit in bank1 because the expected number of addresses to be affected is 1031, which
can be stored within 128KB of flash space.
- check_error_sram1: A baseline software SRAM error detection algorithm, it simply goes through
every byte in SRAM1 and compare it with the values stored in flash. O(n) both time and space.
- dma_read/write_page: Demonstration of how to use DMA to transfer data between SRAM and flash.
DMA memory to memory transfer offers two advantages: fast and independent of CPU. The current
demo doesn't use DMA, but in case the project needs faster data transfer between SRAM and flash,
these functions will be useful.

# Use
Replace induce_error_sram1 in main with your detection function and compare the number of errors
induced and the number of errors found, as well as the runtime in seconds.

# Problems
- Not all 256KB of SRAM1 can be used, I tried it and it halts execution for some reason
- Flash bank2 is not working 
- Seems SRAM can only be written in 4 bytes (not sure), dodgy operations on SRAM1 can halt execution