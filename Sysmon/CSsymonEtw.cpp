#include "StdAfx.h"
#include "CSsymonEtw.h"
#include "CSysmonUtil.h"
#include <winsock2.h>
#include <strsafe.h>
#include <process.h>



#pragma comment(lib,"Ws2_32.lib")

#define EVENT_TRACE_SYSTEM_LOGGER_MODE 0x2000000

CSsymonEtw* CSsymonEtw::_Instace = NULL;

CSsymonEtw::CSsymonEtw(void)
{
	m_pStartTrace = NULL;
	m_pControlTrace = NULL;
	m_pOpenTrace = NULL;
	m_pProcessTrace = NULL;
	m_SessionHandle = NULL;
	m_hEtwTraceThread =0;
	m_bInit = FALSE;
	m_hGlobalEvent = CreateEvent(0, TRUE, 0, 0);
	m_bStartTrace = FALSE;
	m_bInitData = FALSE;
	m_EventClassNums = 0;

	GetSystemTimeAsFileTime(&m_SystemTime);
	QueryPerformanceCounter(&m_PerformanceCount);
	QueryPerformanceFrequency(&m_Frequency);
}


CSsymonEtw::~CSsymonEtw(void)
{
}


ULONG CSsymonEtw::InitData()
{
	FILETIME SystemTimeAsFileTime;

	if ( CSsymonEtw::m_bInitData )
	{
		//sub_4144E0(1);
	}
	else
	{
		InitializeCriticalSection(&m_csData);
		CSsymonEtw::m_bInitData = 1;
	}

	GetSystemTimeAsFileTime(&SystemTimeAsFileTime);
	m_SystemTime = SystemTimeAsFileTime;
	return SystemTimeAsFileTime.dwHighDateTime;
}

void CSsymonEtw::Enter()
{
	EnterCriticalSection(&m_csData);
}

void CSsymonEtw::Leave()
{
	LeaveCriticalSection(&m_csData);
}

