/* 
 * IllegalInstrExceptionHelper.c
 */

#include "syscall.h"

int
main()
{
	void (*functionPtr)(void);
	functionPtr = 0;
	(*functionPtr)(); // should trigger an illegal instruction exception
}
