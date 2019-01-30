#include "CMiniFltFilter.h"


#pragma comment(lib,"FltMgr.lib")

/**********************************************************************************/
#pragma INITCODE

CMiniFltFilter::CMiniFltFilter(void)
	:mFilter(NULL)
{
	memset(
			&mFltRegistration,
			0,
			sizeof(FLT_REGISTRATION));

	mFltRegistration.Size = sizeof(FLT_REGISTRATION);
	mFltRegistration.Version = FLT_REGISTRATION_VERSION;
	mFltRegistration.Flags = 0;
	mFltRegistration.ContextRegistration = mContextRegistration;
	mFltRegistration.OperationRegistration = mOptRegistration;

	for ( int nI = 0; nI < MAX_FLT_OPERATION_REGISTRATION;nI++)
	{
		mOptRegistration[nI].MajorFunction = IRP_MJ_OPERATION_END;
		mOptRegistration[nI].Flags = 0;
		mOptRegistration[nI].PostOperation = 0;
		mOptRegistration[nI].PreOperation = 0;
		mOptRegistration[nI].Reserved1 = 0;
	}

	for ( int nI = 0; nI < MAX_FLT_CONTEXT_REGISTRATION; nI++)
	{
		mContextRegistration[nI].ContextType = FLT_CONTEXT_END;
		mContextRegistration[nI].ContextAllocateCallback = 0;
		mContextRegistration[nI].ContextCleanupCallback = 0;
		mContextRegistration[nI].ContextFreeCallback = 0;
		mContextRegistration[nI].Flags = 0;
		mContextRegistration[nI].PoolTag = 0;
		mContextRegistration[nI].Reserved1 = 0;
		mContextRegistration[nI].Size = 0;
	}

	mFilterUnloadCallback = 0;
	mInstanceSetupCallback = 0;
	mInstanceQueryTeardownCallback = 0;
	mInstanceTeardownStartCallback = 0;
	mInstanceTeardownCompleteCallback = 0;
	mGenerateFileNameCallback = 0;
	mNormalizeNameComponentCallback = 0;
	mNormalizeContextCleanupCallback = 0;
	mTransactionNotificationCallback = 0;
	mNormalizeNameComponentExCallback = 0;

#if FLT_MGR_WIN8
	mSectionNotificationCallback = 0;
#endif // FLT_MGR_WIN8
}

/**********************************************************************************/
#pragma LOCKEDCODE

CMiniFltFilter::~CMiniFltFilter(void)
{
}


/**********************************************************************************/
#pragma INITCODE

void 
CMiniFltFilter::RegisterCallback(int nIndex,void* pFn)
{
	if (nIndex == 0)
	{
		mFilterUnloadCallback = (PFLT_FILTER_UNLOAD_CALLBACK)pFn;
		mFltRegistration.FilterUnloadCallback = mFilterUnloadCallback;
	}

	if (nIndex == 1)
	{
		mInstanceSetupCallback = (PFLT_INSTANCE_SETUP_CALLBACK)pFn;
		mFltRegistration.InstanceSetupCallback = mInstanceSetupCallback;
	}

	if (nIndex == 2)
	{
		mInstanceQueryTeardownCallback = (PFLT_INSTANCE_QUERY_TEARDOWN_CALLBACK)pFn;
		mFltRegistration.InstanceQueryTeardownCallback = mInstanceQueryTeardownCallback;
	}

	if (nIndex == 3)
	{
		mInstanceTeardownStartCallback = (PFLT_INSTANCE_TEARDOWN_CALLBACK)pFn;
		mFltRegistration.InstanceTeardownStartCallback = mInstanceTeardownStartCallback;
	}

	if (nIndex == 4)
	{
		mInstanceTeardownCompleteCallback = (PFLT_INSTANCE_TEARDOWN_CALLBACK)pFn;
		mFltRegistration.InstanceTeardownCompleteCallback = mInstanceTeardownCompleteCallback;
	}

	if (nIndex == 5)
	{
		mGenerateFileNameCallback = (PFLT_GENERATE_FILE_NAME)pFn;
		mFltRegistration.GenerateFileNameCallback = mGenerateFileNameCallback;
	}
	
	if (nIndex == 6)
	{
		mNormalizeNameComponentCallback = (PFLT_NORMALIZE_NAME_COMPONENT)pFn;
		mFltRegistration.NormalizeNameComponentCallback = mNormalizeNameComponentCallback;
	}
	
	if (nIndex == 7)
	{
		mNormalizeContextCleanupCallback = (PFLT_NORMALIZE_CONTEXT_CLEANUP)pFn;
		mFltRegistration.NormalizeContextCleanupCallback = mNormalizeContextCleanupCallback;
	}
				
	if (nIndex == 8)
	{
		mTransactionNotificationCallback = (PFLT_TRANSACTION_NOTIFICATION_CALLBACK)pFn;
		mFltRegistration.TransactionNotificationCallback = mTransactionNotificationCallback;
	}

	if (nIndex == 9)
	{
		mNormalizeNameComponentExCallback = (PFLT_NORMALIZE_NAME_COMPONENT_EX)pFn;
		mFltRegistration.NormalizeNameComponentExCallback = mNormalizeNameComponentExCallback;
	}
			;
}


