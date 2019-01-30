// Sysmon.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Sysmon.h"
#include <ShellAPI.h>
#include "CSysmonDriverOpt.h"
#include "CSsymonEtw.h"
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LOADSTRING 100


extern "C" typedef BOOL (WINAPI *pIsWow64Process)(
	HANDLE hProcess,
	PBOOL  Wow64Process
	);

int APIENTRY _tWinMain(
					HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	int argc = 0;
	char** argv = NULL;
	WCHAR** argvW = (WCHAR**)NULL;
	
	argvW = ::CommandLineToArgvW(::GetCommandLine(), &argc);
// 	argv = (char**)malloc(argc * sizeof(char*));
// 	int cb = 0;
// 	for (int i = 0; i < argc; i++)   
// 	{
// 		cb = WideCharToMultiByte (CP_ACP, 0, argvW[i],  -1, NULL, 0, NULL, NULL);
// 		argv[i] = (char*)malloc(cb * sizeof(WCHAR) + 1);
// 		WideCharToMultiByte (CP_ACP, 0, argvW[i], -1, argv[i], cb, NULL, NULL);
// 	}

	BOOL bIsWow64Process = FALSE;
	HMODULE v4 = GetModuleHandleW(L"kernel32.dll");
	pIsWow64Process v5 = (pIsWow64Process)GetProcAddress(v4, "IsWow64Process");
	if ( v5 )
	{
		v5(GetCurrentProcess(), &bIsWow64Process);
	}

	if (bIsWow64Process)
	{
		return RunSysmonX64();
	}


	if (argc > 1)
	{
		if (lstrcmpi(argvW[1],_T("-i")) == 0 )
		{
			
			CSysmonDriverOpt Opt;

			Opt.InstallDriver(
							_T("Sysmon"),
							_T("Sysmon.sys"),
							_T("370030"));

			
			if(Opt.StartDriver(_T("Sysmon")))
			{
				MessageBox(NULL,_T("Success"),_T("Success"),MB_OK);
			}
		}

		if (lstrcmpi(argvW[1],_T("-n")) == 0 )
		{

			CSsymonEtw::Instance()->StartTrace(1);
		}
	}


	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{

		TranslateMessage(&msg);

		DispatchMessage(&msg);

	}

	return msg.wParam;


 	return TRUE;
}


extern "C" typedef void (WINAPI *pGetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);

BOOLEAN __fastcall SysmonTempFileName(wchar_t * FileName, wchar_t* TempName)
{
	wchar_t *v2;
	int i;
	unsigned int v5;
	wchar_t Dsta[MAX_PATH] = {0};
	WCHAR Buffer[MAX_PATH] = {0};
	WCHAR RandName[20] = {0};
	v2 = TempName;
	wcscpy_s(Dsta, MAX_PATH, FileName);

	if ( !GetTempPathW(MAX_PATH, Buffer) )
		return 0;

	i = 0;

	while ( TRUE )
	{
		do 
		{
			wcscpy_s(v2, 520, Buffer);
			if ( RandName[0] )
			{
				wcscat_s(v2, 520, RandName);
				wcscat_s(v2, 520, L"\\");
				if ( GetFileAttributesW(v2) != -1 || !CreateDirectoryW(v2, 0) )
					break;
			}

			wcscat_s(v2, 520, Dsta);
			if ( GetFileAttributesW(v2) == -1 )
				return 1;
			if ( RandName[0] )
			{
				wcsrchr(v2, '\\')[1] = 0;
				RemoveDirectoryW(v2);
			}

		} while (FALSE);
		
		rand_s(&v5);
		swprintf_s(RandName, 20, L"SYS%u", v5);
		if ( (unsigned int)++i >= 10 )
			return 0;
	}

	return 1;
}

