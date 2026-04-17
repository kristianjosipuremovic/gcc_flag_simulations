/*
FLAG: -fstack-check
Before allocating a large stack frame, GCC inserts code 
that touches each memory page in the allocation sequentially, 
working downard. This ensures the OS sees the stack growing
page by page and can raise a signal before a jump over a
guard page happens. Without it, a large single allocation can 
leap over the guard page entirely and corrupt adjacent memory silently. 

Memory Pages
A memory page is the smallest unit of memory that the OS
manages. The OS doesn't think about memory one byte at a time, 
that would be impossibly slow. Instead it carves RAM into 
fixed-size chunks called pages, and everything it does happens
at the page granularity. On x86 and ARM the standard page sizze is
4096 bytes. That number is baked into the hardware. 

Your 8GB of RAM contains roughly 8 million pages. The OS keeps a table, 
the page table, that tracks which pages are allocated, which process 
owns them, and what permissions they have. WHen your program accesses
an address, the CPU checks the page table to see if that access ia allowed. 

You never explicitly create pages. You just request memory, either though 
malloc, or by compiler allocating stack frames, and the OS maps physical pages
to back that memory when you first touch it. This is called
demand paging. 

Where the Stack Fits Into This
When your pogram starts, the OS allocates a region of virtual
address space for the stack. It doesn't map all physical pages up front. 
Instead it maps just a few pages at the top of the stack where your
current frames live, and leave sthe rest as virtual addresses that aren't backed
by physical memory yet. 

As your functions call other functions and the stack growns downward, each time
execution touches a new page that isn't mapped yet, the CPU faults, the OS
maps a fresh physical page to that virtual address, and execution continues. 

IMPORTANT
At the very bottom of the stack region the OS places a special unmapped
page called the guard page. it has no physical backing and no permissions. If
the stack ever grows into it, the CPU faults, the OS sees it's the guard pages, and 
instead of mapping a new page it sends your process a stack overflow. 

CONNECT TO RESEARCH
Everything here assumes an OS with virtual memory, page tables, and a guard
page mechanism. The Cortex-M3 has none of that. The flat
256KB address space is always fulled mapped. 

*/

#include <stdio.h> // gives printf
#include <stdlib.h> // gives alloca (one some platforms; on others it's in alloca.h)
#include <stdint.h> // gives you fixed-width integer types like uint8_t
#include <string.h> // gives you memset
#define PAGE_SIZE 4096 // This constant is used to compute hosw many pages a given allocation spans

// what does this do?

// Simulates what -fstack-check injects before large stack allocations
// GCC touches each page sequentially downward before using the frame

/*
void probe_stack_pages(size_t size)
size_t is the unsigned integer type guaranteed to be large enough to hold any memory
size on the platform, correct type for anything involving byte counts or sizes

voltatile char *sp - pointer to char, marked volatile
The volatile keyword tells the compiler: do not optimize this access away, do not 
cache it in a register, do not reorder it. Without volatile the compiler might see 
that you're writing 0 to a location that's never read and simply delete the write 
entirely, which would defeat the point of forcing real memory touches. 

From GNU C Docs 21.2 volatile Variables and Fields
The GNU C compiler often performs optimizations that eliminate the need to write or
read a variable. For instance int foo; foo = 1; foo++; might simply sotre the value 2 
into foo without evert storing 1. If the memory containing foo is shared with another 
program, or examined asynchronously by hardware, this optimization could confuse
communiation. That is where volatile comes in. 

Writing volatile with the type in a varaible or field declaration says that the value may be 
examined or changed for reasons outside the control of the program at any moment (directs
compiler not to do certain optimizations)

*/
void probe_stack_pages(size_t size) {   
    /*
    This is integer ceiling division. Plain size PAGE_SIZE truncates - a 4097-byte allocation
    would compute as 1 page, but it actually spans 2. Adding PAGE_SIZE - 1 before dividing 
    ensures that any remainder bumps the count up. Examples: 
        4096 bytes = (4096 + 4096) / 4096 = 8191 /4096 = 1
        4097 bytes = (4097 + 4095) / 4096 - 8192 / 4096 = 2

        I need to prove once per page, so I need to know how many pages the allocation spans
    
    */
    
    size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    // Get approximate current stack pointer
    volatile char *sp;

    /*
    This reads the curren value of the stack pointer register (rsp on x86-64) directly into sp
    __asm__ voltatile(...) - GCC inline assembly. volatile here means to not reorder or eliminate
    the instruction
    "mov %%rsp, %0" - the assembly instruction. %%rsp is the stack pointer register (double %%
    escapes the % in GCC inline asm syntax). %0 is the placeholder for the first output operand
    L "=r"(sp) 0 output constraint, =r means "write the result into any general purpose register, 
    then store it in C variable sp"

    Why is this all needed?
    C doesn't have a standard way of reading the stack pointer. I need to know where the stack 
    currently is so we can compute downward probe addresses relative to it. The real -fstack-check
    flag has GCC emit probes relative to the stack pointer in the generated assembly, this is our way
    of anchoring to the same reference piint. 
    
    */
    __asm__ volatile("move %%rsp, %0" : "=r"(sp));



    /*
    The following commands are for EACH PAGE IN THE ALLOCATION
    sp - (1 * PAGE_SIZE) - compute a probe address one page below the previous. The stack grows
    downward, so "deeper into the stack" means lower addresses, . Page 0 is at sp, page 1 is 
    2p-4096, page 2 is at sp - 8192
    
    *probe = 0 - this is the probe touch. Writing to the address forces the OS to handle the page 
    fault if that page hasn't been mapped yet. This is exactly what -fstack-check does in real
    compiled code: it emits one store instruction per page before doing any stack allocation, so 
    that if the stack would overflow into the guard page, the fault happens here in a controlled way 
    rather than silently smashing the guard page without triggering a fault. Without these probes
    a single large alloca could skip over the guard page and corrypt arbitrary memory without OS
    intervention
    
    */
    // Touch each page going downward (forces OS to see stack growing)
    for (size_t i = 0; i < pages; i++) {
        volatile char *probe = sp - (i * PAGE_SIZE);
        *probe = 0; // touching page
        printf("Probed page %zu at %p\n", i, (void*)probe);

        
        }
}
/*
probe_stack_pages(requested_size) - runs the probe loop before the allocation. In real -fstack-check
compiled code, the probe stores happen in the function prologue before the stack pointer is moved to make
room for the frame. Probe first, then allocate. 

alloca(requested_size) - allocates on the stack by moving the stack pointer. This is runtime stack allocation
Unlike char buf[N] where N must be a compile-time constant, alloca takes a runtime size. Returns a pointer to
the allocates region. The mrmoy is automatically freed when the function returns (stack unwinds)



*/
void function_with_large_frame(size_t requested_size) {
    printf("\n--- Requesting %zu bytes ---\n");
    probe_stack_pages(requested_size);
    char *buf = alloca(requested_size);
    memset(buf, 0, requested_size);
    printf("Allocation succeeded\n");
}

