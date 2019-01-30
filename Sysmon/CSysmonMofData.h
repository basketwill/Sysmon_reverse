#ifndef _CSysmonMofData_h
#define _CSysmonMofData_h

#include "Sysmon.h"
#include <wbemidl.h>
#include <comutil.h>
#include <wbemidl.h>
#include <wmistr.h>
#include <WbemCli.h>
#include <strsafe.h>
#include <intrin.h>
#include <Evntrace.h>

extern "C"
{
	typedef HRESULT (WINAPI * pSafeArrayDestroy)(SAFEARRAY *);
	typedef HRESULT (WINAPI * pSafeArrayGetElement)(SAFEARRAY * , LONG *, void *);
};


typedef struct _CTcpIpInfo
{
	BSTR NetName;
	ULONG NetType;
}CTcpIpInfo;

typedef struct _Sysmon_Net_Report
{
	ULONG			NrSize;
	ULONG			NrProcessId;
	ULONG			NrThreadId;
	GUID			NrGuid;
	LARGE_INTEGER	NrSystemTime;
	ULONG			NrEventType;
	ULONG			NrUdpOrTcp;
}Sysmon_Net_Report;

class CSysmonData
{
	friend class CSysmonDataEntry;
	friend class CSysmonProperty;
	friend class CSysmonMofData;
public:
	CSysmonData();
	~CSysmonData();

	BOOL Compare(BSTR pString);
	BOOL CompareN(BSTR pString,ULONG Size);

	LONG Len()
	{
		if (m_pData)
		{
			__try
			{
				return lstrlen(m_pData);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{

			}
			
		}
		return 0;
	}
private:
	BSTR m_pData;
	void* m_pAlloc;
	LONG  m_Reference; 
};

class CSysmonEventList
{
	friend class CSysmonMofData;
public:
	CSysmonEventList();
	~CSysmonEventList();
	BOOL IsListEmpty()
	{
		return ::IsListEmpty(&m_Entry);
	}

	CSysmonEventList(LIST_ENTRY* Fink,LIST_ENTRY* Blink);
	CSysmonEventList* InsertBack(LIST_ENTRY* Blink,void* pElm);
private:
	void*		m_pEvent;
public:
	LIST_ENTRY	m_Entry;
};

class CSysmonDataEntry
{
	friend class CSysmonMofData;
public:
	CSysmonDataEntry();
	CSysmonDataEntry(GUID* Guid,USHORT Type,USHORT Version,USHORT Level);
	~CSysmonDataEntry();
	void operator =(CSysmonDataEntry* pDataEntry);
	CSysmonDataEntry* InitData(CSysmonData** pData,BSTR pString);
	LONG GetCLassNameLen();
	
private:
	CSysmonData*		m_EventClassName;
	CSysmonData*		m_EventTypeName;
	GUID				m_EventGuid;
	USHORT				m_EventType;
	USHORT				m_EventVersion;
	USHORT				m_EventLevel;

	ULONG				m_ProperitySize;
	CSysmonEventList*	m_ProperityList;
};

class CSysmonProperty
{
	friend class CSysmonMofData;
public:
	CSysmonProperty()
		:m_Data(NULL)
		,m_Len(0),
		m_ArraySize(0)
	{

	}

	CSysmonProperty(BSTR pName,LONG Len,LONG ArraySize)
		:m_Data(NULL)
		,m_Len(0),
		m_ArraySize(0)
	{
		
		if (pName)
		{
			InitData(&m_Data,pName);
		}
		m_Len = Len;
		m_ArraySize = ArraySize;

	}

	~CSysmonProperty()
	{

	}

	void InitData(CSysmonData** pData,BSTR pString)
	{
		BSTR pOldData = 0;
		if (pData && *pData)
		{
			pOldData = (*pData)->m_pData;
		}

		if (!pString || pOldData != pString)
		{
			if (*pData)
			{
				if ( !InterlockedDecrement((volatile LONG *)&(*pData)->m_Reference))
				{
					if ((*pData)->m_pData)
					{
						SysFreeString((*pData)->m_pData);
						(*pData)->m_pData = 0;
					}

					if ( (*pData)->m_pAlloc )
					{
						delete (*pData)->m_pAlloc;
						(*pData)->m_pAlloc = 0;
					}

					delete (*pData);
				}

				(*pData) = NULL;
			}
		}

		CSysmonData* pNew = new CSysmonData();

		if (pNew)
		{
			pNew->m_pAlloc = 0;
			pNew->m_Reference = 1;
			pNew->m_pData = SysAllocString(pString);

			if ( !pNew->m_pData && pString )
			{
				//	LogError(0x8007000E);
			}
		}
		else
		{
			//LogError(0x8007000E);
		}

		*pData = pNew;
	}

private:
	CSysmonData*	m_Data;
	LONG			m_Len;
	LONG			m_ArraySize;
};

class CSysmonMofData
{
public:
	CSysmonMofData(void);
	virtual ~CSysmonMofData(void);

public:
	CSysmonDataEntry* GetEventCategoryClass(
										GUID* ClassGuid,
										USHORT Version,
										USHORT Level,
										USHORT Type);

	CSysmonDataEntry* GetPropertyList(
							IWbemClassObject* pClass,
							GUID* ClassGuid,
							USHORT Version,
							USHORT Level,
							USHORT Type);

	void SysmonAddProperityList(
						CSysmonEventList* pList, 
						BSTR ProperityName,
						LONG Len,
						LONG ArraySize);

	CSysmonDataEntry* Phase_1(
						GUID* ClassGuid,
						USHORT Version,
						USHORT Level,
						USHORT Type);

	void Phase_2(
				PEVENT_TRACE pEvent,
				CSysmonDataEntry* pDataEntry
						);

	HRESULT CoCreateInstance();

	LONG GetArraySize(IWbemQualifierSet* pQualifierSet);
	LONG GetArrayValue(
				CIMTYPE CimType,
				IWbemQualifierSet * pQualifierSet);

	void InsertBack(
				CSysmonEventList* pEventList,
				LONG* Reference,
				void* pElm);


	CSysmonEventList* AddEvent(CSysmonEventList* pEventList);

private:
	IWbemServices*		m_pIWbemService;
	CSysmonEventList*	m_SysmonEventList;
	LONG				m_EventListNums;
public:
	static CTcpIpInfo	m_TcpIpEventTypeName[25];
};

#endif