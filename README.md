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

All flags sourced from GCC 15.2 documentation.

### Completed

- [x] `-fstack-check`
- [x] `-fstack-limit-symbol=sym`

### Applicable on ARM Cortex-M3 / Zephyr bare-metal

**Stack protection — boundary enforcement**
- [ ] `-fstack-limit-register=reg`
- [ ] `-fno-stack-limit`

**Stack protection — canary**
- [ ] `-fstack-protector`
- [ ] `-fstack-protector-strong`
- [ ] `-fstack-protector-all`
- [ ] `-fstack-protector-explicit`

**Memory safety**
- [ ] `-D_FORTIFY_SOURCE=1/2/3`
- [ ] `-ftrivial-auto-var-init=zero|pattern`

**Control flow**
- [ ] `-fharden-compares`
- [ ] `-fharden-conditional-branches`
- [ ] `-fharden-control-flow-redundancy`

### Not applicable on ARM Cortex-M3

**Requires x86 Intel CET hardware**
- `-fcf-protection=full|branch|return` — x86 only, requires Intel CET

**Requires virtual memory / OS guard pages**
- `-fstack-clash-protection` — guard page mechanism required

**Requires aarch64**
- `-fsanitize=shadow-call-stack` — aarch64 only

**Requires runtime library / OS support**
- `-fsanitize=address`
- `-fsanitize=undefined`
- `-fsanitize=thread`
- `-fsanitize=leak`

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
