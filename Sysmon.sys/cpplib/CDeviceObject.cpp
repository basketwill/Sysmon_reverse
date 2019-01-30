
#include "CDeviceObject.h"
#include "CAssert.h"
#include "CDebug.h"

CDeviceObject::CDeviceObject(void)
	:mDeviceObject(NULL)
{
}

CDeviceObject::CDeviceObject(PDEVICE_OBJECT Device)
{
	mDeviceObject = Device;
}

CDeviceObject::CDeviceObject(const CDeviceObject& Device)
{
	mDeviceObject = Device.mDeviceObject;
}

CDeviceObject::~CDeviceObject(void)
{
	mDeviceObject = NULL;
}


CDeviceObject::operator const PDEVICE_OBJECT() const
{
	return mDeviceObject;
}

CDeviceObject::operator PDEVICE_OBJECT &()
{
	return mDeviceObject;
}

PDEVICE_OBJECT* CDeviceObject::operator &()
{
	return &mDeviceObject;
}

void CDeviceObject::operator =(PDEVICE_OBJECT Device)
{
	mDeviceObject = Device;
}

bool CDeviceObject::operator !=(PDEVICE_OBJECT DeviceObject)
{
	return (mDeviceObject != DeviceObject);
}

bool CDeviceObject::operator !()
{
	return (mDeviceObject == NULL);
}

ULONG& CDeviceObject::Flags()
{
	return mDeviceObject->Flags;
}

PVOID& CDeviceObject::DeviceExtension()
{
	return mDeviceObject->DeviceExtension;
}

PDRIVER_OBJECT& CDeviceObject::DriverObject()
{
	return mDeviceObject->DriverObject;
}

ULONG& CDeviceObject::DeviceType()
{
	return mDeviceObject->DeviceType;
}

PDEVICE_OBJECT& CDeviceObject::AttachedDevice()
{
	return mDeviceObject->AttachedDevice;
}

PDEVICE_OBJECT& CDeviceObject::NextDevice()
{
	return mDeviceObject->NextDevice;
}

ULONG& CDeviceObject::Characteristics()
{
	return mDeviceObject->Characteristics;
}


/**********************************************************************************/
#pragma LOCKEDCODE

CErrorStatus 
CDeviceObject::Init(	
	PDRIVER_OBJECT					Driver,
	CStackStringW<256>&		DeviceName,
	CStackStringW<256>&		DosDeviceName,
	const UINT32					DeviceExtSize
	)
{
	CErrorStatus Error;
	UINT32 Index;
	CUnicodeString IoDeviceName;
	Index = 0;
	IoDeviceName = &DeviceName;
	Error = Create( 
				Driver,
				DeviceExtSize,
				IoDeviceName,
				FILE_DEVICE_UNKNOWN,
				0,					
				FALSE
				);

	if (Error.IsNtSuccess())
	{		
		Error = Link(&DosDeviceName,IoDeviceName);

		if ( !Error.IsNtSuccess() )
		{
			Delete();
		}
		else
		{
			Flags() &= 0xFFFFFF7F;
		}
	}

	return Error;
}

/**********************************************************************************/
#pragma LOCKEDCODE

CErrorStatus 
CDeviceObject::Create(
	PDRIVER_OBJECT			DriverObject,
	const UINT32			DeviceExtSize,
	CUnicodeString&	DeviceName, 
	DEVICE_TYPE				DeviceType, 
	UINT32					DeviceChars, 
	BOOL					Exclusive
	)
{
	CErrorStatus Error;
	CAssert(!IsAttached());
	
	Error = ::IoCreateDevice(
					DriverObject,
					DeviceExtSize,
					(PUNICODE_STRING)DeviceName,
					DeviceType,
					DeviceChars,
					Exclusive,
					&mDeviceObject);	

	if ( Error.IsNtSuccess())
	{
//  		CDebug::DebugOut(
//  			"CDeviceObject::IoCreateDevice(%S): 0x%x\n",
//  			DeviceName.Buffer, 
//  			Error);
		
	}

	return Error;
}

/**********************************************************************************/
#pragma LOCKEDCODE

CErrorStatus 
CDeviceObject::Link(
					const WCHAR* LinkName,
					CUnicodeString& DeviceName) const
{
	CErrorStatus Error;
	CUnicodeString DeviceLinkName;
	CAssert(IsAttached());
	CAssert(WeCreated());
	CAssert(!WeLinked());

	do 
	{
		if (!LinkName)
		{
			Error = STATUS_INVALID_ADDRESS;
			break;
		}
		
		DeviceLinkName = LinkName;
		Error = IoCreateSymbolicLink(
				DeviceLinkName, 
				DeviceName
				);

	} while (FALSE);	

	return Error;
}

/**********************************************************************************/
#pragma PAGEDCODE

void CDeviceObject::Delete()
{
	if (mDeviceObject)
	{
		::IoDeleteDevice(mDeviceObject);
	}
	
}

/**********************************************************************************/
#pragma PAGEDCODE

void CDeviceObject::DeleteSymbolicLink(CStackStringW<256>& SymbolicLink)
{
	CUnicodeString SymbolicLinkName;
	SymbolicLinkName = SymbolicLink;
	::IoDeleteSymbolicLink(SymbolicLinkName);
}