wchar_t *__fastcall SysmonFormatMessageW(wchar_t *Dest, int len)
{
	int v2;
	wchar_t *v3;
	DWORD v5;
	WCHAR* Buffer;

	Buffer = 0;

	v3 = Dest;

	v5 = FormatMessageW(12544, 0, GetLastError(), 0, (LPWSTR)&Buffer, 0, 0);
	if ( v5 && len >= (signed int)(v5 + 14) )
	{
		Buffer[lstrlenW(Buffer) - 2] = 0;
		_swprintf(v3, L"%s", Buffer, GetLastError());
	}
	else
	{
		*v3 = 0;
	}
	if ( Buffer )
		LocalFree(*(HLOCAL *)Buffer);
	return v3;
}

BOOLEAN __fastcall SysmonExtractResource(LPCWSTR lpName, wchar_t *Filename)
{
	HRSRC hrFile;
	HGLOBAL hgFile;
	DWORD nFileLength;
	const void *ptr;
	FILE *hFile;

	hrFile = FindResourceW(0, lpName, L"BINRES");

	if ( !hrFile )
		return 0;
	hgFile = LoadResource(0, hrFile);
	nFileLength = SizeofResource(0, hrFile);
	ptr = LockResource(hgFile);
	hFile = _wfopen(Filename, L"wb");

	if ( !hFile )
		return 0;

	fwrite(ptr, 1, nFileLength, hFile);
	fclose(hFile);

	return 1;
}

DWORD RunSysmonX64()
{
	HMODULE v0;
	pGetNativeSystemInfo v1;
	DWORD result;
	wchar_t *v4;

	STARTUPINFOW StartupInfo = {0};
	SYSTEM_INFO SystemInfo = {0};
	PROCESS_INFORMATION ProcessInformation = {0};
	DWORD ExitCode;
	wchar_t Dest[256] = {0};
	WCHAR Filename[520] = {0};

	ExitCode = 0;
	StartupInfo.cb = 68;
	StartupInfo.lpReserved = 0;
	memset(&StartupInfo.lpDesktop, 0, 0x3Cu);

	__m128i Xmm0 = {0};
	_mm_storeu_si128((__m128i *)&ProcessInformation, Xmm0);

	v0 = LoadLibraryW(L"kernel32.dll");
	v1 = (pGetNativeSystemInfo)GetProcAddress(v0, "GetNativeSystemInfo");

	if ( v1 )
		v1(&SystemInfo);
	else
		GetSystemInfo(&SystemInfo);

	if ( SystemInfo.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_AMD64 )
	{
		wprintf(L"Unsupported processor type: %d\n", SystemInfo.wProcessorArchitecture);
		return 1630;
	}

	memset(Filename, 0, sizeof(WCHAR)*520);
	if ( !GetModuleFileNameW(0, Filename, 520) )
	{
		ExitCode = GetLastError();
		wprintf(L"Failed to get the module filename:\n%s\n\n", SysmonFormatMessageW(Dest, 256));
		return ExitCode;
	}

	v4 = wcsrchr(Filename, '\\');

	if ( !SysmonTempFileName(v4 + 1,Filename) )
	{
		ExitCode = GetLastError();

		wprintf(L"Failed to create temporaryfile:\n%s\n\n", SysmonFormatMessageW(Dest, 256));
		return ExitCode;
	}

	if ( !SysmonExtractResource((LPCWSTR)1001,Filename) && GetFileAttributesW(Filename) == -1 )
	{
		ExitCode = GetLastError();
		wprintf(L"Failed to extract the 64-bit version:\n%s\n\n", SysmonFormatMessageW(Dest, 256));
		return ExitCode;
	}

	if ( CreateProcessW(
					Filename,
					GetCommandLineW(),
					0,
					0,
					0,
					0,
					0,
					0,
					&StartupInfo,
					&ProcessInformation) )
	{
		WaitForSingleObject(ProcessInformation.hProcess, 0xFFFFFFFF);
		GetExitCodeProcess(ProcessInformation.hProcess, &ExitCode);
		CloseHandle(ProcessInformation.hProcess);
		CloseHandle(ProcessInformation.hThread);
		result = ExitCode;
		if ( ExitCode == 999 )
			return result;
	}
	else
	{
		ExitCode = GetLastError();
		wprintf(L"Error launching 64-bit version:\n%s\n\n", SysmonFormatMessageW(Dest, 256));
	}

	DeleteFileW(Filename);

	return ExitCode;
}