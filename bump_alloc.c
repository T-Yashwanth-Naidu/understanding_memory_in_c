/*
 * Grab one big region from the kernel once
 * Keep a Pointer to the next free byte and every my_malloc() returns that
 * pointer then it advnaces it past the requested size Bump the pointer forward
 * each time
 */

#include <stdio.h>
#include <unistd.h>

void *my_malloc(size_t size) {

  static void *memory_start = NULL;
  static void *memory_end = NULL;
  static char *current = NULL;
  static void *hold_start = NULL;

  // If this is the first call, bring a big chunk
  if (NULL == current) {
    // Get a big chunk
    if ((memory_start = sbrk(4096)) == (void *)-1) {
      printf("Sbrk failture\n");
      return NULL;
    }

    current = memory_start;
    memory_end = sbrk(0);
  }

  // Round the size to the next multiple of 16
  // Reason: 16 byte alignment is a common requirement for many architectures to
  // avoid performance penalties or crashes due to misaligned access. By
  // rounding up the requested size to the nearest multiple of 16, we ensure
  // that each allocated block starts at a properly aligned address.
  size_t rounded_size = (size + 15) & ~15;

  // Check if the current address + size is less than the end
  // Meaning, if i add current + size, is with still within the end memory
  // block?

  if (current + rounded_size <= (char *)memory_end) {
    // Return current, but round up to 16 before returning
    hold_start = current;
    current += rounded_size;
    printf("Memory Range Starts: %p  - Ends %p.  Current memory allocated from "
           "%p to %p\n",
           memory_start, memory_end, memory_start, current);
    return hold_start;
  }

  // advance the current pointer

  printf("Memory Range Starts: %p  - Ends %p.  Current memory allocated from "
         "%p to %p\n",
         memory_start, memory_end, memory_start, current);

  return NULL;
}

void my_free(void *ptr) {
  // Does nothing for now
}

int main(void) {
  // Call my Malloc
  void *aPtr = my_malloc(10);
  printf("Allocated Memory at: %p\n", aPtr);

  aPtr = my_malloc(10);
  printf("Allocated Memory at: %p\n", aPtr);

  aPtr = my_malloc(10);
  printf("Allocated Memory at: %p\n", aPtr);

  // Does nothing for now
  my_free(aPtr);
}
