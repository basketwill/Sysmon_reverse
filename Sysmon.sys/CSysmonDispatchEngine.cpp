#include "CSysmonDispatchEngine.h"
#include <CErrorStatus.h>
#include <CIoStackLocation.h>
#include <CIrp.h>
#include <CSysmonBase.h>
#include "CSysmonioControl.h"


#define FLT_CONTEXT(Context) (Sysmon_Flt_CompletionContext*)Context

/**********************************************************************************/
#pragma LOCKEDCODE

CSysmonDispatchEngine::CSysmonDispatchEngine(void)
{
}

/**********************************************************************************/
#pragma LOCKEDCODE

CSysmonDispatchEngine::~CSysmonDispatchEngine(void)
{
}

/**********************************************************************************/
#pragma LOCKEDCODE

NTSTATUS 
CSysmonDispatchEngine::SysmonDispatchIrp(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp)
{
	CErrorStatus Status = STATUS_SUCCESS;
	CIoStackLocation IoStack;
	CIrp SysmonIrp(Irp);

	IoStack = SysmonIrp.StackLocation();
	SysmonIrp.Information() = STATUS_SUCCESS;

	if (SysmonIrp.MajorFunction())
	{
		if (SysmonIrp.MajorFunction() != IRP_MJ_DEVICE_CONTROL)
		{
			if ( SysmonIrp.MajorFunction() == IRP_MJ_CLEANUP )
			{
				CDeviceExtension* Externsion = (CDeviceExtension*)
											SysmonIrp.FileObject()->DeviceObject->DeviceExtension;

				if (Externsion)
				{
					CIrp CsqIrp;

					do 
					{
						if (CsqIrp )
						{
							CsqIrp.Information() = STATUS_SUCCESS;
							CsqIrp.Complete(STATUS_CANCELLED,0);
						}

						CsqIrp =  IoCsqRemoveNextIrp(*Externsion,IoStack.FileObject());

					} while (CsqIrp != NULL);				
				}
				
			}
			else
			{
				Status = STATUS_INVALID_DEVICE_REQUEST;
			}			
		}
		else
		{
			//here is  cope IRP_MJ_DEVICE_CONTROL
		}
	}
	else
	{
		SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
		PRIVILEGE_SET            RequiredPrivileges;
		SeCaptureSubjectContext(&SubjectSecurityContext);

		RequiredPrivileges.PrivilegeCount = 1;
		RequiredPrivileges.Control = PRIVILEGE_SET_ALL_NECESSARY;
		RequiredPrivileges.Privilege[0].Luid.LowPart = 20;
		RequiredPrivileges.Privilege[0].Luid.HighPart = 0;
		RequiredPrivileges.Privilege[0].Attributes = 0;

		if(SePrivilegeCheck(
						&RequiredPrivileges,
						&SubjectSecurityContext,
						ExGetPreviousMode()) != TRUE)
		{
			Status = STATUS_ACCESS_DENIED;
		}

		SeReleaseSubjectContext(&SubjectSecurityContext);
	}

	if ( Status != STATUS_PENDING )
	{
		SysmonIrp.Complete(Status,0);//IofCompleteRequest(v3, 0);
	}

	return Status;
}


/**********************************************************************************/
#pragma LOCKEDCODE

