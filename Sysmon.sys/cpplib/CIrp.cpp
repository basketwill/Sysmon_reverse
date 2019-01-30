#include "cpplib.h"
#include "CIrp.h"
#include "CAssert.h"

CIrp::CIrp(void)
:mIrp(NULL)
{

}

CIrp::CIrp(PIRP pIrp)
:mIrp(NULL)
{
	mIrp = pIrp;
}

CIrp::~CIrp()
{
	mIrp = NULL;
}


PIRP		
CIrp::Get() const
{
	return mIrp;
}

NTSTATUS& 	
	CIrp::Status() const 
{
	return mIrp->IoStatus.Status;
}

KPROCESSOR_MODE&		
CIrp::RequestorMode() const
{
	return mIrp->RequestorMode;
}

ULONG_PTR&	
CIrp::Information() const 
{
	return mIrp->IoStatus.Information;
}

PMDL&
CIrp::MdlAddress() const 
{
	return mIrp->MdlAddress;
}

PVOID&		
CIrp::UserBuffer() const 
{
	return mIrp->UserBuffer;
}


PVOID&
CIrp::DriverContext1() const 
{
	return mIrp->Tail.Overlay.DriverContext[1];
}

PVOID&
CIrp::DriverContext2() const 
{
	return mIrp->Tail.Overlay.DriverContext[2];
}

PVOID&
CIrp::DriverContext3() const 
{
	return mIrp->Tail.Overlay.DriverContext[3];
}

PVOID&
CIrp::DriverContext4() const 
{
	return mIrp->Tail.Overlay.DriverContext[4];
}

UINT8&
CIrp::MajorFunction() const 
{
	return StackLocation()->MajorFunction;
}

UINT8&
CIrp::MinorFunction() const 
{
	return StackLocation()->MinorFunction;
}

UINT8&
CIrp::Flags() const 
{
	return StackLocation()->Flags;
}

UINT8&	
CIrp::Control() const 
{
	return StackLocation()->Control;
}

ULONG&
CIrp::ReadLength() const 
{
	return StackLocation()->Parameters.Read.Length;
}

INT64&
CIrp::ReadPos() const 
{
	return StackLocation()->Parameters.Read.ByteOffset.QuadPart;
}

ULONG&
CIrp::WriteLength() const 
{
	return StackLocation()->Parameters.Write.Length;
}

INT64&	
CIrp::WritePos() const 
{
	return StackLocation()->Parameters.Write.ByteOffset.QuadPart;
}

ULONG&
CIrp::IoctlCode() const 
{
	return StackLocation()->Parameters.DeviceIoControl.IoControlCode;
}

void *&
CIrp::IoctlOutputBuffer() const 
{
	return mIrp->UserBuffer;
}

ULONG&
CIrp::IoctlOutputBufferLength() const 
{
	return StackLocation()->Parameters.DeviceIoControl.
		OutputBufferLength;
}

void *&
CIrp::IoctlInputBuffer() const 
{
	return mIrp->AssociatedIrp.SystemBuffer;
}

ULONG&
CIrp::IoctlInputBufferLength() const 
{
	return StackLocation()->Parameters.DeviceIoControl.InputBufferLength;
}

void *&
CIrp::IoctlType3InputBuffer() const 
{
	return StackLocation()->Parameters.DeviceIoControl.Type3InputBuffer;
}

PDEVICE_OBJECT&	CIrp::Device() const  
{
	return StackLocation()->DeviceObject;
}

PFILE_OBJECT&	CIrp::FileObject() const  
{
	return StackLocation()->FileObject;
}

PIO_STACK_LOCATION	
CIrp::StackLocation() const
{
	if (mStackLocation == Current)
		return IoGetCurrentIrpStackLocation(mIrp);
	else
		return IoGetNextIrpStackLocation(mIrp);
}


void	CIrp::UseCurrentStackLocation() 
{
	mStackLocation = Current;
}

void	CIrp::UseNextStackLocation() 
{
	mStackLocation = Next;
}

void 
CIrp::CopyStackLocationToNext() const
{
	IoCopyCurrentIrpStackLocationToNext(mIrp);
}

void 
CIrp::SkipStackLocation() const
{
	IoSkipCurrentIrpStackLocation(mIrp);
}

NTSTATUS 
CIrp::Complete(NTSTATUS status, CCHAR boost)
{
	Status() = status;
	IoCompleteRequest(mIrp, boost);

	return status;
}

NTSTATUS SimpleCompletion(
	DEVICE_OBJECT *device,
	IRP *irp, 
	void* context)
{
	ASSERT(irp);

	UNREFERENCED_PARAMETER(device);

	if(irp->PendingReturned)
	{
		ASSERT(context);
		KeSetEvent((KEVENT*) context, 0, false);
	}

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS 
CIrp::SimpleSend(DEVICE_OBJECT *device)
{
	KEVENT Event;
	NTSTATUS Status;
	KeInitializeEvent(&Event, NotificationEvent, false);
	
	IoSetCompletionRoutine(
						mIrp, 
						SimpleCompletion,
						&Event,
						true, 
						true, 
						true);

	Status = IoCallDriver(device, mIrp);

	if(STATUS_PENDING == Status)
	{
		KeWaitForSingleObject(&Event, Executive, KernelMode, false, 0);	

		Status = mIrp->IoStatus.Status;
	}

	return Status;
}


void 
CIrp::SetCancelRoutine(PDRIVER_CANCEL cancelFunc)
{
	IoSetCancelRoutine(mIrp, cancelFunc);
}

void 
	CIrp::MarkPending()
{
	IoMarkIrpPending(mIrp);
}

void 
CIrp::UnmarkPending()
{
	IoGetCurrentIrpStackLocation(mIrp)->Control &= ~SL_PENDING_RETURNED;
}



CErrorStatus 
CIrp::BuildDeviceIoControlRequest(
	UINT32			IoctlCode, 
	PDEVICE_OBJECT		Device, 
	const void*			InBuf, 
	UINT32			SizeInBuf, 
	void*				OutBuf, 
	UINT32			SizeOutBuf, 
	BOOL				Internal, 
	KEVENT&				Event, 
	IO_STATUS_BLOCK&	IoStatus)
{
	CErrorStatus Error;
	CAssertAddrValid(pDeviceObject, DEVICE_OBJECT);

	mIrp = IoBuildDeviceIoControlRequest(
		IoctlCode,
		Device, 
		const_cast<void *>(InBuf),
		SizeInBuf,
		OutBuf,
		SizeOutBuf,
		Internal, 
		&Event,
		&IoStatus);

	if ( IsNull(mIrp) )
	{
		Error = IoStatus.Status;
	}

	return Error;
}

CErrorStatus 
CIrp::BuildSynchronousFsdRequest(
	UINT32			majorFunc, 
	PDEVICE_OBJECT		pDeviceObject, 
	const void			*buf, 
	UINT32			nBytes, 
	INT64			pos, 
	KEVENT&				event, 
	IO_STATUS_BLOCK&	IoStatus)
{
	CErrorStatus Error;
	CAssertAddrValid(pDeviceObject, DEVICE_OBJECT);

	LARGE_INTEGER	liPos;
	liPos.QuadPart = pos;
	mIrp = IoBuildSynchronousFsdRequest(majorFunc, pDeviceObject, 
		const_cast<void *>(buf), nBytes, &liPos, &event, &IoStatus);

	if (IsNull(mIrp))
	{
		Error = IoStatus.Status;
	}

	return Error;
}
