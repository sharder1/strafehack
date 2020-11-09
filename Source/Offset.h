#pragma once

class cOffset
{
private:
	DWORD dwOldPageProtection;//For EnablePageWrite & RestorePageProtection
public:

	DWORD HwDll, SwDll, HlMod;
	DWORD HwBase, HwSize, HwEnd;
	DWORD ClBase, ClSize, ClEnd;
	DWORD HlBase, HlSize, HlEnd;
	DWORD VgBase, VgSize, VgEnd;

	BYTE HLType;

	void GetRenderType();
	bool GetModuleInfo();

	void Error(PCHAR Msg);

	DWORD FindClientTable();
	DWORD FindEngineTable();
	DWORD FindStudioTable();

	void CopyClient();
	void CopyEngine();
	void CopyStudio();

	DWORD FindGameConsole();
	void ConsoleColorInitalize();

	DWORD Absolute(DWORD Address);

	DWORD FarProc(DWORD Address, DWORD LB, DWORD HB);
	DWORD FindReference(DWORD start, DWORD end, DWORD Address);

	DWORD FindPattern(PCHAR pattern, PCHAR mask, DWORD start, DWORD end, DWORD offset);
	DWORD FindPattern(PCHAR pattern, DWORD start, DWORD end, DWORD offset);
	
	PVOID FindPlayerMove(void);

	DWORD dwSendPacketPointer, dwSendPacketBackup, dwSpeedPointer;

	bool EnablePageWrite(DWORD addr, DWORD size);

	DWORD CL_Move(void);

	void GlobalTime();
	
	DWORD FindSpeed(void);

	DWORD GetModuleSize(DWORD Address);
};
extern cOffset offset;