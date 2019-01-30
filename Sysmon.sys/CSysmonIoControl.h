#ifndef _CSysmonControl_h
#define _CSysmonControl_h

typedef enum _Base_Sysmon_FileObj_Type
{
	kFileObjErrorType = 0,
	kFileObjFilePath = 1,
	kFileObjNamedPipe = 2,
}Base_Sysmon_FileObj_Type;

typedef struct _Sysmon_Flt_CompletionContext
{
	union
	{
		ULONG		ContextValue;
		PVOID*		ReportEvent;
	};	
}Sysmon_Flt_CompletionContext;

typedef struct _Sysmon_Flt_Stream_Context
{
	PVOID* FsContext2;
}Sysmon_Flt_Stream_Context,*PSysmon_Flt_Stream_Context;

#endif
