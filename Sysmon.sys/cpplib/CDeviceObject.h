#ifndef _CDeviceObject_h
#define _CDeviceObject_h
#include "cpplib.h"
#include "CErrorStatus.h"
#include "CString.h"
#include "CUnicodeString.h"

class CDeviceObject
{
public:
	CDeviceObject(void);
	CDeviceObject(PDEVICE_OBJECT Device);
	CDeviceObject(const CDeviceObject& Device);
	~CDeviceObject(void);

public:

	operator const PDEVICE_OBJECT() const;

	operator PDEVICE_OBJECT&();

	PDEVICE_OBJECT* operator &();

	void operator =(PDEVICE_OBJECT Device);

	bool operator !=(PDEVICE_OBJECT DeviceObject);

	bool operator !();

	ULONG& Flags();

	ULONG& Characteristics();

	ULONG& DeviceType();

	PVOID& DeviceExtension();

	PDRIVER_OBJECT& DriverObject();

	PDEVICE_OBJECT& AttachedDevice();

	PDEVICE_OBJECT& NextDevice();

	CErrorStatus ObQueryNameString();

	CErrorStatus 
	Init(	
		PDRIVER_OBJECT					Driver,
		CStackStringW<256>&		DeviceName,
		CStackStringW<256>&		DosDeviceName,
		const UINT32					DeviceExtSize
		);

	CErrorStatus Create(
		PDRIVER_OBJECT			DriverObject,
		const UINT32			DeviceExtSize,
		CUnicodeString&	DeviceName, 
		DEVICE_TYPE				DeviceType, 
		UINT32					DeviceChars, 
		BOOL					Exclusive
		);

	CErrorStatus	Link( 
						const WCHAR* LinkName,
						CUnicodeString& DeviceName) const;
	void Delete();

	void DeleteSymbolicLink(CStackStringW<256>& SymbolicLink);
private:
	/*
	* ...设备对象...
	*/
	PDEVICE_OBJECT	mDeviceObject;
};

#endif