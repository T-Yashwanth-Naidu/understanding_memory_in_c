/*
 * Memory allocation using self-designed heap on bare metal.
 * Heap bounds come from linker symbols, not sbrk.
 */

#include <stddef.h>

extern char __heap_start;
extern char __heap_end;

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
  static char *mem_current = NULL;   // next free byte
  static char *mem_last = NULL;      // the wall, end of heap
  size_t rounded_size;
  size_t header;

  // First call: take heap bounds from the linker symbols.
  if (NULL == mem_current) {
    mem_current = &__heap_start;
    mem_last = &__heap_end;
  }

  rounded_size = align_up(size);
  header = align_up(sizeof(freeList));

  // Free list search, first-fit.
  freeList *prev = NULL, *curr = free_list_head;
  while (curr != NULL) {
    if ((curr->size & ~1UL) >= rounded_size) {
      if (prev == NULL)
        free_list_head = curr->next;
      else
        prev->next = curr->next;
      curr->size |= 1;
      return (char *)curr + header;
    }
    prev = curr;
    curr = curr->next;
  }

  // Bump path. Fit check against the wall.
  if (mem_current + header + rounded_size > mem_last) {
    return NULL;
  }

  freeList *block = (freeList *)mem_current;
  block->size = rounded_size | 1;
  block->next = NULL;

  void *user_ptr = mem_current + header;
  mem_current = mem_current + header + rounded_size;
  return user_ptr;
}

void my_free(void *memory) {
  size_t header = align_up(sizeof(freeList));
  freeList *block = (freeList *)((char *)memory - header);

  // Double-free guard: bit 0 clear means already free.
  if ((block->size & 1) == 0) {
    return;
  }
  block->size &= ~1UL;

  block->next = free_list_head;
  free_list_head = block;
}

