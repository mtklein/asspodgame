/* gba_cart.ld — GBA ROM linker script */
/* Defines memory layout for a standard GBA cartridge ROM */

MEMORY
{
    ewram  : ORIGIN = 0x02000000, LENGTH = 256K
    iwram  : ORIGIN = 0x03000000, LENGTH = 32K
    rom    : ORIGIN = 0x08000000, LENGTH = 32M
}

__sp_usr  = ORIGIN(iwram) + LENGTH(iwram) - 0x100;
__sp_irq  = ORIGIN(iwram) + LENGTH(iwram);
__irq_flags = 0x03007FF8;

ENTRY(_start)

SECTIONS
{
    /* ROM header and code */
    .text : {
        *(.text.start)    /* crt0 must come first */
        *(.text*)
        *(.rodata*)
        *(.gnu.linkonce.r*)
        . = ALIGN(4);
    } >rom

    /* Read-only data */
    .data : {
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } >ewram AT >rom

    __data_lma = LOADADDR(.data);

    /* BSS (zero-initialized) */
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } >ewram

    /* IWRAM code (fast code section) */
    .iwram : {
        *(.iwram*)
        . = ALIGN(4);
    } >iwram AT >rom
}
