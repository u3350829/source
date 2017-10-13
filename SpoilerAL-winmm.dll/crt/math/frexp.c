#ifndef _M_IX86
#include <stdint.h>
#include <float.h>
#include <errno.h>

#ifndef DBL_EXP_BIAS
#define DBL_EXP_BIAS (DBL_MAX_EXP - 1)
#endif

#define MSW(x) ((uint32_t *)&(x))[1]

double __cdecl frexp(double x, int *expptr)
{
	if (expptr)
	{
		int32_t exp;

		exp = 0;
		if (*(uint64_t *)&x & 0x7FFFFFFFFFFFFFFF)
		{
			uint32_t msw;

			msw = MSW(x);
			if (!(msw & 0x7FF00000))
			{
				msw &= 0x80000000;
				while (!((*(uint64_t *)&x <<= 1) & 0x0010000000000000))
					exp++;
				msw |= MSW(x);
			}
			exp += ((msw & 0x7FF00000) >> 20) - (DBL_EXP_BIAS - 1);
			MSW(x) = (msw & 0x800FFFFF) | ((DBL_EXP_BIAS - 1) << 20);
		}
		else
		{
			MSW(x) = 0;
		}
		*expptr = exp;
	}
	else
	{
		errno = EINVAL;
		*(uint64_t *)&x = 0;
	}
	return x;
}
#else
#include <errno.h>

errno_t * __cdecl _errno();
extern double _half;

__declspec(naked) double frexp(double x, int *expptr)
{
	__asm
	{
		emms
		fld     qword ptr [esp + 4]         ; Load real from stack
		fxam                                ; Examine st
		fstsw   ax                          ; Get the FPU status word
		mov     ecx, dword ptr [esp + 12]   ; Put exponent address in ecx
		test    ecx, ecx                    ; Test expptr for zero
		jz      L4                          ; Re-direct if zero
		and     ah, 01000101B               ; Isolate  C0, C2 and C3
		cmp     ah, 01000000B               ; Zero ?
		je      L2                          ; Re-direct if x == 0
		test    ah, 00000001B               ; NaN or infinity ?
		jnz     L1                          ; Re-direct if x is NaN or infinity
		fxtract                             ; Get exponent and significand
		fld     qword ptr [_half]           ; Load 0.5
		fmul                                ; Significand * 0.5
		fxch                                ; Swap st, st(1)
		fld1                                ; Load constant 1
		fadd                                ; Increment exponent
		jmp     L3                          ; End of case
	L1:
		fld1                                ; Set exponent to -1
		fchs
		jmp     L3                          ; End of case
	L2:
		fldz                                ; Set exponent to zero
	L3:
		fistp   dword ptr [ecx]             ; Store result exponent and pop
		ret
	L4:
		call    _errno                      ; Get C errno variable pointer
		mov     dword ptr [eax], EINVAL     ; Set invalid argument (EINVAL)
		ret
	}
}
#endif
