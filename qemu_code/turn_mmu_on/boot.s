.section .text.boot          # boot code. Linker places this first, at 0x80000000.
.global _start               # entry symbol. CPU starts executing here at reset.
_start:
    la sp, __stack_top       # set stack pointer. C cannot run without a stack.

    # Zero the .bss region. C assumes uninitialized globals start at 0.
    # Nothing guarantees that on bare metal, so we clear it ourselves.
    la t0, __bss_start       # t0 = first byte of bss
    la t1, __bss_end         # t1 = one past the last byte
1:  bgeu t0, t1, 2f          # if t0 >= t1, bss is fully zeroed, jump ahead
    sd zero, (t0)            # write 8 zero bytes at t0
    addi t0, t0, 8           # advance one 8-byte word
    j 1b                     # loop

2:  # ---- Drop from M-mode to S-mode ----
    # CPU boots in M-mode (machine, all-powerful). Paging via satp only works
    # in S-mode. So we hand control down to S-mode before using a page table.
    # mret performs the drop, but three registers must be set first.

    # Write 1: PMP. Grant S-mode permission to touch physical memory.
    # M-mode reaches all memory implicitly, S-mode does not. With no PMP rule
    # the first S-mode instruction fetch would fault. Open the full range first.
    li   t0, 0x3fffffffffffff  # cover every physical address
    csrw pmpaddr0, t0
    li   t0, 0x0f              # rule byte: range mode + Read + Write + eXecute
    csrw pmpcfg0, t0

    # Write 2: mstatus.MPP = 01. This field sets which mode mret returns into.
    # 11 = M, 01 = S, 00 = U. Clear both bits, then set 01.
    csrr t0, mstatus
    li   t1, 0x1800           # mask over MPP, bits 12:11
    not  t1, t1               # zeros only in the MPP bits
    and  t0, t0, t1           # clear MPP, keep all other mstatus bits
    li   t1, 0x0800           # value 01 placed at bit 11
    or   t0, t0, t1           # set MPP = S-mode
    csrw mstatus, t0

    # Delegate exceptions to S-mode. By default every trap goes to M-mode,
    # even while running in S-mode. medeleg routes them down so our S-mode
    # handler (stvec) catches page faults.
    li   t0, 0xffff           # delegate exceptions 0..15 to S-mode
    csrw medeleg, t0

    # Write 3: mepc. The address mret jumps to. Point it just below so
    # execution falls straight into S-mode.
    la   t0, in_supervisor
    csrw mepc, t0

    mret                      # the drop: mode <- mstatus.MPP (S), pc <- mepc

in_supervisor:                # first instruction in Supervisor mode
    call main                 # enter C. Everything past here runs in S-mode.

3:  wfi                       # if main returns, idle the core
    j 3b

    # ---- S-mode trap vector ----
    # Where the CPU jumps on an S-mode trap. C loads this label into stvec.
.align 2                      # stvec uses the low 2 bits as a mode flag, so the
                              # handler address must be 4-byte aligned (ends in 00).
.global trap_entry
trap_entry:
    call s_trap_handler       # hand the fault to C
trap_hang:
    wfi                       # handler never returns. Idle forever.
    j trap_hang
