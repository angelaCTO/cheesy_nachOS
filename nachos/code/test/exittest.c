/* 
 * exittest.c
 *
 * Simple program to test exit system call.
 */

#include "syscall.h"

int
main()
{
  Exit(123);
  Exit(456); //should not be reached
}
