#include "Main.h"

cOffset offset;

void cOffset::GetRenderType()
{
	HwDll = (DWORD)GetModuleHandleA("hw.dll");
	SwDll = (DWORD)GetModuleHandleA("sw.dll");
	HlMod = (DWORD)GetModuleHandleA(NULL);
}

bool cOffset::GetModuleInfo()
{
	GetRenderType();

	if (HwDll)
		HwBase = HwDll;
	else if (SwDll)
		HwBase = SwDll;
	else
		HwBase = HlMod;

	HwSize = GetModuleSize(HwBase);
	HwEnd = HwBase + HwSize - 1;

	HlBase = HlMod;
	HlSize = (DWORD)GetModuleSize(HlBase);
	HlEnd = HlBase + HlSize - 1;

	ClBase = (DWORD)GetModuleHandleA("client.dll");

	if (ClBase)
	{
		ClSize = GetModuleSize(ClBase);
		ClEnd = ClBase + ClSize - 1;
	}
	else
	{
		ClBase = HwBase;
		ClEnd = HwEnd;
		ClSize = HwSize;
	}

	VgBase = (DWORD)GetModuleHandleA("GameUI.dll");

	if (VgBase)
	{
		VgSize = (DWORD)GetModuleSize(VgBase);
		VgEnd = VgBase + VgSize - 1;
	}

	return (HwBase && ClBase && HlBase && VgBase);
}

void cOffset::Error(PCHAR Msg)
{
	MessageBoxA(0, Msg, "Error Find", MB_OK | MB_ICONERROR);
	TerminateProcess(GetCurrentProcess(), 0);
}

DWORD cOffset::FindClientTable()
{
	BYTE ClientOffset[2] = { 0x10, 0x13 };

	DWORD PatternAddress = FindPattern("ScreenFade", HwBase, HwEnd, 0);

	if (PatternAddress)
	{
		for (byte i = 0; i < sizeof(ClientOffset); i++)
		{
			DWORD ClientTablePtr = *(PDWORD)(FindReference(HwBase, HwEnd, PatternAddress) + ClientOffset[i]);

			if (!FarProc((DWORD)ClientTablePtr, HwBase, HwEnd) &&
				!IsBadReadPtr((PVOID)ClientTablePtr, sizeof(cl_clientfunc_t)))
			{
				return ClientTablePtr;
			}
		}
	}

	return 0;
}

DWORD cOffset::FindEngineTable()
{
	DWORD PatternAddress = FindPattern("\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\x89\x86\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\x89\x86\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\x89\x86\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\x89\x86\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\x89\x86\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\x89\x86\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\x89\x86\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF", "xx????x????xx????xx????x????xx????xx????x????xx????xx????x????xx????xx????x????xx????xx????x????xx????xx????x????xx????xx????", ClBase, ClEnd, 0x02);

	if (PatternAddress)
	{
		if (!FarProc((DWORD)PatternAddress, ClBase, ClEnd))
		{
			return *(PDWORD)PatternAddress;
		}
	}
	else
	{
		PatternAddress = FindPattern("\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\x89\x86\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\x89\x86\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\x89\x86\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\x89\x86\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\x89\x86\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\x89\x86\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF\x68\xFF\xFF\xFF\xFF\x89\x86\xFF\xFF\xFF\xFF\xFF\x15\xFF\xFF\xFF\xFF", "xx????x????xx????xx????x????xx????xx????x????xx????xx????x????xx????xx????x????xx????xx????x????xx????xx????x????xx????xx????", HlBase, HlEnd, 0x02);

		if (PatternAddress)
		{
			if (!FarProc((DWORD)PatternAddress, HlBase, HlEnd))
			{
				return *(PDWORD)PatternAddress;
			}
		}
	}

	return 0;
}

DWORD cOffset::FindStudioTable()
{
	DWORD StudioTablePtr = *(DWORD*)((DWORD)g_pClient->HUD_GetStudioModelInterface + 0x30); // old patch, dod

	if (FarProc((DWORD)StudioTablePtr, HwBase, HwEnd) && FarProc((DWORD)StudioTablePtr, HlBase, HlEnd) &&
		FarProc((DWORD)StudioTablePtr, ClBase, ClEnd))
	{
		StudioTablePtr = *(DWORD*)((DWORD)g_pClient->HUD_GetStudioModelInterface + 0x1A); // new patch / steam	

		if (FarProc((DWORD)StudioTablePtr, ClBase, ClEnd))
			return 0;
	}

	return StudioTablePtr;
}

void cOffset::CopyClient()
{
	memcpy(&g_Client, g_pClient, sizeof(cl_clientfunc_t));
}

void cOffset::CopyEngine()
{
	memcpy(&g_Engine, g_pEngine, sizeof(cl_enginefunc_t));
}

void cOffset::CopyStudio()
{
	memcpy(&g_Studio, g_pStudio, sizeof(engine_studio_api_t));
}

DWORD cOffset::FindGameConsole()
{
	DWORD PatternAddress = FindPattern("GameConsole003", VgBase, VgEnd, 0);
	DWORD ReferenAddress = FindReference(VgBase, VgEnd, PatternAddress) + 0x21;

	if (FarProc(ReferenAddress, VgBase, VgEnd))
	{
		Error("Couldn't find GameConsole pointer.");
		return 0;
	}

	DWORD GameConsole = *(PDWORD)ReferenAddress;

	return GameConsole;
}

