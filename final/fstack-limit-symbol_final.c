#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern char __stack_limit;

void check_stack_limit(size_t frame_size) {
    volatile char *sp;
    __asm__ volatile("mov %%rsp, %0" : "=r"(sp));
    volatile char *new_sp = sp - frame_size;
    printf("Current sp: %p\n", (void *)sp);
    printf("Requested frame size: %zu\n", (void *)frame_size);
    printf("Projected sp: %p\n", (void *)new_sp);
    printf("Stack Limit Address: %p\n", (void *)&__stack_limit);


    if((uintptr_t)new_sp < (uintptr_t)&__stack_limit) {
        printf("Overflow Detected");
        abort();


    }

    printf("Frame size allowed, proceed...");
    // compute projected new sp
    // print current sp, frame_size, projected sp, and stack limit address
    // compare projected sp against stack limit
    // if overflow: print message and abort()
    // if safe: print message and continue
}

void function_with_frame(size_t requested_size) {

    printf(" -- Allocation %zu bytes --- ");
    check_stack_limit(requested_size);
    char *buf = alloca(requested_size);
    memset(buf, 0, requested_size);
    printf("Successful Allocation!");
    // print what you're allocating
    // call check_stack_limit
    // alloca the space
    // memset it to zero
    // print success
}

int main(void) {
    printf("Stack limit symbol at: %p\n", (void *)&__stack_limit);
    function_with_frame(1024);
    function_with_frame(16384);
    function_with_frame(65536);
    function_with_frame(1048576);
    return 0;
}