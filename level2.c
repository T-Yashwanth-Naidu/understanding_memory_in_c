/*
 *
 * First Step: Use sbrk to get a chunk of memory
 * Second Step: Assign memory
 * Third Step: Create Freelist
 * Fourth Step: Free the assigned memory using Freelist
 *
 * - sbrk one page, define the free list empty
 * - search free list first, reuse if block fits, else bump
 * - clear in-use, push block onto the free list
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct freeList {
  size_t size;
  struct freeList *next;
} freeList;

static freeList *free_list_head = NULL;

#define ALIGN _Alignof(max_align_t)
static inline size_t align_up(size_t size) {
  return (size + ALIGN - 1) & ~(ALIGN - 1);
}

void *my_malloc(size_t size) {

  static void *mem_current =
      NULL; // Stores the next free bytes where the next block starts
  static void *mem_last =
      NULL;            // The wall, one past the usable byte in the page
  size_t rounded_size; // Bytes after that are user's
  size_t header;       // Bytes at the front hold size and next

  // If this is the first my_malloc call, get the page size only once.
  if (NULL == mem_current) {

    // Get a block of memory of size page
    // Get the page size
    long page = sysconf(_SC_PAGESIZE);
    if (page <= 0) {
      return NULL;
    }
    printf("Page size: %lu\n", page);

    mem_current = sbrk(page);
    mem_last = sbrk(0);
    printf("Heap region acquired: starts %p, ends %p (%ld bytes, one page)\n",
           mem_current, mem_last, page);
  }

  // Allocate required memory from available memory
  // Fix the size such that the pointer lands on algined boundary
  // Round user size up so the NEXT block also starts 16-aligned
  rounded_size = align_up(size);

  // Align the header
  // Else user_ptr = mem_current + header drifts off the boundary
  header = align_up(sizeof(freeList));

  // Before starting the bump allocator, start searching

  // Walk the free list, first-fit: take the first block big enough.
  // prev trails curr so we can unlink without losing the chain
  freeList *prev = NULL, *curr = free_list_head;
  while (curr != NULL) {
    // Mask off bit 0 ( the in-use flag) to read the tru size before comparing.
    if ((curr->size & ~1) >= rounded_size) {
      // Found a fit. Unlink curr from the list
      if (prev == NULL)
        free_list_head = curr->next; // curr was the head, move head forward
      else
        prev->next = curr->next; // curr was mid-list, bridge over it
      curr->size |= 1; // re-mark in-use, else next my_free double-pushes it
      printf("Reusing freed block at %p (block size %zu, requested %zu). User "
             "pointer: %p\n",
             (void *)curr, curr->size & ~1UL, rounded_size,
             (void *)((char *)curr + header));
      return (char *)curr +
             header; // hand back the user pointer, past the header
    }
    prev = curr;
    curr = curr->next;
  }

  // Start Bump Allocator

  // 1. Does the size fit in the page ?
  if ((char *)mem_current + header + rounded_size > (char *)mem_last) {
    printf("Allocation failed: need %zu bytes (header %zu + block %zu), only "
           "%ld left in page\n",
           header + rounded_size, header, rounded_size,
           (long)((char *)mem_last - (char *)mem_current));
    return NULL;
  }

  // 2. Write the header. Tread the front of the block as the struct and fill it
  freeList *block = (freeList *)mem_current;
  block->size = rounded_size | 1;
  block->next = NULL;

  // 3. Capture the user poitner, it sits after the header
  void *user_ptr = (char *)mem_current + header;

  // 4. Advance the marker past the whole piece, then return.
  mem_current = (char *)mem_current + header + rounded_size;
  printf("Allocated fresh block: header at %p, user pointer at %p, block size "
         "%zu. Next free byte: %p\n",
         (void *)block, user_ptr, rounded_size,
         (void *)((char *)mem_current + header + rounded_size));
  return user_ptr;
}

void my_free(void *memory) {
  // Step back to header from user pointer
  size_t header = align_up(sizeof(freeList));
  freeList *block = (freeList *)((char *)memory - header);

  // Double-free guard: bit 0 clear means already on the list, bail
  if ((block->size & 1) == 0) {
    printf("Ignoring double free of %p: block already on the free list\n",
           memory);
    return;
  } else
    block->size &= ~1; // clear in-use, block is now free
  // Push the block onto the front of the list
  block->next = free_list_head;
  free_list_head = block;
  printf("Freed block at %p (size %zu). Free list head now: %p\n",
         (void *)block, block->size, (void *)free_list_head);
}

int main(void) {

  // Assign 10 bytes
  void *memory = my_malloc(10);
  my_free(memory);             // List should hold one block
  void *again = my_malloc(10); // search should find that block again
  printf("Second allocation: %p %s\n", again,
         again == memory ? "(same address, reuse works)"
                         : "(new address, reuse FAILED)");
  // Free the assigned memory
  my_free(memory);

  return 0;
}