void cOffset::ConsoleColorInitalize()
{
	DWORD GameConsole = FindGameConsole();

	if (GameConsole)
	{
		DWORD Panel = (*(PDWORD)(GameConsole + 8) - GameConsole);

		Console_TextColor = PColor24(Panel + GameConsole + 288 + sizeof(DWORD));

		if (*(PDWORD)(DWORD(Console_TextColor) + 8) != 0)
		{
			Console_TextColor = PColor24(Panel + GameConsole + 288 + (sizeof(DWORD) * 2));
		}
	}
}

DWORD cOffset::Absolute(DWORD Address)
{
	return Address + *(PDWORD)Address + 4;
}

DWORD cOffset::FarProc(DWORD Address, DWORD LB, DWORD HB)
{
	return ((Address < LB) || (Address > HB));
}

DWORD cOffset::FindReference(DWORD start, DWORD end, DWORD Address)
{
	char szPattern[] = { 0x68 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 };
	*(PDWORD)&szPattern[1] = Address;
	return FindPattern(szPattern, start, end, 0);
}

DWORD cOffset::FindPattern(PCHAR pattern, PCHAR mask, DWORD start, DWORD end, DWORD offset)
{
	int patternLength = strlen(pattern);
	bool found = false;

	for (DWORD i = start; i < end - patternLength; i++)
	{
		found = true;
		for (int idx = 0; idx < patternLength; idx++)
		{
			if (mask[idx] == 'x' && pattern[idx] != *(PCHAR)(i + idx))
			{
				found = false;
				break;
			}
		}
		if (found)
		{
			return i + offset;
		}
	}

	return 0;
}

DWORD cOffset::FindPattern(PCHAR pattern, DWORD start, DWORD end, DWORD offset)
{
	int patternLength = strlen(pattern);
	bool found = false;

	for (DWORD i = start; i < end - patternLength; i++)
	{
		found = true;
		for (int idx = 0; idx < patternLength; idx++)
		{
			if (pattern[idx] != *(PCHAR)(i + idx))
			{
				found = false;
				break;
			}
		}
		if (found)
		{
			return i + offset;
		}
	}

	return 0;
}

PVOID cOffset::FindPlayerMove(void)
{
	DWORD Address = FindPattern("ScreenFade", HwBase, HwEnd, 0);
	PVOID Ptr = (PVOID) * (PDWORD)(FindReference(HwBase, HwEnd, Address) + 0x18);

	if (FarProc((DWORD)Ptr, HwBase, HwEnd))
		Error("PlayerMove: not found.");

	return Ptr;
}

bool cOffset::EnablePageWrite(DWORD addr, DWORD size)
{
	return VirtualProtect((void*)addr, size, PAGE_EXECUTE_READWRITE, &dwOldPageProtection) != 0;
}

DWORD cOffset::CL_Move(void)
{
	DWORD Address = FindPattern("\x56\x57\x33\xFF\x3B\xC7\x0F\x84\x00\x00\x00\x00\x83\xF8\x01\x0F\x84\x00\x00\x00\x00\x83\xF8\x02\x0F\x84\x00\x00\x00\x00\x83\xF8\x03\x75\x22", "xxxxxxx????xxxxx????xxxxx????xxxxx", HwBase, HwEnd, 0);

	if (FarProc((DWORD)Address, HwBase, HwEnd))
		Error("CL_Move: not found.");
	else
	{
		Address = FindPattern("\xC3\x90", "xx", Address - 0x12, HwEnd, 0x2);

		if (FarProc((DWORD)Address, HwBase, HwEnd))
			Error("CL_Move: #2 not found.");
	}

	return Address;
}

void cOffset::GlobalTime()
{
	dwSendPacketPointer = FindPattern("\x75\x13\xD9\x05\x00\x00\x00\x00\xD8\x1D\x00\x00\x00\x00\xDF\xE0\xF6\xC4\x00\x00\x00\xD9\x05\x00\x00\x00\x00\xDC\x1D\x00\x00\x00\x00\xDF\xE0\xF6\xC4\x41", "xxxx????xx????xxxx???xx????xx????xxxxx", HwBase, HwEnd, 0x1b) + 2;

	if (FarProc(dwSendPacketPointer, HwBase, HwEnd))
		Error("dwSendPacket: not found.");

	dwSendPacketBackup = *((uintptr_t*)(dwSendPacketPointer));

	EnablePageWrite(dwSendPacketPointer, sizeof(DWORD));
}

DWORD cOffset::FindSpeed(void)
{
	DWORD Address = FindPattern("Texture load: %6.1fms", HwBase, HwEnd, 0);
	PVOID SpeedPtr = (PVOID) * (DWORD*)(FindReference(HwBase, HwEnd, Address) - 7);

	if (FarProc((DWORD)SpeedPtr, HwBase, HwEnd))
		Error("Speed: not found.");
	else
		EnablePageWrite((DWORD)SpeedPtr, sizeof(double));

	return (DWORD)SpeedPtr;
}

DWORD cOffset::GetModuleSize(DWORD Address)
{
	return PIMAGE_NT_HEADERS(Address + (DWORD)PIMAGE_DOS_HEADER(Address)->e_lfanew)->OptionalHeader.SizeOfImage;
}
