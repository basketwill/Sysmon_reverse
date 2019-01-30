#include "CAttachDevice.h"
#include "CDebug.h"


/**********************************************************************************/
#pragma PAGEDCODE

CAttachDevice::CAttachDevice(void)
:mDeviceObject(NULL),
mLowDeviceObject(NULL)
{
	RtlInitUnicodeString( 
				&mDriverPath,
				L"\\Device\\xxx" );
}

/**********************************************************************************/
#pragma LOCKEDCODE

CAttachDevice::~CAttachDevice(void)
{
}


/**********************************************************************************/
#pragma INITCODE

CErrorStatus
CAttachDevice::Init( PDEVICE_OBJECT AttachDevice,
						PDEVICE_OBJECT OldDevice )
{
	CErrorStatus Error;
	Error = STATUS_SUCCESS;

	mDeviceObject = AttachDevice;
	mLowDeviceObject = OldDevice;

	return Error;
}


/**********************************************************************************/
#pragma LOCKEDCODE

CErrorStatus
CAttachDevice::Attach( 
	PDRIVER_OBJECT Driver,
	UNICODE_STRING& DeviceName)
{
	CErrorStatus Error;
	PDEVICE_OBJECT	AttachDeviceObject;
	PDEVICE_OBJECT  OldDeviceObject;

	Error = IoCreateDevice(
						Driver,
						0,
						NULL,
						FILE_DEVICE_UNKNOWN,
						0,
						TRUE,
						&AttachDeviceObject);

	if ( Error.IsNtSuccess()) 
	{
		//AttachDeviceObject->Flags |= DO_DIRECT_IO;

		Error = IoAttachDevice(
						AttachDeviceObject,
						&DeviceName,
						&OldDeviceObject);

		if (Error.IsNtSuccess()) {

			CDebug::DebugOut(
						"CAfdAttachDevice: IoAttachDevice(%S): 0x%x\n",
						DeviceName.Buffer, 
						Error);
			
			Init(		AttachDeviceObject,
						OldDeviceObject);
		}

		//KdPrint(("[tdi_fw] DriverEntry: %S fileobj: 0x%x\n", devname, *fltobj));
		//return status;
	}
	else
	{
		CDebug::DebugOut(
					"CAfdAttachDevice: IoCreateDevice(%S): 0x%x\n",
					DeviceName.Buffer,
					Error);
	}

	return Error;
}