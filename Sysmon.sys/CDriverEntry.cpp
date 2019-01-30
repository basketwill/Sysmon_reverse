#include "CDriverEntry.h"
#include <CppLib.h>
#include "CErrorStatus.h"
#include "CUnicodeString.h"
#include "CString.h"
#include "CDriverDispatch.h"
#include "CSysmonDispatchEngine.h"
#include "CDeviceExtension.h"
#include <Fltkernel.h>


CDriverEntry CDriverEntry::_Instance;

/**********************************************************************************/
#pragma LOCKEDCODE

CDriverEntry::CDriverEntry(void)
	:mIsSupportFlt(FALSE)
{
}

/**********************************************************************************/
#pragma LOCKEDCODE

CDriverEntry::~CDriverEntry(void)
{
}

/**********************************************************************************/
#pragma LOCKEDCODE

CDriverEntry& CDriverEntry::Instance()
{
	return _Instance;
}

/**********************************************************************************/
#pragma LOCKEDCODE

void CDriverEntry::PsGetVersion()
{
	mSystemVersion.PsGetVersion();
}

/**********************************************************************************/
#pragma LOCKEDCODE

BOOL CDriverEntry::IsSupportFlt()
{
	CVersion OsVersion;
	OsVersion.GetOsVersion();

	mIsSupportFlt =  ( OsVersion.GetMajorVersion() > 6 || 
		( OsVersion.GetMajorVersion() == 6 && OsVersion.GetMinorVersion() >= 2));

	return mIsSupportFlt;
}

/**********************************************************************************/
#pragma INITCODE

NTSTATUS 
CDriverEntry::Init(
	IN PDRIVER_OBJECT	DriverObject,
	IN PUNICODE_STRING RegistryPath
	)
{
	CErrorStatus Status = STATUS_UNSUCCESSFUL;
    CallGlobalConstructors();

	CStackStringW<256> DeviceName;
	CStackStringW<256> DosDeviceName;
	CDriverDispatch DriverDispatch;
	CUnicodeString DevicePath = RegistryPath;
	BOOL IsSupportFlt = FALSE;
	CDriverEntry::Instance().PsGetVersion();

	DeviceName[0] = '\\';
	DeviceName[1] = 'D';
	DeviceName[2] = 'e';
	DeviceName[3] = 'v';
	DeviceName[4] = 'i';
	DeviceName[5] = 'c';
	DeviceName[6] = 'e';
	DeviceName[7] = '\\';
	DeviceName[8] = '\\';

	DosDeviceName[0] = '\\';
	DosDeviceName[1] = 'D';
	DosDeviceName[2] = 'o';
	DosDeviceName[3] = 's';
	DosDeviceName[4] = 'D';
	DosDeviceName[5] = 'e';
	DosDeviceName[6] = 'v';
	DosDeviceName[7] = 'i';
	DosDeviceName[8] = 'c';
	DosDeviceName[9] = 'e';
	DosDeviceName[10] = 's';
	DosDeviceName[11] = '\\';
	DosDeviceName[12] = '\\';

	PWCHAR pEndPathName = &RegistryPath->Buffer[RegistryPath->Length/2];

	do 
	{
		if (pEndPathName == RegistryPath->Buffer)
		{
			if (pEndPathName[0] != '\\')
			{
				break;
			}
		}
		else
		{
			while (pEndPathName[0] != '\\')
			{
				pEndPathName--;

				if (pEndPathName == RegistryPath->Buffer)
				{
					break;
				}
			}
		}

		++pEndPathName;

	} while (FALSE);

	
	ULONG LengthOfName = RegistryPath->Length - (pEndPathName - RegistryPath->Buffer);

	if (LengthOfName > 255)
	{
		LengthOfName = 255;
	}

	memcpy(&DeviceName[8], pEndPathName, LengthOfName);
	wcsncpy(&DosDeviceName[12], pEndPathName, LengthOfName);

	IsSupportFlt = CDriverEntry::Instance().IsSupportFlt();
	
	DriverDispatch = DriverObject->MajorFunction;

	if (!DriverDispatch)
	{
		DriverDispatch[IRP_MJ_CREATE] = CSysmonDispatchEngine::SysmonDispatchIrp;
		DriverDispatch[IRP_MJ_CLOSE] = CSysmonDispatchEngine::SysmonDispatchIrp;
		DriverDispatch[IRP_MJ_DEVICE_CONTROL] = CSysmonDispatchEngine::SysmonDispatchIrp;


		if (!IsSupportFlt)
		{
			DriverDispatch[IRP_MJ_CREATE] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_CREATE_NAMED_PIPE] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_CLOSE] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_READ] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_WRITE] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_QUERY_INFORMATION] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_SET_INFORMATION] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_QUERY_EA] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_SET_EA] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_FLUSH_BUFFERS] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_QUERY_VOLUME_INFORMATION] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_SET_VOLUME_INFORMATION] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_DIRECTORY_CONTROL] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_FILE_SYSTEM_CONTROL] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_DEVICE_CONTROL] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_INTERNAL_DEVICE_CONTROL] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_SHUTDOWN] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_LOCK_CONTROL] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_CLEANUP] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_CREATE_MAILSLOT] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_QUERY_SECURITY] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_SET_SECURITY] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_POWER] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_SYSTEM_CONTROL] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_DEVICE_CHANGE] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_QUERY_QUOTA] = CSysmonDispatchEngine::SysmonDispatchIrp;
			DriverDispatch[IRP_MJ_SET_QUOTA] = CSysmonDispatchEngine::SysmonDispatchIrp;
		}
		
	}

	do 
	{
		Status = CDriverEntry::Instance().InitIoDeice(
			DriverObject,
			DeviceName,
			DosDeviceName,
			sizeof(CDeviceExtension));

		if (Status.IsNtSuccess())
		{
			__try
			{
				CDeviceExtension* DeviceExtension = (CDeviceExtension*)CDriverEntry::Instance().Extension();

				if (DeviceExtension)
				{
					CSysmonDispatchEngine::InitCsq(*DeviceExtension);
					DeviceExtension->Init();
				}

				Status = CDriverEntry::Instance().InitMiniFlt(DriverObject);

				if (!Status.IsNtSuccess())
				{
					CDriverEntry::Instance().DeleteDosLink(DosDeviceName);
					break;
				}
			}
			__except(EXCEPTION_CONTINUE_EXECUTION)
			{

			}
		}

	} while (FALSE);

	
	return Status;
}


/**********************************************************************************/
#pragma INITCODE

CErrorStatus 
CDriverEntry::InitIoDeice(	
	PDRIVER_OBJECT					Driver,
	CStackStringW<256>&		DeviceName,
	CStackStringW<256>&		DosDeviceName,
	const UINT32					DeviceExtSize
	)
{
	return mDeviceObject.Init(Driver,DeviceName,DosDeviceName,DeviceExtSize);
}


/**********************************************************************************/
#pragma LOCKEDCODE

void* 
CDriverEntry::Extension()
{
	return mDeviceObject.DeviceExtension();
}


/**********************************************************************************/
#pragma INITCODE

CErrorStatus
CDriverEntry::InitMiniFlt(PDRIVER_OBJECT DriverObj)
{
	return mMiniFltFilter.Init(DriverObj);
}


/**********************************************************************************/
#pragma PAGEDCODE

void 
CDriverEntry::DeleteDosLink(CStackStringW<256>& SymbolicLink)
{
	mDeviceObject.DeleteSymbolicLink(SymbolicLink);
}