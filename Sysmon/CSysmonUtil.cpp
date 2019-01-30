#include "StdAfx.h"
#include "CSysmonUtil.h"


CSysmonUtil::CSysmonUtil(void)
{
}


CSysmonUtil::~CSysmonUtil(void)
{
}

BOOL CSysmonUtil::SysmonVersionIsSupport()
{
	BOOL result;
	OSVERSIONINFOEX VersionInformation;

	VersionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	VersionInformation.dwMajorVersion = 0;
	memset(
		&VersionInformation.dwMinorVersion,
		0,
		sizeof(OSVERSIONINFOW));

	GetVersionEx((POSVERSIONINFO)&VersionInformation);

	if ( VersionInformation.dwMajorVersion >= 6 )
		return 1;

	if ( VersionInformation.dwMajorVersion != 5 || 
		!VersionInformation.dwMinorVersion )
	{
		result = 0;
	}

	if ( VersionInformation.dwMinorVersion == 1 )
	{
		//¡·= XP2
		return VersionInformation.wServicePackMajor >= 2;
	}
	if ( VersionInformation.dwMinorVersion == 2 )
	{
		result = (VersionInformation.wProductType == VER_NT_WORKSTATION);
	}
	else
	{
		result = 0;
	}

	return result;
}

unsigned __int64 CSysmonUtil::CounterTimes(FILETIME t1,__int64 t2)
{
	unsigned __int64 Result = 0;

	if ( t2 >= 0 )
	{
		return (unsigned __int64)(10000000 * (t2 % (QWORD)(t1.dwLowDateTime + 16))) / (QWORD)(t1.dwLowDateTime + 16)
		+ 10000000 * (t2 / (QWORD)(t1.dwLowDateTime + 16));
	}
	else
	{
		Result = CounterTimes(t1,-t2);
	}

	return Result;
}