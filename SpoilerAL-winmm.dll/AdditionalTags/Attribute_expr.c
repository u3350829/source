#include <windows.h>
#include "intrinsic.h"
#include "bcb6_std_string.h"
#include "bcb6_std_vector.h"
#include "TSSGCtrl.h"
#include "TSSGSubject.h"
#include "TEndWithAttribute.h"

extern HANDLE hHeap;

#define AT_VARIABLE 0x0800

void __stdcall Attribute_expr(TSSGCtrl *SSGCtrl, TSSGSubject *parent, bcb6_std_string *prefix, bcb6_std_string *code)
{
	size_t nCodeLength;

	nCodeLength = code->_M_finish - code->_M_start;
	if (nCodeLength == 0)
		return;
	for (TEndWithAttribute **it = SSGCtrl->attributeSelector.nowAttributeVec->_M_start, **end = SSGCtrl->attributeSelector.nowAttributeVec->_M_finish; it < end; it++)
	{
		bcb6_std_string *lpPrevCode;
		size_t          nPrevCodeLength;

		if ((*it)->type != AT_VARIABLE)
			continue;
		lpPrevCode = &(*it)->code;
		nPrevCodeLength = lpPrevCode->_M_finish - lpPrevCode->_M_start;
		// semicolon(;) is not the lead and tail byte of codepage 932.
		// it can scan from backward.
		if (nPrevCodeLength != 0 || *(code->_M_finish - 1) != ';')
		{
			LPSTR  lpszCode;
			size_t length;

			lpszCode = HeapAlloc(hHeap, 0, nPrevCodeLength + nCodeLength + 2);
			if (lpszCode == NULL)
				break;
			if (nPrevCodeLength != 0)
				__movsb(lpszCode, lpPrevCode->_M_start, nPrevCodeLength);
			__movsb(lpszCode + nPrevCodeLength, code->_M_start, nCodeLength + 1);
			length = nPrevCodeLength + nCodeLength;
			if (!length || *(lpszCode + length - 1) != ';')
				*(LPWORD)(lpszCode + length++) = BSWAP16(';\0');
			TEndWithAttribute_Setting(*it, lpszCode, length);
			HeapFree(hHeap, 0, lpszCode);
		}
		else
		{
			TEndWithAttribute_Setting(*it, code->_M_start, code->_M_finish - code->_M_start);
		}
		break;
	}
}
