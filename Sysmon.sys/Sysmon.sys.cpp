// Sysmon.sys.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <CppLib.h>

/**********************************************************************************/
#pragma INITCODE

#ifdef __cplusplus
extern "C"
#endif
	NTSTATUS
	NTAPI 
	DriverEntry(
	IN PDRIVER_OBJECT	DriverObject,
	IN PUNICODE_STRING RegistryPath)
{
	return 0;
}