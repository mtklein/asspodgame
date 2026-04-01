@ crt0.s — GBA startup code
@ Sets up stacks, copies data, clears BSS, then jumps to main

    .section .text.start, "ax", %progbits
    .arm
    .global _start
    .align 4

_start:
    @ GBA ROM header (192 bytes)
    b       _entry          @ Branch to actual entry
    @ Nintendo logo (required, 156 bytes — filled by gbafix)
    .space  156, 0x00
    @ Game title (12 bytes)
    .ascii  "AS&S GAME   "
    @ Game code (4 bytes)
    .ascii  "ASSG"
    @ Maker code (2 bytes)
    .ascii  "HB"
    @ Fixed byte
    .byte   0x96
    @ Main unit code
    .byte   0x00
    @ Device type
    .byte   0x00
    @ Reserved (7 bytes)
    .space  7, 0x00
    @ Software version
    .byte   0x01
    @ Header checksum (filled by gbafix)
    .byte   0x00
    @ Reserved (2 bytes)
    .space  2, 0x00

_entry:
    @ Set up IRQ stack
    mov     r0, #0x12       @ IRQ mode
    msr     cpsr_c, r0
    ldr     sp, =__sp_irq

    @ Set up System/User stack
    mov     r0, #0x1F       @ System mode
    msr     cpsr_c, r0
    ldr     sp, =__sp_usr

    @ Copy .data from ROM to EWRAM
    ldr     r0, =__data_lma
    ldr     r1, =__data_start
    ldr     r2, =__data_end
1:  cmp     r1, r2
    ldrlt   r3, [r0], #4
    strlt   r3, [r1], #4
    blt     1b

    @ Clear .bss
    mov     r0, #0
    ldr     r1, =__bss_start
    ldr     r2, =__bss_end
2:  cmp     r1, r2
    strlt   r0, [r1], #4
    blt     2b

    @ Jump to main
    ldr     r0, =main
    bx      r0

    .pool