HRESULT CSsymonEtw::StartTrace(BOOL bStart)
{
	WSAData Wsa = {0};
	BOOL IsThanXp = FALSE;
	DWORD dwVerison = GetVersion();
	DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVerison)));
	DWORD dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVerison)));
	DWORD FileMode = 0;
	UINT ThreadId;
	HRESULT bFail;

	if (m_bInit == FALSE)
	{
		HMODULE hAdv = LoadLibraryW(L"advapi32.dll");
		m_pStartTrace = (pStartTrace)GetProcAddress(hAdv, "StartTraceW");
		m_pControlTrace = (pControlTrace)GetProcAddress(hAdv, "ControlTraceW");
		m_pOpenTrace = (pOpenTrace)GetProcAddress(hAdv, "OpenTraceW");
		m_pProcessTrace = (pProcessTrace)GetProcAddress(hAdv, "ProcessTrace");
		int ret = WSAStartup(
						MAKEWORD(2,2),
						&Wsa);
		if ( ret )
			return ret;

		m_bInit = TRUE;
	}

	if (!m_pStartTrace)
	{
		return 1;
	}

	if (m_bStartTrace == bStart)
	{
		return 0;
	}

	if (dwMajorVersion >= 6 )
	{
		if(dwMinorVersion >= 2)
		{
			IsThanXp = TRUE;
		}
	}
		
	Sysomn_Event_Properties SysmonEvent = {0};
	SysmonEvent.Properties.Wnode.BufferSize = sizeof(Sysomn_Event_Properties);
	SysmonEvent.Properties.Wnode.Flags = WNODE_FLAG_TRACED_GUID ;
	SysmonEvent.Properties.Wnode.ClientContext = 1;
	SysmonEvent.Properties.FlushTimer = 1;
	SysmonEvent.Properties.LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
	SysmonEvent.Properties.LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) +
											MAX_PATH*sizeof(TCHAR);


	FileMode = 0;
	if (IsThanXp)
	{
		FileMode = EVENT_TRACE_SYSTEM_LOGGER_MODE;
	}

	SysmonEvent.Properties.LogFileMode = FileMode | 
								EVENT_TRACE_REAL_TIME_MODE;

	if ( dwMajorVersion < 6 || dwMinorVersion < 2)
	{
		SysmonEvent.Properties.Wnode.Guid = SystemTraceControlGuid;
	}

	LPCTSTR pLogName = _T("SYSMON TRACE");

	if (!IsThanXp)
	{
		pLogName = _T("NT Kernel Logger");
	}


	StringCchCopy(SysmonEvent.LoggerName,MAX_PATH,pLogName );

	if (CSysmonUtil::SysmonVersionIsSupport())
	{
		SysmonEvent.Properties.EnableFlags = EVENT_TRACE_FLAG_EXTENSION;
		SysmonEvent.Properties.EnableFlags |= 0x00FF0000;
		SysmonEvent.Properties.EnableFlags |=
								(EVENT_TRACE_FLAG_DISK_IO_INIT |
			                      EVENT_TRACE_FLAG_SYSTEMCALL |
								  EVENT_TRACE_FLAG_PROCESS_COUNTERS);
 		SysmonEvent.dwMax2 = 0x10009;
 		SysmonEvent.dwMax1 = 0x1000A;
 		SysmonEvent.dwMax3 = 0x10000;
	}
	else
	{
		SysmonEvent.Properties.EnableFlags |= EVENT_TRACE_FLAG_NETWORK_TCPIP; 
	}

	if (bStart)
	{
		InitData();

		ULONG bSuccess = m_pStartTrace(
											&m_SessionHandle,
											SysmonEvent.LoggerName,
											&SysmonEvent.Properties);

		if ( bSuccess && bSuccess != ERROR_ALREADY_EXISTS)
		{
			//sub_418280(&Text, L"Network trace initialization failed: Error %d", v11);
			//MessageBoxW(0, &Text, L"System Monitor", 0x10u);
			bFail = bSuccess;
		}
		else
		{
			m_hEtwTraceThread = (HANDLE)
										_beginthreadex(
													0,
													0,
													CSsymonEtw::ProcessTraceThread,
													0,
													0,
													&ThreadId);
			m_bStartTrace = bStart;
			bFail = 0;
		}
	}
	else
	{
		SetEvent(m_hGlobalEvent);
		bFail = m_pControlTrace(
									m_SessionHandle,
									SysmonEvent.LoggerName, 
									&SysmonEvent.Properties,
									EVENT_TRACE_CONTROL_STOP);

		WaitForSingleObject(m_hEtwTraceThread, INFINITE);
		CloseHandle(m_hEtwTraceThread);
		m_hEtwTraceThread = 0;
		ResetEvent(m_hGlobalEvent);

		if ( !bFail )
		{
			m_bStartTrace = 0;
			//sub_414630();
		}
	}

	return bFail;
}

void* CSsymonEtw::GetEventClassPropertyListList(PEVENT_TRACE pEventTrace)
{

	if (!m_EventClassNums)
	{
		//EventTraceGuid

		m_EventClassNums++;
	}

	if (pEventTrace)
	{

	}

	return 0;
}

UINT CSsymonEtw::ProcessTraceThread(void* lp)
{
	BOOL IsThanXp = FALSE;
	BOOL IsStop = FALSE;
	DWORD dwVerison = GetVersion();
	DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVerison)));
	DWORD dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVerison)));

	if (dwMajorVersion >= 6 )
	{
		if(dwMinorVersion >= 2)
		{
			IsThanXp = TRUE;
		}
	}

	EVENT_TRACE_LOGFILE SysmonEvent = {0};
	SysmonEvent.LoggerName = NULL;
	SysmonEvent.Context = 0;
	SysmonEvent.BufferCallback = CSsymonEtw::BufferCallback;
	SysmonEvent.BuffersRead = 0;
	

	LPCTSTR pLogName = _T("SYSMON TRACE");

	if (!IsThanXp)
	{
		pLogName = _T("NT Kernel Logger");
	}

	SysmonEvent.LoggerName = (LPWSTR)pLogName;
	SysmonEvent.CurrentTime = 0;
	SysmonEvent.EventCallback = CSsymonEtw::EventCallback;
	SysmonEvent.LogFileMode = EVENT_TRACE_REAL_TIME_MODE |
							EVENT_TRACE_ADD_HEADER_MODE;

	TRACEHANDLE hOpenTrace = CSsymonEtw::Instance()->m_pOpenTrace(&SysmonEvent);

	if (!hOpenTrace)
	{
		return -1;
	}

	UINT ThreadId = 0;
	HANDLE hDataThread = (void *)_beginthreadex(
												0,
												0,
												ProcessDataThread, 
												0,
												0,
												&ThreadId);

	 ULONG bSuccess = CSsymonEtw::Instance()->m_pProcessTrace(
												&hOpenTrace,
												1,
												0,
												0);

	 if ( WaitForSingleObject(
						CSsymonEtw::Instance()->m_hGlobalEvent,
						0) == STATUS_TIMEOUT )
	 {
		 IsStop = 1;
		 SetEvent(CSsymonEtw::Instance()->m_hGlobalEvent);
	 }

	 WaitForSingleObject(hDataThread, INFINITE);
	 CloseHandle(hDataThread);

	 if ( IsStop )
	 {
		 CloseHandle(CSsymonEtw::Instance()->m_hEtwTraceThread);
		 CSsymonEtw::Instance()->m_hEtwTraceThread = 0;
		 ResetEvent(CSsymonEtw::Instance()->m_hGlobalEvent);

		 CSsymonEtw::Instance()->m_bStartTrace = 0;

		 //sub_414630();

// 		 if ( sub_407C70() & 1 )
 			 CSsymonEtw::Instance()->StartTrace(1);
	 }

	return 0;
}


