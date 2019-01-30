#ifndef _CSysmonDispatchEngine_h
#define _CSysmonDispatchEngine_h
#include <CppLib.h>
#include "CDeviceExtension.h"
#include <fltKernel.h>

class CSysmonDispatchEngine
{
public:
	CSysmonDispatchEngine(void);
	~CSysmonDispatchEngine(void);

public:
	static	
	NTSTATUS 
	NTAPI 
	SysmonDispatchIrp(
						PDEVICE_OBJECT DeviceObject,
						PIRP Irp);

	static
	NTSTATUS
	NTAPI
	SysmonFltPreOperation (
		 PFLT_CALLBACK_DATA Data,
		 PFLT_RELATED_OBJECTS FltObjects,
		PVOID *CompletionContext
		);

	static
	NTSTATUS
	NTAPI
	SysmonFltPostOperation (
		 PFLT_CALLBACK_DATA Data,
		 PCFLT_RELATED_OBJECTS FltObjects,
		 PVOID CompletionContext,
		 FLT_POST_OPERATION_FLAGS Flags
		);

	static
	void
	NTAPI
	SysmonIoCsqInsertIrp(
		PIO_CSQ Csq,
		PIRP Irp
		);

	static
	void 
	NTAPI
	SysmonIoCsqRemoveIrp(
		PIO_CSQ Csq,
		PIRP Irp
		);

	static
	PIRP 
	NTAPI
	SysmonIoCsqPeekNextIrp(
		PIO_CSQ Csq,
		PIRP Irp,
		PVOID PeekContext
		);

	static
	void 
	NTAPI
	SysmonIoCsqAcquireLock(
		PIO_CSQ Csq,
		PKIRQL Irql
		);

	static	
	void
	NTAPI
	SysmonIoCsqReleaseLock(
		PIO_CSQ Csq,
		KIRQL Irql
		);

	static
	void
	NTAPI
	SysmonIoCsqCompleteCanceledIrp(
		PIO_CSQ Csq,
		PIRP Irp
		);


	static 
	void
	InitCsq(CDeviceExtension& DevExt);
};

#endif