#include "CVersion.h"

CVersion::CVersion(void)
	:mMajorVersion(0)
	,mMinorVersion(0)
	,mBuildNumber(0)
{
}


CVersion::~CVersion(void)
{
}

/**********************************************************************************/
#pragma LOCKEDCODE

void CVersion::GetOsVersion()
{
	OSVERSIONINFOW VersionInformation;
	VersionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
	if (NT_SUCCESS(::RtlGetVersion(&VersionInformation)))
	{
		mMajorVersion = VersionInformation.dwMajorVersion;
		mMinorVersion = VersionInformation.dwMinorVersion;
	}
}

/**********************************************************************************/
#pragma LOCKEDCODE

void CVersion::PsGetVersion()
{
	::PsGetVersion(&mMajorVersion,&mMinorVersion, &mBuildNumber, 0);
}


/**********************************************************************************/
#pragma LOCKEDCODE

ULONG CVersion::GetMajorVersion()
{
	return mMajorVersion;
}


/**********************************************************************************/
#pragma LOCKEDCODE

ULONG CVersion::GetMinorVersion()
{
	return mMinorVersion;
}


/**********************************************************************************/
#pragma LOCKEDCODE

ULONG CVersion::GetBuilderNumber()
{
	return mBuildNumber;
}