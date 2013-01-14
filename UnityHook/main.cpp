#include <Windows.h>
#include <stdio.h>
//#include <mono/mini/jit.h>
//#include <mono/metadata/assembly.h>
//#include <mono/metadata/class-internals.h>
#include <detours.h>
#include "mono.h"

tmono_domain_get _mono_domain_get = NULL;
tmono_domain_assembly_open _mono_domain_assembly_open = NULL;
tmono_assembly_get_image _mono_assembly_get_image = NULL;
tmono_jit_exec _mono_jit_exec = NULL;
tmono_runtime_invoke _mono_runtime_invoke = NULL;

#define GET_MONO_FUNC(name) _ ## name = *((t ## name*)GetProcAddress(hGame, #name)); LogConsole("%s = 0x%X\n", #name, _ ## name); 

void LogConsole( const char *szFmt, ... )
{
	va_list args;
	va_start( args, szFmt );

	int buffSize = _vscprintf( szFmt, args ) + 1;

	if ( buffSize <= 1 )
		return;

	char *szBuff = new char[ buffSize ];
	memset( szBuff, 0, buffSize );

	int len = vsprintf_s( szBuff, buffSize, szFmt, args );

	szBuff[ buffSize - 1 ] = 0;

	HANDLE hOutput = GetStdHandle( STD_OUTPUT_HANDLE );

	DWORD numWritten = 0;
	WriteFile( hOutput, szBuff, len, &numWritten, NULL );

	delete [] szBuff;
}

MonoObject* det_mono_runtime_invoke(MonoMethod *method, void *obj, void **params, MonoObject **exc)
{
	_asm pushad;
	LogConsole("mono_runtime_invoke on '%s'\n", method->name);
	if(!strcmp("Update", method->name))
	{
		//_asm popad;
		//return NULL;
	}
	_asm popad;
	return _mono_runtime_invoke(method, obj, params, exc);
}

DWORD WINAPI onAttach(LPVOID lpThreadParameter)
{
	AllocConsole();
	LogConsole("== Loaded ==\n");
	HMODULE hGame = LoadLibrary("AngryBots.exe");
	LogConsole("hGame = 0x%X\n", hGame);

	GET_MONO_FUNC(mono_domain_get)
	GET_MONO_FUNC(mono_domain_assembly_open)
	GET_MONO_FUNC(mono_assembly_get_image)
	GET_MONO_FUNC(mono_jit_exec)
	GET_MONO_FUNC(mono_runtime_invoke)

	/*
	LogConsole("Overwriting thing...\n");
	DWORD* addrOfAddrOfFunc = (DWORD*)GetProcAddress(hGame, "mono_runtime_invoke");
	DWORD dwProtect;
	if(VirtualProtect(addrOfAddrOfFunc, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &dwProtect) != 0)
	{
		LogConsole("Doing the overwriting...\n");
		(*addrOfAddrOfFunc) = (DWORD)&det_mono_runtime_invoke;
	}
	LogConsole("Finished overwriting\n");
	*/
	//_asm int 3;
	//MonoDomain* domain = (*_mono_domain_get)();
	//LogConsole("domain = 0x%X\n", domain);
	/*
	MonoAssembly* assembly = _mono_domain_assembly_open(domain, "ConsoleApplication1.exe");
	if(!assembly)
	{
		LogConsole("mono_domain_assembly_open failed\n");
	}
	int retval = _mono_jit_exec(domain, assembly, 0, NULL);
	LogConsole("mono_jit_exec result = %d\n", retval);
	*/
	FreeLibrary(hGame);
	return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hModule);
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)onAttach, NULL, 0, NULL);
			return true;
		break;

		case DLL_PROCESS_DETACH:
			FreeConsole();
			return true;
		break;
	}
}