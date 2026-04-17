# gcc_flag_simulations

C simulations of GCC compiler hardening flag mechanisms, written as part of 
CERIAS/Caterpillar-funded embedded binary hardening research at Purdue University.

## Context

This repository accompanies ongoing research characterizing compiler-based binary 
hardening techniques on ARM Cortex-M3 embedded systems running Zephyr RTOS. Rather 
than simply enabling each GCC flag and measuring overhead, each flag's underlying 
mechanism is first reproduced in plain C to build ground-level mechanical understanding 
before benchmarking.

Each simulation includes architectural applicability notes for bare-metal Cortex-M3; 
several flags designed for OS-based virtual memory environments are irrelevant on this 
target, which is itself a research finding.

## Methodology

Each flag has two files:

- `learning/` — annotated study file written during the learning process, includes 
  inline notes, questions asked, concepts explained, and connections to research context
- `final/` — clean implementation written entirely from memory after the learning file 
  is complete, no reference material used

This workflow enforces genuine understanding over copying. The learning files are 
intentionally included as part of the portfolio to document the learning process. 

## Flags

- [x] -fstack-check
- [ ] -fstack-protector
- [ ] -fstack-protector-strong
- [ ] -fstack-protector-all
- [ ] -fstack-protector-explicit
- [ ] -fstack-clash-protection
- [ ] -fstack-limit-symbol
- [ ] -fstack-limit-register
- [ ] -fharden-compares
- [ ] -fharden-conditional-branches
- [ ] -ftrivial-auto-var-init
- [ ] -D_FORTIFY_SOURCE

## Research Context

**Platform:** Zephyr RTOS 4.3.99, ARM Cortex-M3 (qemu_cortex_m3), arm-zephyr-eabi-gcc 14.3.0  
**Institution:** Purdue University CERIAS  
**Industry Partner:** Caterpillar Inc.  
**Related:** gcc_hardening_benchmarks repository (binary size and runtime overhead measurements)

## Note on Background

These simulations were written while simultaneously learning C and OS fundamentals 
from scratch. Concepts like virtual memory, page tables, guard pages, and inline 
assembly are explained in the learning files as they were encountered. This is 
intentional since the goal is document the learning process
