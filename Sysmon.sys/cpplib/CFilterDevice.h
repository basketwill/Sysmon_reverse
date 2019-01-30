#ifndef _CFilterDevice_H
#define _CFilterDevice_H

#include "cpplib.h"
#include "CErrorStatus.h"
#include "CAttachDevice.h"

class CFilterDevice;
class CIrp;

typedef CErrorStatus ( NTAPI *IrpDispatchFunc)(
									CFilterDevice * FilterDevice,
									CIrp&Irp,
									void* RefPtr, 
									BOOL& IsIrpCompleted);

class CFilterDevice
{
public:
	CFilterDevice(void);
	~CFilterDevice(void);
	
public:
	CErrorStatus Create( PDRIVER_OBJECT Driver );

	CErrorStatus 
		Init(	PDRIVER_OBJECT			Driver,
				PUNICODE_STRING			Registry,
				const UINT32			DeviceExtSize
				);

	CErrorStatus Create(
					PDRIVER_OBJECT			DriverObject,
					const UINT32			DeviceExtSize,
					const WCHAR*			DeviceName, 
					DEVICE_TYPE				DeviceType, 
					UINT32				DeviceChars, 
					BOOL					Exclusive, 
					IrpDispatchFunc			DispatchFunc, 
					void					*RefPtr);

	CErrorStatus	Link( const WCHAR* LinkName);
	CErrorStatus	Unlink();
	void			Delete();
 
	PDEVICE_OBJECT
	Device();


	static 
	CAttachDevice&  
	AttachDevice();

	PDRIVER_OBJECT  DriverObject() const {return mDriverObject;};

	operator PDEVICE_OBJECT() {return mDeviceObject;};
	operator const PDEVICE_OBJECT() const {return mDeviceObject;};
	BOOL			IsAttached() const{return mIsAttached;};
	BOOL			WeCreated() const {return mWeCreated;};
	BOOL			WeLinked()	const {return mWeLinked;};

	CErrorStatus ToAttachDevice( WCHAR* DeviceName );

	CErrorStatus
		DispatchIrp( 
		CIrp& Irp,
		BOOL& IsIrpCompleted );

private:
	/*
	* ...设备对象...
	*/
	DEVICE_OBJECT*	mDeviceObject;

	/*
	* ...驱动对象...
	*/
	DRIVER_OBJECT*	mDriverObject;

	/*
	Attach设备
	*/
	CAttachDevice mAttachDevice;

	/*
	* ...设备名称(DeviceName)...
	*/
	UNICODE_STRING	mDeviceName;

	/*
	* ...设备链接名称(LinkName)...
	*/
	UNICODE_STRING	mDeviceLinkName;

	/*
	* ...引用设备...
	*/
	void*			mRefPtr;

	/*
	* ...设备是否被附加Attached...
	*/
	BOOL			mIsAttached;

	/*
	* ...设备是否被创建Created...
	*/
	BOOL			mWeCreated;

	/*
	* ...设备是否被链接Linked...
	*/
	BOOL			mWeLinked;

	/*
	*
	*/
	BOOL			mIsRelease;

	/*
	* ...设备的引用设备...
	*/

	PFILE_OBJECT	mFileObjectToUse;
	IrpDispatchFunc	mDispatchFunc;

	static WCHAR* sDeviceName;
	static WCHAR* sLinkName;

};

#endif