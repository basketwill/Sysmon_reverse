#ifndef _CIrp_h
#define _CIrp_h

#include "cpplib.h"
#include "CErrorStatus.h"

class CIrp
{
public:
	enum IrpStackLocation {Current, Next};
	CIrp(void);
	CIrp(PIRP pIrp);
	~CIrp();

	operator PIRP (){ return mIrp;};
	operator IRP&() {return *mIrp;};
	operator const IRP&() const {return *mIrp;};
	operator const PIRP() const {return mIrp;};
	operator ULONG_PTR(){ return (ULONG_PTR)mIrp; };
	PIRP		operator->() {return mIrp;};
	const PIRP	operator->() const {return mIrp;};

	PIRP Get() const;

	NTSTATUS& 			Status() const;
	KPROCESSOR_MODE&	RequestorMode() const;
	ULONG_PTR&			Information() const;	
	PMDL&				MdlAddress() const;	
	PVOID&				UserBuffer() const;	
	PVOID&				DriverContext1() const;	
	PVOID&				DriverContext2() const;
	PVOID&				DriverContext3() const;
	PVOID&				DriverContext4() const;
	UINT8&			MajorFunction() const;
	UINT8&			MinorFunction() const;
	UINT8&			Flags() const;
	UINT8&			Control() const;	
	ULONG&			ReadLength() const;
	INT64&			ReadPos() const;
	ULONG&			WriteLength() const;
	INT64&			WritePos() const;	
	ULONG&			IoctlCode() const;	
	void *&			IoctlOutputBuffer() const;	
	ULONG&			IoctlOutputBufferLength() const;	
	void *&			IoctlInputBuffer() const;	
	ULONG&			IoctlInputBufferLength() const;	
	void *&			IoctlType3InputBuffer() const;	
	PDEVICE_OBJECT&		Device() const; 	
	PFILE_OBJECT&		FileObject() const;  	
	PIO_STACK_LOCATION	StackLocation() const;
	void				UseCurrentStackLocation() ;	
	void				UseNextStackLocation();
	void				CopyStackLocationToNext() const;
	void				SkipStackLocation() const;

	NTSTATUS			Complete(	NTSTATUS status, 
									CCHAR boost = IO_NO_INCREMENT);
	void				SetCancelRoutine(PDRIVER_CANCEL cancelFunc);
	void				MarkPending();
	void				UnmarkPending();

	CErrorStatus			BuildDeviceIoControlRequest(
										UINT32			IoctlCode, 
										PDEVICE_OBJECT		Device,
										const void*			InBuf, 
										UINT32			SizeInBuf,
										void *				OutBuf, 
										UINT32			SizeOutBuf, 
										BOOL				Internal,
										KEVENT& event,
										IO_STATUS_BLOCK& IoStatus);

	CErrorStatus			BuildSynchronousFsdRequest(
										UINT32		IoctlCode, 
										PDEVICE_OBJECT	Device,
										const void*		InBuf,
										UINT32 nBytes, 
										INT64 pos, 
										KEVENT& event, 
										IO_STATUS_BLOCK& IoStatus);

	NTSTATUS				SimpleSend(DEVICE_OBJECT *device);
private:
	IRP*				mIrp;
	IrpStackLocation	mStackLocation;
};

#endif
