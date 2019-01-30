#include "StdAfx.h"
#include "CEventLogger.h"

#include <strsafe.h>
#pragma comment (lib, "strsafe.lib")


CEventLogger::CEventLogger(void):
m_hTrace(0), 
	m_hSession(0),
	m_bLoggingEnabled(FALSE),
	m_nLoggingLevel(TRACE_LEVEL_INFORMATION)
{ 
	memset(
		&m_provider_guid,
		0, 
		sizeof(m_provider_guid)); 
	memset(
		&m_session_guid,
		0,
		sizeof(m_session_guid)); 
}

CEventLogger::~CEventLogger(void) 
{ 
	DeInit();
}

/* Initialize the logger lpszLogFile: location of log file, 
must end with .etl and all folders in path must exists lpszSessionName:
unique session name, like "_trace" */

HRESULT CEventLogger::Init(
	LPCTSTR lpszLogFile, 
	LPCTSTR lpszSessionName, 
	GUID *pguid) 
{ 
	HRESULT hr = S_OK; 
	if(NULL == m_hTrace) 
	{ 
		m_strLogFile = lpszLogFile;
		m_strSessionName = lpszSessionName;
		if(m_strLogFile.GetLength() > 0 && m_strSessionName.GetLength() > 0) 
		{
			// if caller didn¡¯t supply a GUID we will create one on the fly 
			if(NULL == pguid) 
				hr = CoCreateGuid(&m_provider_guid); 
			else 
				memcpy_s(
					&m_provider_guid,
					sizeof(m_provider_guid),
					pguid, 
					sizeof(GUID));

			// register us as event provider
			hr = AtlHresultFromWin32(
				RegisterTraceGuids(
							&CEventLogger::ControlCallback, 
							this,
							&m_provider_guid,
							NULL,
							NULL,
							NULL,
							NULL,
							&m_hTrace));

			if(SUCCEEDED(hr))
			{
				m_hSession = NULL;
				m_bLoggingEnabled = FALSE;
				m_nLoggingLevel = TRACE_LEVEL_INFORMATION;

				/**
				Initialize the structure that will be used for writing events, the structure looks like this:
				struct{
				EVENT_TRACE_HEADER header;
				MOF_FIELD mof;
				}
				We will use only one MOF_FIELD after the EVENT_TRACE_HEADER and the logging text will be put here
				*/

				if(m_eventdata.GetCount() == 0)
					m_eventdata.SetCount(sizeof(EVENT_TRACE_HEADER) + sizeof(MOF_FIELD));
				if(m_eventdata.GetCount() > 0)
				{
					memset(m_eventdata.GetData(), 0, m_eventdata.GetCount());
					PEVENT_TRACE_HEADER pEventHeader = (PEVENT_TRACE_HEADER)m_eventdata.GetData();
					pEventHeader->Size = m_eventdata.GetCount();
					pEventHeader->Flags = WNODE_FLAG_USE_MOF_PTR;
					pEventHeader->Class.Type = EVENT_TRACE_TYPE_INFO;
				}

				hr = CoCreateGuid(&m_session_guid);
				m_sessiondata.SetCount(
					sizeof(EVENT_TRACE_PROPERTIES) + 
					(m_strLogFile.GetLength() + m_strSessionName.GetLength() + 2) * 2);

				if(m_sessiondata.GetCount() > 0)
				{
					memset(
						m_sessiondata.GetData(), 
						0, 
						m_sessiondata.GetCount());

					PEVENT_TRACE_PROPERTIES pTracePropperties = (PEVENT_TRACE_PROPERTIES)m_sessiondata.GetData();
					pTracePropperties->Wnode.BufferSize = m_sessiondata.GetCount();
					pTracePropperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
					pTracePropperties->Wnode.ClientContext = 1;
					memcpy_s(
							&pTracePropperties->Wnode.Guid,
							sizeof(pTracePropperties->Wnode.Guid),
							&m_session_guid,
							sizeof(m_session_guid));

					pTracePropperties->LogFileMode = EVENT_TRACE_FILE_MODE_CIRCULAR | 
													EVENT_TRACE_USE_PAGED_MEMORY;

					pTracePropperties->MaximumFileSize = 1;
					pTracePropperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
					StringCchCopyW(
							(LPWSTR)((LPBYTE)pTracePropperties + pTracePropperties->LogFileNameOffset),
							m_strLogFile.GetLength() + 1,
							m_strLogFile.GetString());

					pTracePropperties->LoggerNameOffset =
											pTracePropperties->LogFileNameOffset + 
													(m_strLogFile.GetLength() + 1) * 2;

					StringCchCopyW(
						(LPWSTR)((LPBYTE)pTracePropperties + pTracePropperties->LoggerNameOffset), 
						m_strSessionName.GetLength() + 1, 
						m_strSessionName.GetString());

					// start and enable trace
					hr = AtlHresultFromWin32(
						StartTrace(
							&m_hSession, 
							m_strSessionName,
							pTracePropperties));

					if(SUCCEEDED(hr))
					{
						hr = AtlHresultFromWin32(
										EnableTrace(
												TRUE, 
												0, 
												TRACE_LEVEL_VERBOSE,
												&m_provider_guid,
												m_hSession));
					}
				}
			}
		}
		else
		{
			hr = E_INVALIDARG;
		}
	}

	return hr;
}


