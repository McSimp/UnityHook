#include <Windows.h>
#include <stdio.h>
#include <detours.h>
#include "mono.h"

tmono_domain_get _mono_domain_get = NULL;
tmono_domain_assembly_open _mono_domain_assembly_open = NULL;
tmono_assembly_get_image _mono_assembly_get_image = NULL;
tmono_runtime_invoke _mono_runtime_invoke = NULL;
tmono_class_get_method_from_name _mono_class_get_method_from_name = NULL;
tmono_class_from_name _mono_class_from_name = NULL;

typedef int (*tPlayerLoop) (int, int, int);
tPlayerLoop pPlayerLoop = reinterpret_cast<tPlayerLoop>(0x14878D0);

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

bool loadedExtern = false;
int calls = 0;

int hkPlayerLoop(int unk1, int unk2, int unk3)
{
	_asm pushad;
	
	if(!loadedExtern && ++calls > 100)
	{
		MonoDomain* domain = _mono_domain_get();
		LogConsole("MonoDomain = 0x%X\n", domain);

		MonoAssembly* assembly = _mono_domain_assembly_open(domain, "InjectedManagedLib.dll");
		MonoImage* image = _mono_assembly_get_image(assembly);
		MonoClass* mclass = _mono_class_from_name(image, "InjectedManagedLib", "InjectedLib");
		MonoMethod* method = _mono_class_get_method_from_name(mclass, "Initialize", 0);
		
		_mono_runtime_invoke(method, NULL, NULL, NULL);
		LogConsole("InjectedLib::Initialize() called in game");

		loadedExtern = true;
	}

	_asm popad;
	return pPlayerLoop(unk1, unk2, unk3);
}

MonoObject* det_mono_runtime_invoke(MonoMethod *method, void *obj, void **params, MonoObject **exc)
{
	_asm pushad;
	LogConsole("mono_runtime_invoke on '%s'\n", method->name);
	if(!strcmp("OnGUI", method->name))
	{
		_asm nop;
	}
	_asm popad;
	return _mono_runtime_invoke(method, obj, params, exc);
}


DWORD WINAPI onAttach(LPVOID lpThreadParameter)
{
	AllocConsole();
	LogConsole("== Loaded ==\n");

	char path[2048];
	GetModuleFileNameA(NULL, path, 2048);
	LogConsole("Parent executable = %s\n", path);

	HMODULE hGame = LoadLibrary(path);
	LogConsole("hGame = 0x%X\n", hGame);

	GET_MONO_FUNC(mono_domain_get)
	GET_MONO_FUNC(mono_domain_assembly_open)
	GET_MONO_FUNC(mono_assembly_get_image)
	GET_MONO_FUNC(mono_runtime_invoke)
	GET_MONO_FUNC(mono_class_get_method_from_name)
	GET_MONO_FUNC(mono_class_from_name)

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)pPlayerLoop, hkPlayerLoop);
	bool success = DetourTransactionCommit() == NO_ERROR;
	LogConsole("Detouring PlayerLoop: %d\n", success);

	/*
	LogConsole("Detouring mono_runtime_invoke...\n");
	DWORD* addrOfAddrOfFunc = (DWORD*)GetProcAddress(hGame, "mono_runtime_invoke");
	DWORD dwProtect;
	if(VirtualProtect(addrOfAddrOfFunc, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &dwProtect) != 0)
	{
		LogConsole("Doing the overwriting...\n");
		(*addrOfAddrOfFunc) = (DWORD)&det_mono_runtime_invoke;
	}
	LogConsole("Finished detouring\n");
	*/

	//_asm int 3;

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