// Include Things
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {

  long page_size = sysconf(_SC_PAGESIZE);

  // BIG Chunk: 1MiB
  // glibc brings this via mmap  ( anything over ~128KiB )
  char *big_chunk = malloc(1024 * 1024);    // 1MiB
  char *probe_big = big_chunk + 512 * 1024; // Probe that memory

  // SMALL Chunk: 5 Bytes
  // glibc brings this from its brk heap pool
  // The pool page is already touched by glibc bookkeeping
  char *small_chunk = malloc(5);
  char *probe_small = small_chunk;

  int fd = open("/proc/self/pagemap", O_RDONLY);
  if (fd < 0) {
    perror("Run again with sudo");
    return 1;
  }

  // ~~~~~~~~~~~~~ BIG CHUNK THINGS ~~~~~~~~~~~~~~~~~~~~~~~~~~

  uintptr_t virtual_address_big_chunk = (uintptr_t)probe_big;
  uintptr_t page_big = virtual_address_big_chunk / page_size;

  printf("BIG chunk Virtual Address: %p\n", (void *)probe_big);

  // Lets read the page-map entry before we write into that location
  // This gives us an idea if a memory chunck from VM is brought to RAM

  uint64_t big_chunk_before;
  if (pread(fd, &big_chunk_before, sizeof big_chunk_before,
            page_big * sizeof big_chunk_before) != sizeof big_chunk_before) {
    perror("pread");
    return 1;
  }

  printf("BIG CHUNK BEFORE poke: Present = %llu\n",
         (unsigned long long)((big_chunk_before >> 63) & 1));

  // Lets poke 1 byte into that memory now
  *probe_big = 'B';

  // Lets read the page again. This should bring the memory onto RAM.

  uint64_t big_chunk_after;
  if (pread(fd, &big_chunk_after, sizeof big_chunk_after,
            page_big * sizeof big_chunk_after) != sizeof big_chunk_after) {
    perror("pread");
    return 1;
  }

  printf("BIG CHUNK AFTER poke: Present = %llu\n",
         (unsigned long long)((big_chunk_after >> 63) & 1));

  //~~~~~~~~~~~~~~~~~~~~~ SMALL CHUNK ~~~~~~~~~~~~~~~~~~~~~

  uintptr_t virtual_address_small = (uintptr_t)probe_small;
  uintptr_t page_small = virtual_address_small / page_size;

  printf("SMALL Chunk Virtual Address: %p\n", (void *)probe_small);

  uint64_t small_chunk_before;
  if (pread(fd, &small_chunk_before, sizeof small_chunk_before,
            page_small * sizeof small_chunk_before) !=
      sizeof small_chunk_before) {
    perror("pread");
    return 1;
  }

  printf("SMALL Chunk Before Poke: %llu\n",
         (unsigned long long)((small_chunk_before >> 63) & 1));
  *probe_small = 'S';

  uint64_t small_chunk_after;
  if (pread(fd, &small_chunk_after, sizeof small_chunk_after,
            page_small * sizeof small_chunk_after) !=
      sizeof small_chunk_after) {
    perror("pread");
    return 1;
  }

  printf("SMALL Chunk After Poke: %llu\n",
         (unsigned long long)((small_chunk_after >> 63) & 1));

  close(fd);
  free(big_chunk);
  free(small_chunk);
  return 0;
}
