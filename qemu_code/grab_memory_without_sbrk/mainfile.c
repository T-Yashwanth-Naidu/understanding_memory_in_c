#include <stddef.h>

#define UART0 0x10000000

// Allocator lives in level2.c, declare so main.c knows the signatures
void *my_malloc(size_t size);
void my_free(void *memory);

void uart_putc(char c) {
  volatile char *uart = (volatile char *)UART0;
  *uart = c;
}

void main(void) {
  void *memory = my_malloc(10);
  my_free(memory);
  void *again = my_malloc(10);

  // gdb reads these; no UART needed yet
  (void)memory;
  (void)again;

  uart_putc('A');   // liveness signal: reached end of test
  while (1) { }
}
