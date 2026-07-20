#include <stdint.h>

/* The Sv39 root page table: 512 entries x 8 bytes = 4096 bytes = one page.
   aligned(4096) is required: satp locates the table by frame number, so it
   must start on a 4KB boundary. Zero-initialized, so every unset slot is
   invalid (V=0) for free. */
static uint64_t root[512] __attribute__((aligned(4096)));

void build_page_table(void)
{
  /* Two 1GB gigapage leaves, identity mapped (virtual == physical).
     Value rule: (PA >> 2) | flags. Flags 0xCF = V R W X A D. */
  root[0] = 0x000000CFUL;   /* slot 0: VA 0-1GB   -> PA 0x00000000, covers the UART */
  root[2] = 0x200000CFUL;   /* slot 2: VA 2-3GB   -> PA 0x80000000, covers code + RAM */
}

void enable_paging(void)
{
  uint64_t root_ppn = ((uint64_t)&root) >> 12;         /* table's physical frame number */
  uint64_t satp_val = ((uint64_t)8 << 60) | root_ppn;  /* mode 8 = Sv39, plus that PPN */

  asm volatile("csrw satp, %0" :: "r"(satp_val));      /* translation ON from this instr */
  asm volatile("sfence.vma");                           /* flush TLB so stale entries die */
}
