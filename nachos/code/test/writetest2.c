/*
 * writetest2.c
 * Incorrect number of bytes to write.
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
  Write(buffer, -1, ConsoleOutput);
  return 0;
}
