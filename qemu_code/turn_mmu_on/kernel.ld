OUTPUT_ARCH("riscv")            /* target is RISC-V, so tools pick the right encoding */
ENTRY(_start)                   /* execution begins at _start, the label in boot.s */

MEMORY {
  /* One physical region. QEMU virt puts RAM at 0x80000000. 128M is the size
     we asked QEMU for. rwx = this region may hold code, data, and be written. */
  RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 128M
}

SECTIONS {
  /* .text.boot is listed FIRST so boot code lands at 0x80000000, the reset PC.
     If it were not first, the CPU would start executing the wrong bytes. */
  .text   : { *(.text.boot) *(.text .text.*) } > RAM   /* all executable code */
  .rodata : { *(.rodata .rodata.*) }           > RAM   /* string literals, constants */
  .data   : { *(.data .data.*) }               > RAM   /* initialized globals */

  /* .bss = globals that start at zero. The linker records where it begins and
     ends; boot.s reads these two symbols to zero the region by hand. */
  .bss : {
    __bss_start = .;                /* '.' is the current address counter */
    *(.bss .bss.*) *(COMMON)
    . = ALIGN(8);                   /* round up so the end is 8-byte aligned */
    __bss_end = .;
  } > RAM

  . = ALIGN(16);                    /* align before carving the stack */
  . = . + 0x1000 ;                  /* reserve 4KB of space... */
  __stack_top = .;                  /* ...stack grows downward from this top */

  __heap_start = .;                 /* heap begins right after the stack */
  . = . + 0x100000 ;                /* reserve 1MB for the allocator */
  __heap_end = .;                   /* my_malloc's wall */
}
