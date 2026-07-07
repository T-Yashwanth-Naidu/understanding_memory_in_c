.section .text.boot
.global _start
_start:
    la sp, __stack_top
    #zero the bss region
    la t0, __bss_start #t0 = start address
    la t1, __bss_end #t1 = end address  
 1: bgeu t0,t1, 2f #if t0>=t1, done, jump forwared to label 2
    sd zero, (t0) #store 8 bytes of zero at address t0
    addi t0, t0, 8 #advance 8 bytes
    j 1b # loop back to label 1
 2: call main
 3: wfi
    j 3b
   
