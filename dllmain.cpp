#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <cstdint>

#pragma GCC diagnostic ignored "-Wconversion-null"
bool Initialize(HMODULE hModule)
{
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	
	const auto exit_procedure = [&]()
	{
		_fcloseall();
		PostMessage(GetConsoleWindow(), WM_CLOSE, NULL, NULL);
		FreeConsole();
		FreeLibraryAndExitThread(hModule, 0);
	};
	
	static const uint8_t shell[] = {0xC3, 0x90, 0x90};
	DWORD old;
	
	auto terminate = reinterpret_cast<uint64_t>(TerminateProcess);
	VirtualProtect(reinterpret_cast<LPVOID>(terminate), sizeof(shell), PAGE_EXECUTE_READWRITE, &old);
	for (int idx = 0; idx < sizeof(shell); idx++)
		*(uint8_t*)(terminate + idx * 0x1) = shell[idx]; //Write shell
	VirtualProtect(reinterpret_cast<LPVOID>(terminate), sizeof(shell), old, nullptr);
	
	char result[512];
	sprintf(result, "Succesfully patched TerminateProcess(%s)", terminate);
	MessageBox(0, result ,"Success", MB_ICONINFORMATION);
	exit_procedure();
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Initialize, hModule, 0, nullptr));
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}
