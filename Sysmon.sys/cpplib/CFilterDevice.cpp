#include "CFilterDevice.h"
#include "CIrp.h"
#include "CppLib.h"
#include "CAssert.h"

WCHAR* CFilterDevice::sDeviceName = L"\\Device\\XXXXXXX";
WCHAR* CFilterDevice::sLinkName   = L"\\DosDevices\\XXXXXXX";


CFilterDevice::CFilterDevice(void)
:mDeviceObject(NULL),
mDriverObject(NULL),
mIsAttached(FALSE),
mWeCreated(FALSE),
mWeLinked(NULL),
mFileObjectToUse(NULL),
mDispatchFunc(NULL),
mRefPtr(NULL),
mIsRelease(FALSE)
{
}


CFilterDevice::~CFilterDevice(void)
{
}

CErrorStatus 
CFilterDevice::Init(	PDRIVER_OBJECT			Driver,
						PUNICODE_STRING			Registry,
						const UINT32			DeviceExtSize)
{

	CErrorStatus Error;
	UINT32 Index;

	Index = 0;

	Error = Create( Driver,
					DeviceExtSize,
					sDeviceName,
					FILE_DEVICE_UNKNOWN,
					0,					
					FALSE,
					NULL,
					NULL );

	if (Error.IsNtSuccess())
	{

// 		for ( Index = 0; Index < IRP_MJ_MAXIMUM_FUNCTION ; Index++)
// 		{
// 			Driver->MajorFunction[Index] = CAfdFilterDispatch::FilterDispatch;
// 		}
		//Error = AfdFastIo.Init(Driver);

		//if (Error.IsNtSuccess())
		//{
			//Error = CZPMountEngine::Init(this);

			//if ( Error.IsNtSuccess())
			//{
		Error = Link(sLinkName);

		if ( !Error.IsNtSuccess() )
		{
			Delete();

		}
		else
		{
// 			Error = ToAttachDevice( L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Afd" );
// 
// 			if ( !Error.IsNtSuccess())
// 			{
// 				Unlink();
// 				Delete();
// 			}
		}		
			//}			
		//}

		mIsRelease = TRUE;

	}

	return Error;
}

CErrorStatus 
CFilterDevice::Create(
	PDRIVER_OBJECT			DriverObject,
	const UINT32			DeviceExtSize,
	const WCHAR*			DeviceName, 
	DEVICE_TYPE				DeviceType, 
	UINT32				DeviceChars, 
	BOOL					Exclusive, 
	IrpDispatchFunc			DispatchFunc, 
	void					*RefPtr)
{
	CErrorStatus Error;
	CAssert(!IsAttached());
	CAssertAddrValid( DispatchFunc, IrpDispatchFunc);

	RtlInitUnicodeString( 
		&mDeviceName ,
		DeviceName );

	Error = IoCreateDevice(
		DriverObject,
		DeviceExtSize,
		&mDeviceName,
		DeviceType,
		DeviceChars,
		Exclusive,
		&mDeviceObject);	

	if ( Error.IsNtSuccess())
	{
		mDriverObject			= DriverObject;
		mDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
		mDeviceObject->Flags |= DO_DIRECT_IO;
		mDeviceObject->DeviceExtension = this;

		mIsAttached		= TRUE;
		mWeCreated		= TRUE;
		mDispatchFunc	= DispatchFunc;
		mRefPtr			= RefPtr;
	}

	return Error;
}

void 
CFilterDevice::Delete()
{
	CAssert(IsAttached());
	CAssert(WeCreated());
	CAssert(!WeLinked());

	IoDeleteDevice(mDeviceObject);
	
	//mDeviceName.Empty();
	mIsAttached		= FALSE;
	mDeviceObject	= NULL;
	mDispatchFunc	= NULL;
	mRefPtr			= NULL;
}


CErrorStatus 
CFilterDevice::Link(const WCHAR* LinkName)
{
	CErrorStatus Error;
	CAssert(IsAttached());
	CAssert(WeCreated());
	CAssert(!WeLinked());

	RtlInitUnicodeString(	&mDeviceLinkName,
							LinkName );

	//if ( Error.IsNtSuccess() )
	//{
		Error = IoCreateSymbolicLink(
						&mDeviceLinkName, 
						&mDeviceName
						);
	//}

	if ( Error.IsNtSuccess())
	{
		mWeLinked = TRUE;
	}

	return Error;
}

CErrorStatus 
CFilterDevice::Unlink()
{
	CErrorStatus Error;
	CAssert(IsAttached());
	CAssert(WeCreated());
	CAssert(WeLinked());

	Error = IoDeleteSymbolicLink(
		&mDeviceLinkName
		);


	if ( Error.IsNtSuccess() )
	{
		mWeLinked = FALSE;
	}

	return Error;
}

CErrorStatus 
CFilterDevice::ToAttachDevice( WCHAR* DeviceName )
{
	CErrorStatus Error;
    UNICODE_STRING AttachDeviceName;
	PDEVICE_OBJECT	DeviceObject;
	FILE_OBJECT*	FileObject;
	

	RtlInitUnicodeString( &AttachDeviceName , DeviceName);

	Error = ZwLoadDriver(&AttachDeviceName);

	if ( Error.IsNtSuccess() || 
		Error == STATUS_IMAGE_ALREADY_LOADED )
	{
		RtlInitUnicodeString(&AttachDeviceName, L"\\Device\\Afd");
		Error = IoGetDeviceObjectPointer( 
									&AttachDeviceName, 
									FILE_READ_ATTRIBUTES,
									&FileObject, 
									&DeviceObject);

		if ( Error.IsNtSuccess() )
		{
			Error = mAttachDevice.Attach( mDriverObject,AttachDeviceName);

			if ( Error.IsNtSuccess() )
			{
				mIsAttached = TRUE;
			}
		}
	}

	return Error;
}


PDEVICE_OBJECT
CFilterDevice::Device()
{
	return mDeviceObject;
}

CAttachDevice&
CFilterDevice::AttachDevice()
{	
	CAttachDevice AttachDev;
	return AttachDev;
}