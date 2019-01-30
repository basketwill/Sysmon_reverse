#ifndef _CSsymonEtw_h
#define _CSsymonEtw_h

#include "CMofDataParser.h"
#include "CSysmonMofData.h"
#include <wmistr.h>
#include <Evntrace.h>

typedef void* HEVENT;

#ifdef __cplusplus
extern "C"{
#endif

typedef
ULONG
(WINAPI
*pStartTrace)(
	 PTRACEHANDLE            SessionHandle,
	 LPCTSTR                 SessionName,
	 PEVENT_TRACE_PROPERTIES Properties
	);

typedef
ULONG
(WINAPI
*pControlTrace)
(
	TRACEHANDLE             SessionHandle,
	LPCTSTR                 SessionName,
	PEVENT_TRACE_PROPERTIES Properties,
	ULONG                   ControlCode
);

typedef
TRACEHANDLE
(WINAPI
*pOpenTrace)
(
	PEVENT_TRACE_LOGFILE Logfile
);

typedef
ULONG
(WINAPI
*pProcessTrace)(
	_In_ PTRACEHANDLE HandleArray,
	_In_ ULONG        HandleCount,
	_In_ LPFILETIME   StartTime,
	_In_ LPFILETIME   EndTime
	);

#ifdef __cplusplus
};
#endif

typedef struct _Sysomn_Event_Properties
{
	EVENT_TRACE_PROPERTIES  Properties;
	TCHAR				    LoggerName[MAX_PATH];
	TCHAR					LoggerFile[MAX_PATH];
	ULONG					dwMax1;
	ULONG					dwMax2;
	ULONG					dwMax3;
	ULONG					dwMax4;
	ULONG					dwMax5;
	ULONG					dwMax6;
	ULONG					dwMax7;
	ULONG					dwMax8;
	ULONG					dwMax9;
	ULONG					dwMax10;
}Sysomn_Event_Properties;

class CSsymonEtw
{
public:
	CSsymonEtw(void);
	virtual ~CSsymonEtw(void);

public:
	HRESULT StartTrace(BOOL bStart);
	ULONG   InitData();

	void* GetEventClassPropertyListList(PEVENT_TRACE pEventTrace);
	void Enter();
	void Leave();
public:
	pStartTrace m_pStartTrace;
	pControlTrace m_pControlTrace;
	pOpenTrace m_pOpenTrace;
	pProcessTrace m_pProcessTrace;

	BOOL				m_bInit;
	BOOL				m_bStartTrace;
	TRACEHANDLE			m_SessionHandle;
	HANDLE				m_hEtwTraceThread;
	HEVENT				m_hGlobalEvent;

	BOOL				m_bInitData;
	CRITICAL_SECTION	m_csData;
	FILETIME			m_SystemTime;

	LARGE_INTEGER		m_PerformanceCount;
	LARGE_INTEGER		m_Frequency;

	UINT64				m_EventClassNums;

	CSysmonMofData		m_SysmonMofData;
	
	CMofDataParser		m_MofDataParser;
private:
	static 
	UINT
	WINAPI
	ProcessTraceThread(void* lp);

	static
	UINT
	WINAPI
	ProcessDataThread(void* lp);

	static
	ULONG
	WINAPI 
	BufferCallback(
		PEVENT_TRACE_LOGFILE Buffer
		);

	static
	VOID
	WINAPI
	EventCallback(
		PEVENT_TRACE pEvent
		);

public:
	static CSsymonEtw* Instance()
	{
		if (!_Instace)
		{
			_Instace = new CSsymonEtw();
		}

		return _Instace;
	}

	static CSsymonEtw* _Instace;
};

#endif