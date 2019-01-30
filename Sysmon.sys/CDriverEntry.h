#ifndef _CDriverEntry_h
#define _CDriverEntry_h
#include "CVersion.h"
#include <CppLib.h>
#include "CReportRecord.h"
#include "CDeviceObject.h"
#include "CSysmonMiniFltFilter.h"


#ifdef __cplusplus
extern "C"{
#endif
	class CDriverEntry
	{
	public:
		CDriverEntry(void);
		~CDriverEntry(void);

	public:
		static
		NTSTATUS
		NTAPI
		Init(
			IN PDRIVER_OBJECT	DriverObject,
			IN PUNICODE_STRING RegistryPath
			);
		static
		CDriverEntry& Instance();

	public:
		void PsGetVersion();
		BOOL IsSupportFlt();

		CErrorStatus 
		InitIoDeice(	
				PDRIVER_OBJECT					Driver,
				CStackStringW<256>&		DeviceName,
				CStackStringW<256>&		DosDeviceName,
				const UINT32					DeviceExtSize
				);

		void DeleteDosLink(CStackStringW<256>& SymbolicLink);

		void* Extension();

		CErrorStatus InitMiniFlt(PDRIVER_OBJECT DriverObj);
	private:
		CVersion				mSystemVersion;
		BOOL					mIsSupportFlt;
		CReportSysmonRecord		mReportSysmonRecord;

		CDeviceObject			mDeviceObject;
		CSysmonMiniFltFilter	mMiniFltFilter;
	private:
		static CDriverEntry _Instance;
	};


#ifdef __cplusplus
};
#endif

#endif