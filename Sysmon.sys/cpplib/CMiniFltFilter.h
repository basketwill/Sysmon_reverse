#ifndef _CMiniFltFilter_h
#define _CMiniFltFilter_h

#include <fltKernel.h>
#include "CErrorStatus.h"

#define MAX_FLT_CONTEXT_REGISTRATION 20
#define MAX_FLT_OPERATION_REGISTRATION 50

typedef enum _Flt_Fn_Callback
{
	kFilterUnloadCallback,
	kInstanceSetupCallbac,
	kInstanceQueryTeardowCallback,
	kInstanceTeardownStartCallback,
	kInstanceTeardownCompleteCallback,
	kGenerateFileNameCallback,
	kNormalizeNameComponentCallback,
	kNormalizeContextCleanupCallback,
	kTransactionNotificationCallback,
	kNormalizeNameComponentExCallback,

}Flt_Fn_Callback;

class CMiniFltFilter
{
public:
	CMiniFltFilter(void);
	virtual ~CMiniFltFilter(void);

	void RegisterCallback(int nIndex,void* pFn);
	void RegisterOptRegistration(
							int nIdex,
							USHORT ContextSize,
							FLT_OPERATION_REGISTRATION_FLAGS Flags,
							PFLT_PRE_OPERATION_CALLBACK      PreOperation,
							PFLT_POST_OPERATION_CALLBACK     PostOperation,
							PVOID                            Reserved1 = NULL
							);

	void RegisterContext(
					int					nIdex,
					FLT_CONTEXT_TYPE	ContextType,
					USHORT				ContextSize,
					ULONG				PoolTag,
					PVOID               Reserved1 = NULL
					);

	CErrorStatus FltRegisterFilter(PDRIVER_OBJECT DriverObj);

public:
	static
	NTSTATUS 
	NTAPI
	FilterUnloadCallback(
		FLT_FILTER_UNLOAD_FLAGS Flags
		);

	static
	NTSTATUS
	NTAPI
	InstanceSetupCallback(
		PCFLT_RELATED_OBJECTS FltObjects,
		FLT_INSTANCE_SETUP_FLAGS Flags,
		DEVICE_TYPE VolumeDeviceType,
		FLT_FILESYSTEM_TYPE VolumeFilesystemType
		);

	static
	NTSTATUS
	NTAPI
	InstanceQueryTeardownCallback(
		PCFLT_RELATED_OBJECTS FltObjects,
		FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
		);

private:
	PFLT_FILTER								mFilter;
	FLT_REGISTRATION						mFltRegistration;
	FLT_CONTEXT_REGISTRATION				mContextRegistration[MAX_FLT_CONTEXT_REGISTRATION];
	FLT_OPERATION_REGISTRATION				mOptRegistration[MAX_FLT_OPERATION_REGISTRATION];
	
	PFLT_FILTER_UNLOAD_CALLBACK				mFilterUnloadCallback;
	PFLT_INSTANCE_SETUP_CALLBACK			mInstanceSetupCallback;
	PFLT_INSTANCE_QUERY_TEARDOWN_CALLBACK	mInstanceQueryTeardownCallback;
	PFLT_INSTANCE_TEARDOWN_CALLBACK			mInstanceTeardownStartCallback;
	PFLT_INSTANCE_TEARDOWN_CALLBACK			mInstanceTeardownCompleteCallback;
	PFLT_GENERATE_FILE_NAME					mGenerateFileNameCallback;
	PFLT_NORMALIZE_NAME_COMPONENT			mNormalizeNameComponentCallback;
	PFLT_NORMALIZE_CONTEXT_CLEANUP			mNormalizeContextCleanupCallback;
	PFLT_TRANSACTION_NOTIFICATION_CALLBACK	mTransactionNotificationCallback;
	PFLT_NORMALIZE_NAME_COMPONENT_EX		mNormalizeNameComponentExCallback;

#if FLT_MGR_WIN8
	PFLT_SECTION_CONFLICT_NOTIFICATION_CALLBACK mSectionNotificationCallback;
#endif // FLT_MGR_WIN8
};


#endif