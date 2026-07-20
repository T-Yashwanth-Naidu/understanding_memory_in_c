/*
 * Allocator on a bare-metal heap. No sbrk, no OS.
 * Heap bounds come from the linker symbols __heap_start / __heap_end.
 */

#include <stddef.h>

extern char __heap_start;         /* not real variables. The linker plants these */
extern char __heap_end;           /* symbols; we take their ADDRESS as the bounds */

typedef struct freeList {
  size_t size;                    /* block size. Low bit is stolen as a used flag */
  struct freeList *next;          /* next free block, when this one is on the list */
} freeList;

static freeList *free_list_head = NULL;   /* head of the reuse list, empty at start */

#define ALIGN _Alignof(max_align_t)       /* strictest alignment any type needs */
static inline size_t align_up(size_t size) {
  /* round size up to the next ALIGN boundary so every block stays aligned */
  return (size + ALIGN - 1) & ~(ALIGN - 1);
}

void *my_malloc(size_t size) {
  static char *mem_current = NULL;   /* bump pointer: next never-used byte */
  static char *mem_last = NULL;      /* the wall: end of the heap */
  size_t rounded_size;
  size_t header;

  /* First call only: load the heap bounds from the linker symbols. */
  if (NULL == mem_current) {
    mem_current = &__heap_start;
    mem_last = &__heap_end;
  }

  rounded_size = align_up(size);              /* caller's size, aligned */
  header = align_up(sizeof(freeList));        /* bytes reserved ahead of user data */

  /* Path 1, reuse. Walk the free list, first block big enough wins. */
  freeList *prev = NULL, *curr = free_list_head;
  while (curr != NULL) {
    if ((curr->size & ~1UL) >= rounded_size) {   /* mask off the flag bit to compare */
      if (prev == NULL)
        free_list_head = curr->next;             /* unlink the head */
      else
        prev->next = curr->next;                 /* unlink a middle block */
      curr->size |= 1;                           /* set low bit: now marked used */
      return (char *)curr + header;              /* hand back the byte after the header */
    }
    prev = curr;
    curr = curr->next;
  }

  /* Path 2, bump. No reusable block, carve fresh memory. */
  if (mem_current + header + rounded_size > mem_last) {
    return NULL;                                 /* would cross the wall: out of memory */
  }

  freeList *block = (freeList *)mem_current;      /* place a header at the bump point */
  block->size = rounded_size | 1;                /* record size, low bit = used */
  block->next = NULL;

  void *user_ptr = mem_current + header;          /* user data starts after the header */
  mem_current = mem_current + header + rounded_size;  /* advance the bump pointer */
  return user_ptr;
}

void my_free(void *memory) {
  size_t header = align_up(sizeof(freeList));
  /* step back from the user pointer to reach this block's header */
  freeList *block = (freeList *)((char *)memory - header);

  /* Double-free guard: low bit clear means already freed. Ignore. */
  if ((block->size & 1) == 0) {
    return;
  }
  block->size &= ~1UL;                 /* clear the used bit: mark free */

  block->next = free_list_head;        /* push this block onto the free list */
  free_list_head = block;
}
