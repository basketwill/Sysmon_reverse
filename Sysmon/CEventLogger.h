#ifndef _CEventLogger_h
#define _CEventLogger_h
#include <CGuid.h>
#include <atlbase.h>
#include <atlstr.h> 
#include <atlcoll.h>
#include <Wmistr.h>
#include <Evntrace.h>

class CEventLogger
{
public: 
	CEventLogger(void);
	virtual ~CEventLogger(void);

	HRESULT Init(
				LPCTSTR lpszLogFile,
				LPCTSTR lpszSessionName,
				GUID *pguid = NULL);

	HRESULT DeInit();

	void Log(UCHAR nLevel, LPCTSTR lpszFormat,...);

protected: 
	static 
		ULONG
		WINAPI ControlCallback(
		WMIDPREQUESTCODE nRequestCode,
		PVOID pContext,
		ULONG* Reserved,
		PVOID pBuffer);

	ULONG _ControlCallback(
		WMIDPREQUESTCODE nRequestCode, 
		PWNODE_HEADER pHeader);

	GUID m_provider_guid;
	TRACEHANDLE m_hTrace;
	TRACEHANDLE m_hSession;
	BOOL m_bLoggingEnabled;
	UCHAR m_nLoggingLevel;
	CAtlArray<BYTE> m_eventdata;
	GUID m_session_guid;
	CStringW m_strLogFile;
	CStringW m_strSessionName;
	CAtlArray<BYTE> m_sessiondata;
};

#endif

