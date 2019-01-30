#ifndef _CVersion_h
#define _CVersion_h
#include <cpplib.h>

class CVersion
{
public:
	CVersion(void);
	~CVersion(void);

	void GetOsVersion();
	void PsGetVersion();

	ULONG GetMajorVersion();
	ULONG GetMinorVersion();
	ULONG GetBuilderNumber();
	
private:
	ULONG mMajorVersion;
	ULONG mMinorVersion;
	ULONG mBuildNumber;
	ULONG mCSDVersion;
};

#endif