#include <stddef.h>
#include <stdint.h>

#define UART0 0x10000000          /* QEMU virt 16550 UART, memory-mapped here */

/* Allocator lives in level2.c. Declare the signatures so this file can call them. */
void *my_malloc(size_t size);
void my_free(void *memory);
void build_page_table(void);      /* fills root[] with the two leaf entries */
void enable_paging(void);         /* writes satp, turns translation on */

/* Write one byte to the UART. On bare metal this IS our printf. */
void uart_putc(char c) {
  volatile char *uart = (volatile char *)UART0;   /* volatile: never optimize the store away */
  *uart = c;
}

extern void trap_entry(void);     /* the S-mode trap stub, defined in boot.s */

/* Tell the CPU where to jump on an S-mode trap: load trap_entry into stvec. */
void set_trap_vector(void) {
  asm volatile("csrw stvec, %0" :: "r"((uint64_t)trap_entry));
}

/* Landing site for any S-mode fault. Printing 'F' at all is the proof a
   fault fired and we caught it. Then halt. */
void s_trap_handler(void) {
  uart_putc('F');
  while (1) { }
}

void main(void) {
  build_page_table();             /* step i: entries in memory, still inert */
  set_trap_vector();              /* handler must be armed BEFORE any fault */
  enable_paging();                /* step iii: satp written, translation live */

  /* Prove the allocator still works with paging on. */
  void *memory = my_malloc(10);
  my_free(memory);
  void *again = my_malloc(10);
  (void)memory;
  (void)again;

  /* step iv: touch slot 1, which we left unmapped. This MUST fault. */
  volatile uint64_t *bad = (volatile uint64_t *)0x40000000UL;
  uint64_t x = *bad;              /* triggers a load page fault -> s_trap_handler */
  (void)x;

  uart_putc('B');                 /* unreachable. 'B' would mean the MMU did not enforce. */
  while (1) { }
}