NTSTATUS
CSysmonDispatchEngine::SysmonFltPreOperation(
	PFLT_CALLBACK_DATA Data,
	PFLT_RELATED_OBJECTS FltObjects,
	PVOID *CompletionContext
	)
{
	CErrorStatus Status = STATUS_WAIT_1;
	LARGE_INTEGER FileCreateTime;
	Sysmon_Flt_CompletionContext* FltCompletionContext = FLT_CONTEXT(CompletionContext);
	FltCompletionContext->ContextValue = 0;

	do 
	{
		USHORT FileObjType = CSysmonBase::SysmonIsNamedPipe(FltObjects);
		if (FileObjType == kFileObjFilePath)
		{
			Status = STATUS_SUCCESS;
			break;
		}
		else if (FileObjType == kFileObjNamedPipe )
		{
			Status = STATUS_SUCCESS;
			break;
		}

		if (!IoGetTopLevelIrp())
		{
			//表示用户模式irp
			if (CSysmonBase::SysmonFileIsCommonName(Data))
			{
				 FltCompletionContext->ContextValue = 4;
				 Status = STATUS_SUCCESS;
				break;
			}


			if ( Data->Iopb->MajorFunction == IRP_MJ_SET_INFORMATION)
			{
				// 这里设置文件basicInfo
				if ( Data->RequestorMode == UserMode           
					&& Data->Iopb->Parameters.SetFileInformation.FileInformationClass ==
					FileBasicInformation
					&& 
					Data->Iopb->Parameters.SetFileInformation.Length >= 
					sizeof(FILE_BASIC_INFORMATION) )
				{
					FILE_BASIC_INFORMATION* pBaseInfo = (FILE_BASIC_INFORMATION *)
										Data->Iopb->Parameters.SetFileInformation.InfoBuffer;

					FileCreateTime.QuadPart = pBaseInfo->CreationTime.QuadPart;
					if ( pBaseInfo )
					{
						
						if ( pBaseInfo->CreationTime.QuadPart )
						{
							if ( (FileCreateTime.HighPart & FileCreateTime.LowPart) != -1
								&& (FileCreateTime.LowPart != -2 || FileCreateTime.HighPart != -1)
								&& CSysmonBase::SysmonQueryFileFileAttributes(FltObjects) )
							{
// 								pReportEvent = SysmonCReateSetFileInfoReport(
// 									FltObjects,
// 									FileCreateTime->LowPart,
// 									FileCreateTime->HighPart);
// 								if ( pReportEvent )
// 								{
// 									*CompletionContext = pReportEvent;
// 									v3 = 0;
// 								}
							}
						}
					}
				}
			}
			else if ( Data->Iopb->MajorFunction == IRP_MJ_CLEANUP )
			{
				//if ( byte_10015C17 )
				{
					PSysmon_Flt_Stream_Context FltContext = 0;
					if ( NT_SUCCESS(FltGetStreamContext(
										FltObjects->Instance,
										FltObjects->FileObject,
										(PFLT_CONTEXT*)&FltContext)))
					{
						if ( FltObjects->FileObject->FsContext2 == FltContext->FsContext2 )
						{
							FltContext->FsContext2 = 0;
// 							v9 = (struct _Sysmon_Report_Common_Header *)SysmonGetFileReportEventAndIsCalHash(
// 								(PFLT_INSTANCE)FltObjects->Instance,
// 								FltObjects->FileObject,
// 								1);
// 							if ( v9 )
// 								SysmonReportEvent(v9);

							FltDeleteContext(FltContext);
							
						}
						FltReleaseContext(FltContext);
					}
				}
			}

			if ( !Data->Iopb->MajorFunction /*&& (byte_10015C2C || byte_10015C17)*/ )
			{
				//if ( v3 )
				{
					UCHAR CreateOption = (UCHAR)(((Data->Iopb->Parameters.Create.Options)>>24) & 0xFF);
					
					if ( CreateOption == FILE_SUPERSEDE || //新建/覆盖文件
						CreateOption == FILE_CREATE ||  //新建文件
						CreateOption == FILE_OVERWRITE || //覆盖文件
						CreateOption == FILE_MAXIMUM_DISPOSITION || //打开并覆盖/创建文件
						CreateOption == FILE_OPEN_IF //打开已存在文件
						)
					{
						Status = STATUS_SUCCESS;
						FltCompletionContext->ContextValue = 5;
					}
				}
			}
		}

	} while (FALSE);
	

	return Status;
}


/**********************************************************************************/
#pragma LOCKEDCODE

