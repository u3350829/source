#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <stddef.h>
#include <assert.h>
#include "Internal.h"

DWORD SetTimeDateStamp(PVOID BaseAddress, DWORD SizeOfImage, PIMAGE_NT_HEADERS NtHeaders, BOOL PE32Plus, LPWSTR lpParameter)
{
	DWORD                 TimeDateStamp;
	PIMAGE_DATA_DIRECTORY DataDirectory;
	DWORD                 Offset;

	if (GetDwordNumber(lpParameter, &TimeDateStamp) == FALSE)
	{
		return ERROR_INVALID_PARAMETER;
	}

	if (BaseAddress == NULL)
	{
		return ERROR_INVALID_PARAMETER;
	}

	NtHeaders->FileHeader.TimeDateStamp = TimeDateStamp;
	if (PE32Plus == FALSE)
	{
		DataDirectory = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.DataDirectory;
	}
	else
	{
		DataDirectory = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.DataDirectory;
	}
	if (DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress != 0 && DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size != 0)
	{
		PIMAGE_EXPORT_DIRECTORY ExportDirectory;

		if (GetFileOffsetFromRVA(NtHeaders, DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress, &Offset) == FALSE)
		{
			return ERROR_BAD_EXE_FORMAT;
		}
		ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((LPBYTE)BaseAddress + Offset);
		ExportDirectory->TimeDateStamp = TimeDateStamp;
	}
	if (DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress != 0 && DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size != 0)
	{
		PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor;

		if (GetFileOffsetFromRVA(NtHeaders, DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, &Offset) == FALSE)
		{
			return ERROR_BAD_EXE_FORMAT;
		}
		ImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)BaseAddress + Offset);
		ImportDescriptor->TimeDateStamp = TimeDateStamp;
	}
	if (DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress != 0 && DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size != 0)
	{
		PIMAGE_RESOURCE_DIRECTORY ResourceDirectory;

		if (GetFileOffsetFromRVA(NtHeaders, DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress, &Offset) == FALSE)
		{
			return ERROR_BAD_EXE_FORMAT;
		}
		ResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)((LPBYTE)BaseAddress + Offset);
		ResourceDirectory->TimeDateStamp = TimeDateStamp;
	}
	if (DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress != 0 && DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size != 0)
	{
		PIMAGE_DEBUG_DIRECTORY DebugDirectory;

		if (GetFileOffsetFromRVA(NtHeaders, DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress, &Offset) == FALSE)
		{
			return ERROR_BAD_EXE_FORMAT;
		}
		DebugDirectory = (PIMAGE_DEBUG_DIRECTORY)((LPBYTE)BaseAddress + Offset);
		DebugDirectory->TimeDateStamp = TimeDateStamp;
	}
	if (DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress != 0 && DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size != 0)
	{
		PIMAGE_LOAD_CONFIG_DIRECTORY LoadConfigDirectory;

#if _MSC_VER > 1310
		assert(offsetof(IMAGE_LOAD_CONFIG_DIRECTORY32, TimeDateStamp) == offsetof(IMAGE_LOAD_CONFIG_DIRECTORY64, TimeDateStamp));
#endif

		if (GetFileOffsetFromRVA(NtHeaders, DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress, &Offset) == FALSE)
		{
			return ERROR_BAD_EXE_FORMAT;
		}
		LoadConfigDirectory = (PIMAGE_LOAD_CONFIG_DIRECTORY)((LPBYTE)BaseAddress + Offset);
		LoadConfigDirectory->TimeDateStamp = TimeDateStamp;
	}
	if (DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress != 0 && DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size != 0)
	{
		PIMAGE_BOUND_IMPORT_DESCRIPTOR Current, Next, End;

		if (GetFileOffsetFromRVA(NtHeaders, DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress, &Offset) == FALSE)
		{
			return ERROR_BAD_EXE_FORMAT;
		}
		Current = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)((LPBYTE)BaseAddress + Offset);
		End = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)((LPBYTE)Current + DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size);
		while ((Next = Current + 1) <= End)
		{
			WORD i;

			if (!Current->OffsetModuleName)
				break;
			Current->TimeDateStamp = TimeDateStamp;
			i = Current->NumberOfModuleForwarderRefs;
			Current = Next;
			if (i)
			{
				do
				{
					Next = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)((PIMAGE_BOUND_FORWARDER_REF)Current + 1);
					if (Next > End)
						goto NESTED_BREAK;
					((PIMAGE_BOUND_FORWARDER_REF)Current)->TimeDateStamp = TimeDateStamp;
					Current = Next;
				} while (--i);
			}
		}
	NESTED_BREAK:;
	}

	assert(offsetof(IMAGE_NT_HEADERS32, OptionalHeader) == offsetof(IMAGE_NT_HEADERS64, OptionalHeader));
	assert(offsetof(IMAGE_OPTIONAL_HEADER32, CheckSum) == offsetof(IMAGE_OPTIONAL_HEADER64, CheckSum));

	UpdateCheckSum(BaseAddress, SizeOfImage, &NtHeaders->OptionalHeader.CheckSum);

	return ERROR_SUCCESS;
}