ULONG 
CSsymonEtw::BufferCallback(
	PEVENT_TRACE_LOGFILE Buffer
	)
{
	return 0;
}

#include "CMofDataParser.h"
VOID
CSsymonEtw::EventCallback(
	PEVENT_TRACE pEvent
	)
{
	if (pEvent)
	{
		//CMofParseRes Result;
		//CSsymonEtw::Instance()->m_MofDataParser.Parse(pEvent,Result);
		
		return;

 		CSysmonDataEntry* pDataEntry = CSsymonEtw::Instance()->m_SysmonMofData.Phase_1(
 															&pEvent->Header.Guid,
 															pEvent->Header.Class.Version,															
 															pEvent->Header.Class.Level,
 															pEvent->Header.Class.Type);
 
 		if (pDataEntry)
 		{
 			CSsymonEtw::Instance()->Enter();
 			CSsymonEtw::Instance()->m_SysmonMofData.Phase_2(pEvent,pDataEntry);
 			CSsymonEtw::Instance()->Leave();
 		}
	}
	
}


UINT
CSsymonEtw::ProcessDataThread(void* lp)
{
	LARGE_INTEGER PerformanceCount;

	while ( WaitForSingleObject(
					CSsymonEtw::Instance()->m_hGlobalEvent, 
					250) == STATUS_TIMEOUT )
	{
		CSsymonEtw::Instance()->Enter();

		QueryPerformanceCounter(&PerformanceCount);
		ULONG LowPart = PerformanceCount.LowPart - 
			CSsymonEtw::Instance()->m_PerformanceCount.LowPart;

		ULONG HighPart = (PerformanceCount.HighPart -
				CSsymonEtw::Instance()->m_PerformanceCount.HighPart);

		ULONGLONG QuadPart = PerformanceCount.QuadPart - 
			CSsymonEtw::Instance()->m_PerformanceCount.QuadPart;

		ULONGLONG Frequency = 0;
		LARGE_INTEGER ft2 = {0};
		LARGE_INTEGER lg2 = {0};
		LARGE_INTEGER lg3 = {0};

		if ( HighPart >= 0 )
		{
			lg2.QuadPart = ((10000000 * (QuadPart % CSsymonEtw::Instance()->m_Frequency.QuadPart)) / 
				CSsymonEtw::Instance()->m_Frequency.QuadPart +
					10000000 * (QuadPart / CSsymonEtw::Instance()->m_Frequency.QuadPart));

			ft2.LowPart = lg2.LowPart;
		}
		else
		{
			

			lg2.QuadPart = CSysmonUtil::CounterTimes(
													CSsymonEtw::Instance()->m_SystemTime,
													-HighPart);

			ft2.LowPart = lg2.LowPart;
		}

		ft2.HighPart = CSsymonEtw::Instance()->m_SystemTime.dwHighDateTime;
		lg3.LowPart += CSsymonEtw::Instance()->m_SystemTime.dwLowDateTime;
		lg2.LowPart = CSsymonEtw::Instance()->m_SystemTime.dwLowDateTime;

		ft2.HighPart = (ft2.QuadPart + lg2.QuadPart) >> 32;

		CSsymonEtw::Instance()->Leave();
	}

	return 0;
}