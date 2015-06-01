/* 
 * exectest7.c
 *
 * Simple program to test exec system call.
 * Does not end in null character
 */


#include "syscall.h"

int main(){
    char arr[13];
    arr[0] = '.';
    arr[1] = '.';
    arr[2] = '/';
    arr[3] = 't';
    arr[4] = 'e';
    arr[5] = 's';
    arr[6] = 't';
    arr[7] = '/';
    arr[8] = 'a';
    arr[9] = 'r';
    arr[10] = 'r';
    arr[11] = 'a';
    arr[12] = 'a';
    int result1 = Exec(arr,0,0,0);
    Exit(result1);
}
