#include "CSysmonMiniFltFilter.h"
#include "CSysmonDispatchEngine.h"
#include "CSysmonIoControl.h"


#define FLTPOST(Fn) (PFLT_POST_OPERATION_CALLBACK)CSysmonDispatchEngine::Fn
#define FLTPRE(Fn) (PFLT_PRE_OPERATION_CALLBACK)CSysmonDispatchEngine::Fn

CSysmonMiniFltFilter::CSysmonMiniFltFilter(void)
{
}


CSysmonMiniFltFilter::~CSysmonMiniFltFilter(void)
{
}


/**********************************************************************************/
#pragma INITCODE

CErrorStatus CSysmonMiniFltFilter::Init(PDRIVER_OBJECT DriverObj)
{
	CErrorStatus Status;
	RegisterCallback(
		kFilterUnloadCallback,
		CSysmonMiniFltFilter::FilterUnloadCallback);

	RegisterCallback(
		kInstanceSetupCallbac,
		CSysmonMiniFltFilter::InstanceSetupCallback);

	RegisterCallback(
		kInstanceQueryTeardowCallback,
		CSysmonMiniFltFilter::InstanceQueryTeardownCallback);

	RegisterContext(
				0,
				FLT_STREAMHANDLE_CONTEXT,
				2,
				'csyS');

	RegisterContext(
				1,
				FLT_STREAMHANDLE_CONTEXT,
				sizeof(Sysmon_Flt_Stream_Context),
				'xchf');

	RegisterOptRegistration(
		0,
		IRP_MJ_CREATE,
		0,
		FLTPRE(SysmonFltPreOperation),
		FLTPOST(SysmonFltPostOperation)
		);

	RegisterOptRegistration(
		1,
		IRP_MJ_CLEANUP,
		0,
		FLTPRE(SysmonFltPreOperation),
		FLTPOST(SysmonFltPostOperation)
		);

	RegisterOptRegistration(
		2,
		IRP_MJ_SET_INFORMATION,
		0,
		FLTPRE(SysmonFltPreOperation),
		FLTPOST(SysmonFltPostOperation)
		);

	RegisterOptRegistration(
		3,
		IRP_MJ_CLOSE,
		0,
		FLTPRE(SysmonFltPreOperation),
		FLTPOST(SysmonFltPostOperation)
		);

	RegisterOptRegistration(
		4,
		IRP_MJ_CREATE_NAMED_PIPE,
		0,
		FLTPRE(SysmonFltPreOperation),
		FLTPOST(SysmonFltPostOperation)
		);

	Status = FltRegisterFilter(DriverObj);

	return Status;
}

/**********************************************************************************/
#pragma LOCKEDCODE

NTSTATUS 
CSysmonMiniFltFilter::FilterUnloadCallback(
	FLT_FILTER_UNLOAD_FLAGS Flags
	)
{
	return STATUS_SUCCESS;
}


/**********************************************************************************/
#pragma LOCKEDCODE

NTSTATUS
CSysmonMiniFltFilter::InstanceSetupCallback(
	PCFLT_RELATED_OBJECTS FltObjects,
	FLT_INSTANCE_SETUP_FLAGS Flags,
	DEVICE_TYPE VolumeDeviceType,
	FLT_FILESYSTEM_TYPE VolumeFilesystemType
	)
{

	if ( VolumeDeviceType == FILE_DEVICE_DISK_FILE_SYSTEM)
	{
		return STATUS_SUCCESS;
	}

// 	if (VolumeFilesystemType != FLT_FSTYPE_NPFS)
// 	{
// 	}
}


/**********************************************************************************/
#pragma LOCKEDCODE

NTSTATUS
CSysmonMiniFltFilter::InstanceQueryTeardownCallback(
	PCFLT_RELATED_OBJECTS FltObjects,
	FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
	)
{
	return STATUS_SUCCESS;
}