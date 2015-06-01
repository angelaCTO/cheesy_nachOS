/* 
 * exectes6.c
 *
 * Simple program to test exec system call.
 * Loading many programs one after the other - Addrspace releases 
 * memory when a process goes away so it can be used again
 */

#include "syscall.h"

int main() {
    int i;
    int result = 0;
    for (i = 0; i < 5; i++) {
        result = Exec("../test/array", 0,0,0);
    }
    Exit(result);
}   
