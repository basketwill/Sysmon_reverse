#ifndef _CAfdAttachDevice_H
#define _CAfdAttachDevice_H

#include "cpplib.h"
#include "CListEntry.h"
#include "CErrorStatus.h"


class CAttachDevice
{
public:
	CAttachDevice(void);
	virtual ~CAttachDevice(void);

	void operator =( PDEVICE_OBJECT DeviceObject)
	{
		mDeviceObject = DeviceObject;
	}


	PDEVICE_OBJECT Device()
	{
		return mDeviceObject;
	}

	PDEVICE_OBJECT LowDevice()
	{
		return mLowDeviceObject;
	}

	CErrorStatus Init( PDEVICE_OBJECT AttachDevice,
						PDEVICE_OBJECT OldDevice );

	CErrorStatus Attach( 
					PDRIVER_OBJECT Driver,
					UNICODE_STRING& DeviceName);
private:
	PDEVICE_OBJECT mDeviceObject;
	PDEVICE_OBJECT mLowDeviceObject;
	UNICODE_STRING mDriverPath;
};

#endif