HRESULT CEventLogger::DeInit()
{ 
	HRESULT hr = S_OK; 
	if(NULL != m_hSession) 
	{ 
		// Disable provider 
		hr = AtlHresultFromWin32(
						EnableTrace(
									FALSE, 
									0, 
									0, 
									&m_provider_guid,
									m_hSession));

		// Stop trace
		hr = AtlHresultFromWin32(
			ControlTrace(
			m_hSession, 
			m_strSessionName,
			(PEVENT_TRACE_PROPERTIES)m_sessiondata.GetData(),
			EVENT_TRACE_CONTROL_STOP)
			);
	}

	if(NULL != m_hTrace)
	{
		// Unregister provider
		hr = AtlHresultFromWin32(
					UnregisterTraceGuids(m_hTrace));

		m_hTrace = NULL;
		m_hSession = NULL;
	}
	return hr;
}

void CEventLogger::Log(
	UCHAR nLevel, 
	LPCTSTR lpszFormat,...)
{ 
	if(NULL != m_hSession && nLevel <= m_nLoggingLevel && m_eventdata.GetCount() > 0)
	{
		va_list args;
		va_start(args, lpszFormat);
		CString strLog; 
		strLog.FormatV(lpszFormat, args);
		va_end(args);

		PEVENT_TRACE_HEADER pEventHeader = (PEVENT_TRACE_HEADER)m_eventdata.GetData();
		PMOF_FIELD pMOF = (PMOF_FIELD)((LPBYTE)pEventHeader + sizeof(EVENT_TRACE_HEADER));

		// Put the string as binary into the first the only MOF structure
		pEventHeader->Class.Level = nLevel;
		pMOF->Length = (strLog.GetLength() + 1) * sizeof(TCHAR);
		pMOF->DataPtr = (ULONG64)strLog.GetString();

		// Write event
		HRESULT hr = AtlHresultFromWin32(
								TraceEvent(
										m_hSession, 
										pEventHeader)
										);
	}
}

ULONG CEventLogger::ControlCallback(
	WMIDPREQUESTCODE nRequestCode, 
	PVOID pContext, 
	ULONG* Reserved, 
	PVOID pBuffer) 
{ 
	if(NULL != pContext) 
		return ((CEventLogger*)pContext)->_ControlCallback(
		nRequestCode,
		(PWNODE_HEADER)pBuffer
		); 

	return ERROR_SUCCESS; 
}

// Callback function which responses to logging control from ETW 
ULONG CEventLogger::_ControlCallback(
	WMIDPREQUESTCODE nRequestCode, 
	PWNODE_HEADER pHeader) 
{ 
	if(WMI_ENABLE_EVENTS == nRequestCode) 
	{
		m_bLoggingEnabled = TRUE; 
		m_hSession = GetTraceLoggerHandle(pHeader);
		if(NULL != m_hSession)
			m_nLoggingLevel = GetTraceEnableLevel(m_hSession);
	} 
	else if(WMI_DISABLE_EVENTS == nRequestCode)
	{
		m_bLoggingEnabled = FALSE;
		m_hSession = 0; 
	} 

	return ERROR_SUCCESS;
}