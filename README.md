# understanding_memory_in_c

Learning how memory actually works in C - from malloc down to thekernel's page-fault handler. Every concept is demonstrated with runnable code and real traces (strace, ftrace, pagemap), not paraphrased from man pages.  

## Articles

- Part 1: Tracing a byte to its physical RAM address —  <https://www.linkedin.com/posts/t-yashwanth-naidu_embedded-embeddedengineers-embeddedsystems-share-7477222635656081409-jm4N/>  
- Part 2: Lazy allocation — does malloc actually give you memory? —  <https://www.linkedin.com/posts/t-yashwanth-naidu_more-memory-misadventures-ugcPost-7477619575329746944-OXYQ/>  

## Programs

- `the_physical_frame.c` — reads /proc/self/pagemap to find the physical frame backing a virtual address (Part 1)  
- `lazy_allocation.c` — proves memory isn't backed until you touch it,big-chunk vs small-chunk (Part 2)  
- `stage1_break.c`, `stage2.c` — early experiments with the program break  

## Evidence

- `article_evidence/` — raw strace, kernel ftrace, and program output behind the Part 2 article  
