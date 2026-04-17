#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

/*
 * -fstack-check simulation
 * 
 * Simulates the page-striding probe mechanism that GCC injects into
 * function prologues when -fstack-check is enabled. On systems with
 * virtual memory and a guard page below the stack, these probes force
 * the CPU to touch every page on the way down, guaranteeing that a
 * large frame allocation cannot leap over the guard page.
 *
 * Architecturally irrelevant on Cortex-M3 / Zephyr: no MMU, no pages,
 * no guard page. Native equivalent is CONFIG_MPU_STACK_GUARD.
 */

void probe_stack_pages(size_t size) {

    // pages variable calculates number of pages for stack
    size_t pages = size + (PAGE_SIZE-1) / PAGE_SIZE;
    


    // Read the current stack pointer via inline assembly (x86-64 host).
    // C has no way to access sp directly; this is GCC extended asm syntax.
    volatile char *sp;
    __asm__ volatile ("mov %%rsp, %0" : "=r"(sp));


    // for loop, i need probe to touch each page, and set a store with *probe=0
    for (size_t i = 1; i<=pages;i++) {
        volatile char * probe = sp - (PAGE_SIZE*i);
        *probe=0;
        printf("Requestd %zu bytes at %p\n", i,  (void *)probe);
    }
    
}

 void large_enough_frame(size_t requested_bytes) {
    printf("\n--- Allocating %zu bytes ---\n");
    probe_stack_pages(requested_bytes);
    char *buf = alloca(requested_bytes);
    memset(buf,0,requested_bytes);
    printf("Successful allocation\n");
 }


int main(void) {
    // 
    //  the mechanism, e.g. something bigger than the typical stack size]
    function_with_large_frame(1024);
    function_with_large_frame(16384);
    function_with_large_frame(65536);
   return 0;
}

/*
    The mechanism, summarized
    When the CPU executes any memory access (load or store), it checks the page table to see if that address is mapped. That page table is
    a data is a data structyre the OS maintains for every process, saying "address range X is valid, address range Y is not"
        If the address is in a mapped page, the CPU completes the access normally
        If the address is in an unmapped page, the CPU raises a hardware execution (a page fault). Executions tops mid-instruction.



    The setup (before the program does anything)
    When the OS starts your process, it carves out the memory layout:
        It picks a region for the stack (say, 8MB)
        It marks those pages as mapped (readable/writable)
        It marks the page immediately below as unmapped, thats the guard page
        It sets sp to the top of the mapped region
        (All this happens before main runs)

    When it does a memory access, the CPu consults the page table the OS set up. Mapped means the accesss succeeded. Unmapped means a fault, t
    then the OS kills the process. 

    OS places guard page directly below stack
    Probe loop walks downard from sp covering the range alloca will claim
    If the allocation is bigger than the remaining stack space, the probe's walk must cross the guard page, because the guard page is the next 
    thing past the stack
    Store lands on guard page, so the CPU faults, and the process dies

    Where does the choice for stack allocation come from?

    The stack size is chosen, not discovered. It's set by default or configured explicitly, and the OS allocates exactly that much when it creates
    the thread. 

    Three levels
    1. System default:
        Every OS ships with a default thread stack size. 
    2. Per-process/per-thread override
        Linker flags at compile time, shell limits, when spawning a new thread there are functions like pthread_attr_setstacksize() works
    3. Zephyr/embedded 0 I can declare stack size explicitly thread, at compile time:
    K-THREAD_STACK_DFEINE(my_stack, 2048);


    How allocation actually works in Linux
    For the main thread, at process creation:
    1. Kernel reads the configured stack size (default 8 MB or whatever's set)
    2. Kernel reserves a range of virtual address of that size
    3. Kernel marks those pages as mapped (reserved in the page table but not backed by physical RAM until first touched)
    4. Kernel marks one page below the regoin as unmapped (GUARD PAGES)
    5. Kernel sets sp to point at the top of the reserved region
    6. Your _start code eventually calls main, which starts using the stack








*/