int main() {
    function_with_large_frame(1024);
    function_with_large_frame(16384);
    function_with_large_frame(65536);
    return 0;
}


/*
OPUS 4.7 PROMPTING

Instead, walk down the new frame in page-sized strides, and touch each
stride along the way. That guarantees the guard page gets hit if the 
frame crosses it.

"Ok, but isn't the point of a guard page to detect that anyway, why do we 
need a flag to touch it? What is guard page's functionality without touching
it with fstack-check?"

The guard page is passive. It doesn't "detect" anything on its own. It's
not watching the stack. It's just a page that's been marked unmapped. It has
no active behavior. 

The only thing that cuases a fault is A CPU INSTRUCTION TRYING TO ACCESS AN 
ADDRESS IN THAT PAGE. No access, no fault. The guard page is a tripwire, but a 
tripwire only works if something walks into it. 

Normal code walks into it on its own...
In typical code, functions have small frames. Each call does

sp -=64 ; make room for locals
str lr, [sp, #60] ; save return address - TOUCHES memory near new sp
str r4, [sp, #60] ; save a register, TOUCHES memory near new sp
...

Every call moves sp down a little and accesses memory near the new sp. So
as the stack grows deeper over many calls, the accesses march downward 
in lockstep with sp. The stack is growing one frame at a time.The guard page
works for free here because the code is already touching every region it claims. 

This is the guard page's functionality without -fstack-check;
catch the normal case where stack grows in smal lincrements and code
naturally touches neach new frame

THE HUGE FRAME SKIPS THE TRIPWIRE
sp -= 8 MB ; one instruction, moves sp by 8MB
str r0, [sp] ; first access is way down here

One instruction moves sp by 8 MB. The next instruction accesses memory near the
new sp. Everything in between, including the guard page, is never touched by any 
instruction.

SO WHAT DOES -fstack-check DO?
It makes the big-frame case behave like a small-frame case. Instead of 
jumping sp by 8 MB and touching only the bottom, the prologue walks down in 
page-sizes strides and touches each stride. 

probe = sp;
probe -= 4KB; *probe; // touches page 1


"Isn't the guard page at the bottom, how can it be skipped over?"

The guard page SITS AT A FIXED ADDRESS, the page immediately below the stack 
region, placed there by the OS when the thread is created. 

"What is sp"

sp is the STACK POINTER, a CPU register that holds the address of the current top
of the stack. Every function call uses it. When a fucntion starts, the prologue
does

sp -= frame_size to carve out space for its locals. When the function returns, 
the epilogue does sp += frame_size to release it. 

"Okay, what do you mean "locals" in this context"
Locals = local variabels, the variables declared inside a function. These
live on the stack, in the function's frame, the chunk of stack space the prologue
carves out with sp -= frame_size. Once the frame exists, each local lives at a fixed
offset from sp. 

"when yo usay carve out space for the locals thats what sp -=frame_size does"
Exactly right

"I'm not understanding what size_t is, since i dont recognize it from python, 
why not just naming it "size""

size_t is a C-ism. It is JUST A TYPE, like int or char. It is the tpye C uses for 
"a size or count of bytes in memory". 

size_t remaining = frame_size
int x = 5;


"What's a CPU store?"

A store is one of the two fundamental memory operations a CPU can do/
Load - read a value from a memory address into a register
Store - write a value from a register into a memory address

This explains why -fstack-check exists: sp -= 8MB is not a load or a store, 
so the CPU has no reason to access any address, so the guard page never gets 
touched. The injected probes turn the silent register only subtraction into a
sequence of actual stores that the CPU must perform. 
"*/