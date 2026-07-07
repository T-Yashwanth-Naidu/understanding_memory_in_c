OUTPUT_ARCH("riscv")
ENTRY(_start)

MEMORY {
RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 128M
}

SECTIONS {
    .text : {*(.text.boot) *(.text .text.*) } > RAM
    .rodata : {*(.rodata .rodata.*) } > RAM
    .data : {*(.data .data.*) } > RAM
    .bss : {__bss_start = .; *(.bss .bss.*) *(COMMON) . = ALIGN(8); __bss_end = .; } > RAM


    . = ALIGN(16);
    . = . + 0x1000 ; 
    __stack_top = .;
    __heap_start = .; 
    . = . + 0x100000 ;
    __heap_end = .;
}
