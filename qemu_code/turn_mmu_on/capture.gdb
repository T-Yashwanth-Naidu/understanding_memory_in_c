cat > capture.gdb << 'EOF'
set pagination off
set confirm off
set architecture riscv:rv64
target remote localhost:1234

printf "\n===== PROOF 1: mode drop, M -> S =====\n"
break in_supervisor
continue
info registers priv

printf "\n===== PROOF 2: page table in RAM =====\n"
break enable_paging
continue
x/4gx &root
p/x $satp

printf "\n===== PROOF 3: satp written, paging ON =====\n"
tbreak paging.c:18
continue
p/x $satp

printf "\n===== PROOF 4: unmapped access faults =====\n"
break s_trap_handler
continue
p/x $scause
p/x $stval
p/x $sepc

printf "\n===== END =====\n"
detach
quit
EOF