/**********************************************************************************/
#pragma INITCODE

void 
CMiniFltFilter::RegisterOptRegistration(
	int nIdex,
	USHORT MajorFunction,
	FLT_OPERATION_REGISTRATION_FLAGS Flags,
	PFLT_PRE_OPERATION_CALLBACK      PreOperation,
	PFLT_POST_OPERATION_CALLBACK     PostOperation,
	PVOID                            Reserved1
	)
{
	mOptRegistration[nIdex].MajorFunction = static_cast<UCHAR>(MajorFunction);
	mOptRegistration[nIdex].Flags = Flags;
	mOptRegistration[nIdex].PreOperation = PreOperation;
	mOptRegistration[nIdex].PostOperation = PostOperation;
	mOptRegistration[nIdex].Reserved1 = Reserved1;
}

void CMiniFltFilter::RegisterContext(
							int		nIdex,
							FLT_CONTEXT_TYPE	ContextType,
							USHORT				ContextSize,
							ULONG				PoolTag,
							PVOID               Reserved1
							)
{
	mContextRegistration[nIdex].Reserved1 = Reserved1;
	mContextRegistration[nIdex].Size = ContextSize;
	mContextRegistration[nIdex].ContextAllocateCallback = 0;
	mContextRegistration[nIdex].ContextCleanupCallback = 0;
	mContextRegistration[nIdex].ContextFreeCallback = 0;
	mContextRegistration[nIdex].ContextType = ContextType;
	mContextRegistration[nIdex].Size = ContextSize;
	mContextRegistration[nIdex].PoolTag = PoolTag;
}

/**********************************************************************************/
#pragma INITCODE

CErrorStatus 
CMiniFltFilter::FltRegisterFilter(PDRIVER_OBJECT DriverObj)
{
	CErrorStatus Status;

	Status = ::FltRegisterFilter(DriverObj,&mFltRegistration,&mFilter);

	if (Status.IsNtSuccess())
	{
		::FltStartFiltering(mFilter);
	}

	return Status;
}

/**********************************************************************************/
#pragma LOCKEDCODE

NTSTATUS 
CMiniFltFilter::FilterUnloadCallback(
	FLT_FILTER_UNLOAD_FLAGS Flags
	)
{
	return STATUS_SUCCESS;
}


/**********************************************************************************/
#pragma LOCKEDCODE

NTSTATUS
CMiniFltFilter::InstanceSetupCallback(
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

	return STATUS_UNSUCCESSFUL;
}


/**********************************************************************************/
#pragma LOCKEDCODE

NTSTATUS
CMiniFltFilter::InstanceQueryTeardownCallback(
	PCFLT_RELATED_OBJECTS FltObjects,
	FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
	)
{
	return STATUS_SUCCESS;
}