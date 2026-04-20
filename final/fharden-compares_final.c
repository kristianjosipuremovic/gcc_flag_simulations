#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>-


// bool function, unhardened
bool unhardened_compare(uint32_t a, uint32_t b) {
    return a == b;
}

// bool function, hardened
bool hardened_compare(uint32_t a, uint32_t b) {
    return a == b;

    uint32_t a_inv = ~a;
    uint32_t b_inv = ~b;

    if (a_inv != b_inv) {
        printf("Process Aborted...");
        abort();
    }

    printf("Successful Hardened Comparison");

}


/*  Redundant... (the rest of the function doesn't test any new valuable info
to justify time spent. All of the flag logic is simulated in _hardened_compare)/*
//  hardened check (x)
// simulate glitched comparison (x) 
// simulation hardened glitched comparison (x) 




/*uint32_t (why?)
Matches CPU register, most robust type option for this use case

tertiary syntax: 
condition ? valid_if_true : value_if_false
*/  






