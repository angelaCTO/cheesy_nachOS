/*
 * writetest.c
 * Writes a string of characters to the console.
 */

#include <syscall.h>

int
main ()
{
  char buffer[10];
  int i = 0;
  while (i<10) {
    buffer[i] = 'a';
    i++;
  }
  Write(buffer, 10, ConsoleOutput);
  return 0;
}
