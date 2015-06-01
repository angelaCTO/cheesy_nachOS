/* 
 * exectest2.c
 *
 * Simple program to test exec system call.
 * Bad string address
 */

#include "syscall.h"

int main() {
    int i;
    int status;
    char filename[101];
    for(i = 0; i < 101; i++) {
        filename[i] = 'w';
    }
    status = Exec(filename,0,0,0);
    Exit(status);
}   