NTSTATUS
CSysmonDispatchEngine::SysmonFltPostOperation(
	PFLT_CALLBACK_DATA Data,
	PCFLT_RELATED_OBJECTS FltObjects,
	PVOID CompletionContext,
	FLT_POST_OPERATION_FLAGS Flags
	)
{
	CErrorStatus Status = STATUS_SUCCESS;

	return Status;
}


/**********************************************************************************/
#pragma LOCKEDCODE

void
CSysmonDispatchEngine::SysmonIoCsqInsertIrp(
	PIO_CSQ Csq,
	PIRP Irp
	)
{
	PLIST_ENTRY ListHead = (PLIST_ENTRY)((UCHAR*)Csq 
										- sizeof(IO_CSQ)
										- sizeof(LIST_ENTRY));

	InsertTailList(ListHead,&Irp->Tail.Overlay.ListEntry);
}


/**********************************************************************************/
#pragma LOCKEDCODE

void 
CSysmonDispatchEngine::SysmonIoCsqRemoveIrp(
	PIO_CSQ Csq,
	PIRP Irp
	)
{
	RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
}

/**********************************************************************************/
#pragma LOCKEDCODE

PIRP 
CSysmonDispatchEngine::SysmonIoCsqPeekNextIrp(
	PIO_CSQ Csq,
	PIRP Irp,
	PVOID PeekContext
	)
{
	PLIST_ENTRY ListHead = (PLIST_ENTRY)((UCHAR*)Csq 
		- sizeof(IO_CSQ)
		- sizeof(LIST_ENTRY));

	LIST_ENTRY* Flink = NULL;
	if (Irp)
	{
		Flink = Irp->Tail.Overlay.ListEntry.Flink;
	}
	else
	{
		Flink = ListHead->Flink;
	}

	if (Flink == ListHead)
	{
		return 0;
	}

	while ( PeekContext && Flink[1].Flink[3].Flink != PeekContext )
	{
		Flink = Flink->Flink;
		if ( Flink == ListHead )
			return 0;
	}

	return Irp;
}


/**********************************************************************************/
#pragma LOCKEDCODE

void 
CSysmonDispatchEngine::SysmonIoCsqAcquireLock(
	PIO_CSQ Csq,
	PKIRQL Irql
	)
{
	ExAcquireFastMutex((PFAST_MUTEX)&Csq[-1]);
}


/**********************************************************************************/
#pragma LOCKEDCODE
	
void
CSysmonDispatchEngine::SysmonIoCsqReleaseLock(
	PIO_CSQ Csq,
	KIRQL Irql
	)
{
	ExReleaseFastMutex((PFAST_MUTEX)&Csq[-1]);
}

/**********************************************************************************/
#pragma LOCKEDCODE

void
CSysmonDispatchEngine::SysmonIoCsqCompleteCanceledIrp(
	PIO_CSQ Csq,
	PIRP Irp
	)
{
	Irp->IoStatus.Status = STATUS_CANCELLED;
	Irp->IoStatus.Information = 0;
	IofCompleteRequest(Irp, 0);
}

/**********************************************************************************/
#pragma INITCODE

void
CSysmonDispatchEngine::InitCsq(CDeviceExtension& DevExt)
{
	::IoCsqInitialize(
					DevExt,
					(PIO_CSQ_INSERT_IRP)SysmonIoCsqInsertIrp,
					(PIO_CSQ_REMOVE_IRP)SysmonIoCsqRemoveIrp,
					(PIO_CSQ_PEEK_NEXT_IRP)SysmonIoCsqPeekNextIrp,
					(PIO_CSQ_ACQUIRE_LOCK)SysmonIoCsqAcquireLock,
					(PIO_CSQ_RELEASE_LOCK)SysmonIoCsqReleaseLock,
					(PIO_CSQ_COMPLETE_CANCELED_IRP)SysmonIoCsqCompleteCanceledIrp);
}