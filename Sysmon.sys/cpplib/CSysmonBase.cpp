#include "CSysmonBase.h"
#include <CErrorStatus.h>
#include <cpplib.h>

CSysmonBase::CSysmonBase(void)
{
}


CSysmonBase::~CSysmonBase(void)
{
}


/**********************************************************************************/
#pragma LOCKEDCODE

USHORT
CSysmonBase::SysmonIsNamedPipe(PCFLT_RELATED_OBJECTS pFltFileObj)
{
	CErrorStatus Status;
	USHORT Type;	
	UNICODE_STRING DestinationString;
	UNICODE_STRING String2;
	UNICODE_STRING String1;
	SIZE_T NumberOfBytes;

	String1.Buffer = 0;
	String1.Length = 0;

	Status = FltGetVolumeName(
							pFltFileObj->Volume,
							&String1,
							&NumberOfBytes);

	if ( Status == STATUS_BUFFER_TOO_SMALL )
	{
		if ( NumberOfBytes != 36 && NumberOfBytes != 34 )
			return 0;

		String1.Buffer = (PWSTR)ExAllocatePoolWithTag(
													NonPagedPool,
													NumberOfBytes,
													'PsyS');
		if ( !String1.Buffer )
			return 0;

		String1.Length = NumberOfBytes & 0xFF;
		String1.MaximumLength = NumberOfBytes & 0xFF;
		Status = FltGetVolumeName(
							pFltFileObj->Volume, 
							&String1,
							&NumberOfBytes);
	}

	do 
	{
		if ( Status != STATUS_SUCCESS)
		{
			if ( !String1.Buffer )
				return 0;

			ExFreePoolWithTag(
							String1.Buffer,
							'EPIP');
			return 0;
		}

		RtlInitUnicodeString(
			&DestinationString,
			L"\\Device\\NamedPipe");
		RtlInitUnicodeString(
						&String2,
						L"\\");

		// 判断是否是NampedPipe
		if ( RtlCompareUnicodeString(
									&String1,
									&DestinationString,
									1) )
		{
			if ( RtlCompareUnicodeString(
										&String1,
										&String2, 
										1) )
			{

				ExFreePoolWithTag(
					String1.Buffer,
					'EPIP');

				return 0;
			}

			ExFreePoolWithTag(
							String1.Buffer, 
							'EPIP');
			Type = 1;// 0表示是文件路径，1表示有问题
		}
		else
		{
			ExFreePoolWithTag(
							String1.Buffer,
							'EPIP');
			Type = 2;// 2表示管道
		}

	} while (FALSE);	

	return Type;
}

/**********************************************************************************/
#pragma LOCKEDCODE

BOOL 
CSysmonBase::SysmonFileIsCommonName(
							PFLT_CALLBACK_DATA CallbackData
							)
{
	BOOL IsDirectory;
	PFLT_FILE_NAME_INFORMATION FileNameInformation;

	IsDirectory = 0;
	if ( CallbackData )
	{
		if ( !CallbackData->Iopb->MajorFunction )
		{
			FileNameInformation = 0;

			// #define FLT_FILE_NAME_NORMALIZED    0x01
			//     #define FLT_FILE_NAME_OPENED        0x02
			//     #define FLT_FILE_NAME_SHORT         0x03
			//  #define FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP 0x0400
			if ( NT_SUCCESS(FltGetFileNameInformation(
									CallbackData,
									FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP |
									FLT_FILE_NAME_OPENED,
									&FileNameInformation)) )
			{
				if ( NT_SUCCESS(FltParseFileNameInformation(FileNameInformation))
					&& !FileNameInformation->ParentDir.Length
					&& !FileNameInformation->Share.Length
					&& !FileNameInformation->FinalComponent.Length
					&& !FileNameInformation->Extension.Length
					&& FileNameInformation->Volume.Length )
				{
					IsDirectory = 1;
				}

				FltReleaseFileNameInformation(FileNameInformation);
			}
		}
	}

	return IsDirectory;
}


/**********************************************************************************/
#pragma LOCKEDCODE

bool 
CSysmonBase::SysmonQueryFileFileAttributes(PFLT_RELATED_OBJECTS pFltObj)
{
	FILE_BASIC_INFORMATION FileBaseInfo;

	return NT_SUCCESS(FltQueryInformationFile(
							pFltObj->Instance,
							pFltObj->FileObject,
							&FileBaseInfo,
							sizeof(FileBaseInfo), 
							FileBasicInformation, 0))
		&& !(FileBaseInfo.FileAttributes &
		(FILE_ATTRIBUTE_DEVICE|
		FILE_ATTRIBUTE_DIRECTORY));
}