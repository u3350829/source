#ifdef _MSC_VER
#include <errno.h>
errno_t _terrno = 0;
#ifndef _M_IX86
__declspec(naked) errno_t * __cdecl _errno()
{
	__asm
	{
		mov     eax, offset _terrno
		ret
	}
}
#else
errno_t * __cdecl _errno()
{
	return &_terrno;
}
#endif
#endif

#ifdef __BORLANDC__
#pragma warn -8070
extern int * __errno();
__declspec(naked) int * __cdecl _errno()
{
	__asm
	{
		jmp     __errno
	}
}
#endif
