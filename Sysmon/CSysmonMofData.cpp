#include "StdAfx.h"
#include "CSysmonMofData.h"
#include "CSsymonEtw.h"
#include "CSysmonUtil.h"
#include "Sysmon.h"

#define MAX_MOF_DATA_LIST 357913940

CTcpIpInfo CSysmonMofData::m_TcpIpEventTypeName[25]  = {
	{_T("Send"),2},
	{_T("SendIPV4"),2},
	{_T("SendIPV6"),2},
	{_T("Recv"),3},
	{_T("RecvIPV4"),3},
	{_T("RecvIPV6"),3},
	{_T("Accept"),4},
	{_T("AcceptIPV4"),4},
	{_T("AcceptIPV6"),4},
	{_T("Connect"),5},
	{_T("ConnectIPV4"),5},
	{_T("ConnectIPV6"),5},
	{_T("Disconnect"),6},
	{_T("DisconnectIPV4"),6},
	{_T("DisconnectIPV6"),6},
	{_T("Reconnect"),7},
	{_T("ReconnectIPV4"),7},
	{_T("ReconnectIPV6"),7},
	{_T("Retransmit"),8},
	{_T("RetransmitIPV4"),8},
	{_T("RetransmitIPV6"),8},
	{_T("TCPCopy"),9},
	{_T("TCPCopyIPV4"),9},
	{_T("TCPCopyIPV6"),9}
};

CSysmonMofData::CSysmonMofData(void)
	:m_pIWbemService(NULL)
	,m_EventListNums(0)
{
	m_SysmonEventList = new CSysmonEventList(0,0);
}


CSysmonMofData::~CSysmonMofData(void)
{
	if (m_SysmonEventList)
	{
		delete m_SysmonEventList;
		m_SysmonEventList = NULL;
	}
}

HRESULT CSysmonMofData::CoCreateInstance()
{
	OLECHAR *pwWdmi;
	HRESULT hr;
	IWbemLocator *pLocator;
	
	pLocator = 0;
	pwWdmi = SysAllocString(L"root\\wmi");
	CoInitializeEx(0, 0);
	hr = ::CoCreateInstance(
							__uuidof(WbemLocator),
							0,
							CLSCTX_INPROC_SERVER,
							__uuidof(IWbemLocator),
							(LPVOID *)&pLocator);
	if ( SUCCEEDED(hr) )
	{
		hr = pLocator->ConnectServer(
								pwWdmi,
								NULL,
								NULL, 
								NULL,
								0L,
								NULL,
								NULL,
								&m_pIWbemService);

		if ( SUCCEEDED(hr) )
			hr = CoSetProxyBlanket(
								m_pIWbemService, 
								RPC_C_AUTHN_WINNT,
								RPC_C_AUTHZ_NONE,
								0,
								RPC_C_AUTHN_LEVEL_PKT,
								RPC_C_IMP_LEVEL_IMPERSONATE,
								0,
								EOAC_NONE);
	}

	SysFreeString(pwWdmi);
	if ( pLocator )
		pLocator->Release();

	return hr;
}

CSysmonDataEntry* CSysmonMofData::Phase_1(
	GUID* ClassGuid,
	USHORT Version,
	USHORT Level,
	USHORT Type)
{
	CSysmonDataEntry* pDataEntry = NULL;
	LONG nIndex = 0;
	do 
	{

		if ( !m_EventListNums )
		{
			CSysmonDataEntry* pDataEntry_1 = new CSysmonDataEntry(
				(GUID*)&EventTraceGuid,
				-1,
				0,
				0);

			if ( pDataEntry_1 )
			{
				pDataEntry_1->InitData(&pDataEntry_1->m_EventClassName, L"EventTrace");
				pDataEntry_1->InitData(&pDataEntry_1->m_EventTypeName, L"Header");

				CSysmonEventList* pEventList = m_SysmonEventList->InsertBack(
					m_SysmonEventList->m_Entry.Blink,
					pDataEntry_1);

				if ( (unsigned int)(MAX_MOF_DATA_LIST - m_EventListNums) < 1 )
				{
					//std::_Xlength_error("list<T> too long");
				}

				++m_EventListNums;

				m_SysmonEventList->m_Entry.Blink = &pEventList->m_Entry;
				pEventList->m_Entry.Blink->Flink = &pEventList->m_Entry;
			}
		}

		if ( !IsListEmpty(&m_SysmonEventList->m_Entry) )
		{
			LIST_ENTRY* pEntry = m_SysmonEventList->m_Entry.Flink;
			CSysmonDataEntry* pTem_2 = NULL;
			CSysmonDataEntry* pTem_3 = NULL;
			while(TRUE)
			{
				CSysmonEventList* pList = CONTAINING_RECORD(pEntry,CSysmonEventList,m_Entry);
				CSysmonDataEntry* pTemp = (CSysmonDataEntry*)pList->m_pEvent;
				
				int TotleSize = 12;

				if (pTemp)
				{
					LONG* p1 = (LONG*)&(pTemp->m_EventGuid);
					LONG* p2 = (LONG*)ClassGuid;

					while( *p1 == *p2)
					{
						p1++;
						p2++;

						BOOL bExit = TotleSize < 4;						
						TotleSize -= 4;

						if (bExit)
						{
							if ( pTemp->m_EventType == Type &&
								pTemp->m_EventVersion == Version)
							{

								pDataEntry = pTemp;
								return pDataEntry;
							}

							if (pTemp->m_EventType == 0x0FFFF )
							{
								if (!pTem_2)
								{
									pTem_2 = pTemp;
								}
							}

							break;
						}						
					}
				}

				pEntry = pEntry->Flink;

				if (pEntry != &m_SysmonEventList->m_Entry)
				{
					continue;
				}

				break;
			}
		}

		if (!pDataEntry)
		{
			pDataEntry = GetEventCategoryClass(ClassGuid,Version,Level,Type);

			if (!pDataEntry)
			{
				pDataEntry = new CSysmonDataEntry(
											ClassGuid,
											Type,
											Version,
											Level);

				if ( pDataEntry )
				{
					pDataEntry->m_EventClassName = NULL;
					pDataEntry->m_EventTypeName = NULL;

					pDataEntry->m_ProperityList = 0;
					pDataEntry->m_ProperitySize = 0;
						
				}
			}
		}
		
	} while (FALSE);
	

	return pDataEntry;
}

void 
CSysmonMofData::Phase_2(
	PEVENT_TRACE pEvent,
	CSysmonDataEntry* pDataEntry
	)
{
	LARGE_INTEGER SystemTime;
	LARGE_INTEGER SystemTime_2;

	__try
	{
		if (pEvent && pDataEntry)
		{
			if (pEvent->MofData && 
				pEvent->MofLength)
			{
				CSysmonData* pEventClass = pDataEntry->m_EventClassName;
				ULONG dwClassType = 0;

				if (pEventClass && pEventClass->Compare(_T("TcpIp")))
				{
					dwClassType = 1;
				}
				else if (pEventClass && pEventClass->Compare(_T("UdpIp")))
				{
					dwClassType = 2;
				}
				else if (pEventClass && pEventClass->Compare(_T("MSNT_TcpIpInformation")))
				{
					dwClassType = 3;
				}

				if ( dwClassType == 1 ||
					dwClassType == 2 ||
					dwClassType == 3 )
				{
					BOOL v15 = (pEvent->Header.TimeStamp.HighPart == 0x1000000);
					BOOL v16 = (pEvent->Header.TimeStamp.HighPart > 0x1000000);

					if (pEvent->Header.TimeStamp.HighPart < 0x1000000 ||
						pEvent->Header.TimeStamp.LowPart < 0 )
					{
						SystemTime.QuadPart = CSysmonUtil::CounterTimes(
							CSsymonEtw::Instance()->m_SystemTime,
							pEvent->Header.TimeStamp.QuadPart - 
							CSsymonEtw::Instance()->m_PerformanceCount.QuadPart
							);

						SystemTime_2.LowPart = CSsymonEtw::Instance()->m_SystemTime.dwLowDateTime;
						SystemTime_2.HighPart = CSsymonEtw::Instance()->m_SystemTime.dwHighDateTime;
						SystemTime_2.QuadPart += SystemTime.QuadPart;

						SystemTime.LowPart = CSsymonEtw::Instance()->m_SystemTime.dwLowDateTime;
						SystemTime.HighPart = CSsymonEtw::Instance()->m_SystemTime.dwHighDateTime;
					}
					else
					{
						SystemTime_2.HighPart = pEvent->Header.TimeStamp.HighPart;
						SystemTime_2.LowPart = pEvent->Header.TimeStamp.LowPart;

						SystemTime.LowPart = CSsymonEtw::Instance()->m_SystemTime.dwLowDateTime;
						SystemTime.HighPart = CSsymonEtw::Instance()->m_SystemTime.dwHighDateTime;
					}


					if (SystemTime_2.QuadPart >= SystemTime.QuadPart)
					{
						CSysmonData* pEventType = pDataEntry->m_EventTypeName;
						LONG EventType = 0;
						BOOL IsUdp = FALSE;

						__try
						{
							IsUdp = pEventClass->CompareN(_T("Udp"),3);
						}
						__except(EXCEPTION_EXECUTE_HANDLER)
						{
							IsUdp = FALSE;
						}
						
						if (pEventType && pEventType->Len())
						{						
							LONG nI = 0;
							BOOL NotEque = TRUE;
							__try
							{
								while(TRUE && nI < 24)
								{
									BSTR pEventTypeName_1 = CSysmonMofData::m_TcpIpEventTypeName[nI].NetName;
									BSTR pEventTypeName_2 = pEventType->m_pData;
									while(TRUE && pEventTypeName_1 && pEventTypeName_2)
									{
										if (pEventTypeName_1[0] != pEventTypeName_2[0])
										{
											break;
										}

										if (pEventTypeName_1[0] == 0)
										{
											NotEque = FALSE;
											break;
										}

										pEventTypeName_1++;
										pEventTypeName_2++;
									}

									if (NotEque == FALSE)
									{
										break;
									}

									nI++;

									if (nI > 24)
									{
										EventType = 0;
										__leave;
									}
								}

								EventType = CSysmonMofData::m_TcpIpEventTypeName[nI].NetType;
							}
							__except(EXCEPTION_EXECUTE_HANDLER)
							{

							}							
							
						}
						else
						{
							EventType = (pEvent->Header.Class.Type==11)?3:(pEvent->Header.Class.Type==10)+1;
						}

						if (IsUdp == TRUE || EventType == 4 || EventType == 5)
						{
							if ( pDataEntry->m_ProperityList &&
								!pDataEntry->m_ProperityList->IsListEmpty())
							{
								__try
								{
									Sysmon_Net_Report* NetReport = (Sysmon_Net_Report*)
										malloc(sizeof(Sysmon_Net_Report));

									if (NetReport)
									{
										NetReport->NrEventType = 1;
										NetReport->NrProcessId = pEvent->Header.ProcessId;
										if (pEvent->Header.ThreadId == -1)
										{
											NetReport->NrThreadId = 0;
										}
										else
										{
											NetReport->NrThreadId = pEvent->Header.ThreadId;
										}

										__m128i m128Zero = {0};
										_mm_storeu_si128((__m128i *)&NetReport->NrGuid, m128Zero);
									}
								}
								__except(EXCEPTION_EXECUTE_HANDLER)
								{

								}
								
							}
						}
					}
				}				
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{

	}
}

CSysmonDataEntry* 
CSysmonMofData::GetEventCategoryClass(
	GUID* ClassGuid,
	USHORT Version,
	USHORT Level,
	USHORT Type)
{
	HRESULT hr = S_OK;
	OLECHAR* oleTrace = NULL;
	OLECHAR* oleClassName = NULL;
	OLECHAR* oleGuid = NULL;
	OLECHAR* oleVersion = NULL;
	VARIANT varGuid;
	VARIANT varVersion;
	VARIANT varClassName;
	CSysmonDataEntry* pDataEntry;
	OLECHAR* oleClassName_2 = NULL;
	pDataEntry = NULL;
	IEnumWbemClassObject* pClasses = NULL;
	IEnumWbemClassObject* pClasses_2 = NULL;
	IWbemClassObject* pClass = NULL;
	IWbemClassObject* pClass_2 = NULL;
	IWbemQualifierSet* pQualifiers = NULL;
	ULONG CntTrace = 1;
	ULONG CntClass = 1;
	TCHAR EventGuid[5*sizeof(GUID)] = {0};
	TCHAR GuidString[5*sizeof(GUID)] = {0};

	VariantInit(&varClassName);
	VariantInit(&varGuid);
	VariantInit(&varVersion);

	do
	{
		if (!m_pIWbemService && CoCreateInstance())
		{
			break;
		}

		StringCchPrintf(
			EventGuid,
			5*sizeof(GUID),
			_T("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
			ClassGuid->Data1,
			ClassGuid->Data2,
			ClassGuid->Data3,
			ClassGuid->Data4[0],
			ClassGuid->Data4[1],
			ClassGuid->Data4[2],
			ClassGuid->Data4[3],
			ClassGuid->Data4[4],
			ClassGuid->Data4[5],
			ClassGuid->Data4[6],
			ClassGuid->Data4[7]);

		oleTrace		= SysAllocString(_T("EventTrace"));
		oleClassName	= SysAllocString(_T("__CLASS"));
		oleGuid			= SysAllocString(_T("Guid"));
		oleVersion	= SysAllocString(_T("EventVersion"));

		hr = m_pIWbemService->CreateClassEnum(
									oleTrace,
									WBEM_FLAG_DEEP | 
									WBEM_FLAG_UPDATE_ONLY | 
									WBEM_FLAG_USE_AMENDED_QUALIFIERS,
									0,
									&pClasses);
		SysFreeString(oleTrace);

		if (hr != S_OK)
		{
			break;
		}

		__try
		{		

			do 
			{
				if (pClass)
				{
					pClass->Release();
					pClass = NULL;
				}
				
				if( pClasses->Next(5000,1, &pClass, &CntTrace) != S_OK)
				{
					continue;
				}

				if (CntTrace != 1)
				{
					break;
				}

				if ( pClass->Get(oleClassName, 0, &varClassName, 0, 0) != S_OK )
					continue;

				oleClassName_2 = SysAllocString(varClassName.bstrVal);
				m_pIWbemService->CreateClassEnum(
					oleClassName_2,
					WBEM_FLAG_DEEP | 
					WBEM_FLAG_UPDATE_ONLY | 
					WBEM_FLAG_USE_AMENDED_QUALIFIERS,
					0,
					&pClasses_2);

				SysFreeString(oleClassName_2);
				VariantClear(&varClassName);

				CntClass = 1;

				do 
				{
					pClass_2 = NULL;
					if (pClasses_2)
					{
						if ( pClasses_2->Next(5000, 1, &pClass_2, &CntClass) != S_OK )
							continue;

						if ( CntClass != 1 )
							break;					
					}
					else
					{
						CntClass = 1;
						pClass_2 = pClass;
					}

					HRESULT hr2 = pClass_2->Get(oleClassName, 0, &varClassName, 0, 0);
					VariantClear(&varClassName);

					if (hr2 == S_OK)
					{
						if ( pQualifiers )
						{
							pQualifiers->Release();
							pQualifiers = 0;
						}

						HRESULT hrQualifier = pClass_2->GetQualifierSet(&pQualifiers);

						if ( FAILED(hrQualifier))
						{
							break;
						}

						if(pQualifiers->Get(
							oleGuid,
							0,
							&varGuid,
							0) == S_OK)
						{
							wcscpy_s(GuidString, 5*sizeof(GUID), varGuid.bstrVal);
							VariantClear(&varGuid);

 							if ( !wcsstr(GuidString, L"{") )
 								StringCchPrintf(GuidString, 5*sizeof(GUID),_T("{%s}"),GuidString);

							if ( !_wcsicmp(GuidString, EventGuid) )
							{
								if(pQualifiers->Get(
									oleVersion,
									0,
									&varVersion,
									0) == S_OK)
								{
									VariantChangeType(&varVersion, &varVersion, 0, VT_I2);
									WORD VersionTmp = varVersion.iVal;
									VariantClear(&varVersion);
									if ( Version != VersionTmp )
										continue;

									pDataEntry = GetPropertyList(
															pClass_2,
															ClassGuid,
															Version,
															Level,
															Type);
								}
								else
								{
									pDataEntry = GetPropertyList(
																pClass_2,
																ClassGuid,
																Version,
																Level,
																Type);
								}

								__leave;
							}							
						}					
					}

				} while (CntClass == 1);

				if (pClasses_2)
				{
					pClasses_2->Release();
					pClasses_2 = NULL;
				}

			} while (CntTrace == 1);

		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{

		}
		
		if (pClasses)
		{
			pClasses->Release();
			pClasses = NULL;
		}

	} while (FALSE);
		
	VariantClear(&varGuid);
	VariantClear(&varVersion);

	SysFreeString(oleGuid);
	SysFreeString(oleClassName);
	SysFreeString(oleVersion);

	if (pClasses)
	{
		pClasses->Release();
		pClasses = NULL;
	}

	if (pClasses_2)
	{
		pClasses_2->Release();
		pClasses_2 = NULL;
	}

	if (pQualifiers)
	{
		pQualifiers->Release();
		pQualifiers = NULL;
	}

	return pDataEntry;
}

void CSysmonMofData::InsertBack(
			CSysmonEventList* pEventList,
			LONG* Reference,
			void* pElm)
{
	CSysmonEventList* pNewList = pEventList->InsertBack(pEventList->m_Entry.Blink,pElm);
	(*Reference)++;
	 if ( 0x15555554 - *Reference < 1 )
	 {
		 //std::_Xlength_error("list<T> too long");
	 }

	 (*Reference)++;
	 pEventList->m_Entry.Blink = &pNewList->m_Entry;
	 pNewList->m_Entry.Blink->Flink = &pNewList->m_Entry;
}

void CSysmonDataEntry::operator =(CSysmonDataEntry* pDataEntry)
{
	if (this != pDataEntry)
	{
		m_EventClassName = pDataEntry->m_EventClassName;
		if ( pDataEntry->m_EventClassName )
			InterlockedIncrement((volatile LONG *)&m_EventClassName->m_Reference);
	}
}

void CSysmonMofData::SysmonAddProperityList(
				CSysmonEventList* pList, 
				BSTR ProperityName,
				LONG Len,
				LONG ArraySize)
{
	if ( pList)
	{
		LONG Reference = 0;
		LIST_ENTRY* pEntry = pList->m_Entry.Flink;
		while( pEntry != &pList->m_Entry)
		{
			CSysmonEventList* pEventList = CONTAINING_RECORD(pEntry,CSysmonEventList,m_Entry);

			if (pEventList)
			{
				CSysmonDataEntry* pDataEntry = (CSysmonDataEntry*)pEventList->m_pEvent;

				if (pDataEntry)
				{
					CSysmonProperty* pProperity = new CSysmonProperty(ProperityName,Len,ArraySize);
					pDataEntry->m_ProperitySize++;

					CSysmonEventList* pNewList = pDataEntry->m_ProperityList->InsertBack(
						pDataEntry->m_ProperityList->m_Entry.Blink,
						pProperity);

					pDataEntry->m_ProperityList->m_Entry.Blink = &pNewList->m_Entry;
					pNewList->m_Entry.Blink = &pNewList->m_Entry;

				}
				
			}		
			

			pEntry = pEntry->Flink;
		}

	}
}

CSysmonDataEntry* CSysmonMofData::GetPropertyList(
							IWbemClassObject* pClass,
							GUID* ClassGuid,
							USHORT Version,
							USHORT Level,
							USHORT Type)
{
	CSysmonDataEntry* pDataEntry;
	CSysmonEventList* pEventList;
	CSysmonEventList* pEventList_2;
	CSysmonDataEntry* pRetEventv63;
	CSysmonDataEntry* pOldDataEntry;
	LONG Reference;
	BSTR bstrDisplayName;
	BSTR bstrClassName;
	BSTR bstrEventType;
	BSTR bstrEventTypeName;
	BSTR bstrWmiDataId;
	BSTR bstrVarClassName;
	VARIANT varClassName;
	VARIANT varDisplayName;
	VARIANTARG vargType;
	VARIANTARG vargTypeName;
	IWbemQualifierSet * pQualifierSet = NULL;
	IEnumWbemClassObject* pClasses = NULL;
	IWbemClassObject*	  pClass_1 = NULL;
	ULONG Cnt;
	HRESULT hr;
	ULONG*		ppvData;
	BSTR*		ppvData_2;
	OLECHAR szClassName[MAX_PATH] = {0};
	OLECHAR szEventName[MAX_PATH] = {0};
	pSafeArrayDestroy ApiSafeArrayDestroy = NULL;
	pSafeArrayGetElement ApiSafeArrayGetElement = NULL;
	pEventList = new CSysmonEventList(0,0);
	pDataEntry = NULL;
	VariantInit(&varClassName);
	VariantInit(&varDisplayName);
	VariantInit(&vargType);
	VariantInit(&vargTypeName);

	//VariantInit(&v40);
	//VariantInit(&v36);
	Reference = 1;
	pRetEventv63 = NULL;
	bstrClassName = SysAllocString(L"__CLASS");
	bstrWmiDataId = SysAllocString(L"WmiDataId");
	bstrEventType = SysAllocString(L"EventType");
	bstrEventTypeName = SysAllocString(L"EventTypeName");
	bstrDisplayName = SysAllocString(L"DisplayName");

	if ( pClass->Get(
					bstrClassName,
					0,
					&varClassName, 
					0,
					0) == S_OK )
	{
		StringCchCopy(
				szClassName,
				MAX_PATH,
				varClassName.bstrVal);

		if (pQualifierSet)
		{
			pQualifierSet->Release();
			pQualifierSet = NULL;
		}

		pClass->GetQualifierSet(&pQualifierSet);
		if ( !pQualifierSet->Get(
								bstrDisplayName,
								0,
								&varDisplayName,
								0) && varDisplayName.lVal )
				StringCchCopy(szClassName, MAX_PATH, varDisplayName.bstrVal);

		pDataEntry = new CSysmonDataEntry(ClassGuid,-1,-1,-1);
		pRetEventv63 = pDataEntry;
		pOldDataEntry = pDataEntry;

		if (pDataEntry)
		{
			pDataEntry->InitData(&pDataEntry->m_EventClassName,szClassName);
			pEventList_2 = pEventList->InsertBack(
											pEventList->m_Entry.Blink,
											pDataEntry);

			pEventList->m_Entry.Blink = &pEventList_2->m_Entry;
			pEventList_2->m_Entry.Blink->Flink = &pEventList_2->m_Entry;

			bstrVarClassName = SysAllocString(varClassName.bstrVal);
			hr = m_pIWbemService->CreateClassEnum(
											bstrVarClassName,
											WBEM_FLAG_DEEP | 
											WBEM_FLAG_UPDATE_ONLY | 
											WBEM_FLAG_USE_AMENDED_QUALIFIERS,
											0,
											&pClasses);
			SysFreeString(bstrVarClassName);

			if ( hr == S_OK)
			{
				ApiSafeArrayDestroy = SafeArrayDestroy;
				ApiSafeArrayGetElement = SafeArrayGetElement;

				Cnt = 1;
				while ( TRUE )
				{
					pClass_1 = 0;
					if ( pClasses->Next(5000, 1, &pClass_1, &Cnt) != S_OK )
					{

						pClass_1 = pClass;
						pClass_1->AddRef();

					}

					if ( pQualifierSet )
					{
						pQualifierSet->Release();
						pQualifierSet = 0;
					}

					pClass_1->GetQualifierSet(&pQualifierSet);
					 VariantClear(&vargType);

					 if ( pQualifierSet->Get(
											bstrEventType,
											0,
											&vargType,
											0) == S_OK )
					 {

						 if (vargType.vt & VT_ARRAY)
						 {
							 SAFEARRAY* pl1 = vargType.parray;
							 SAFEARRAY* pl2 = NULL;
							 LONG plLbound_1 = 0;
							 LONG plUbound_1 = 0;
							 LONG plLbound_2 = 0;
							 LONG plUbound_2 = 0;
							 VariantClear(&vargTypeName);
							 if ( pQualifierSet->Get(
													bstrEventTypeName,
													0,
													&vargTypeName, 
													0) == S_OK && (vargTypeName.vt & VT_ARRAY) )
							 {
								 pl2 = vargTypeName.parray;
							 }

							 if ( !pl1
								 || SafeArrayGetLBound(pl1, 1, &plLbound_1)
								 || SafeArrayGetUBound(pl1, 1, &plUbound_1)
								 || plUbound_1 < 0 )
							 {
								 pDataEntry = pRetEventv63;
								 break;
							 }

							  SafeArrayAccessData(pl1, (void**)&ppvData);

							  if ( pl2 )
							  {
								  if ( SafeArrayGetLBound(pl2, 1, &plLbound_2) ||
									  SafeArrayGetUBound(pl2, 1, &plUbound_2) ||
									  plUbound_2 < 0 )
								  {
									  pDataEntry = pRetEventv63;
									  break;
								  }

								  SafeArrayAccessData(pl2, (void**)&ppvData_2);
							  }

							  if ( plLbound_1 <= plUbound_1)
							  {
								  LONG rgIndices = plLbound_1;

								  while(TRUE)
								  {
									  do 
									  {
										  USHORT uType = (*(USHORT*)&ppvData[rgIndices]);

										  try
										  {
											  CSysmonDataEntry* pDataEntry__3 = new CSysmonDataEntry(ClassGuid,uType,Version,Level);

											  if (pDataEntry__3)
											  {
												  InsertBack(pEventList,&Reference,pDataEntry__3);

												  if (pOldDataEntry->GetCLassNameLen())
												  {
													  *pDataEntry__3 = pOldDataEntry;
												  }

												  if (Type == uType)
												  {
													  pRetEventv63 = pDataEntry__3;
												  }

												  if ( pl2 )
												  {
													  if ( rgIndices < plLbound_2 || rgIndices > plUbound_2 )
													  {
														  break;
													  }

													  pDataEntry__3->InitData(
														  &pDataEntry__3->m_EventTypeName,
														  ppvData_2[rgIndices]);
												  }
											  }

										  }
										  catch(...)
										  {

										  }

									  } while (FALSE);
									  
									  rgIndices++;

									  if ( rgIndices > plUbound_1 )
									  {
										  pl1 = vargType.parray;
										  break;
									  }
								  }
							  }

							   SafeArrayUnaccessData(pl1);
							   ApiSafeArrayDestroy(pl1);
							   VariantInit(&vargType);

							   if ( pl2 )
							   {
								   SafeArrayUnaccessData(pl2);
								   SafeArrayDestroy(pl2);
								   VariantInit(&vargTypeName);
							   }

						 }
						 else
						 {

							 VariantChangeType(&vargType, &vargType, 0, VT_I2);
							 USHORT vType = vargType.iVal;
							 VariantClear(&vargTypeName);

							 if ( pQualifierSet->Get(
													bstrEventTypeName,
													0,
													&vargTypeName, 
													0) )
							 {
								 szEventName[0] = 0;
							 }
							 else
							 {
								 wcscpy_s(
										szEventName,
										MAX_PATH,
										vargTypeName.bstrVal
										);
							 }

							 CSysmonDataEntry* pDataEntry__3 = new CSysmonDataEntry(ClassGuid,vType,Version,Level);
							 if (pDataEntry__3)
							 {
								 InsertBack(pEventList,&Reference,pDataEntry__3);

								 if (pOldDataEntry->GetCLassNameLen())
								 {
									 *pDataEntry__3 = pOldDataEntry;
								 }

								 if ( Type == vType)
								 {
									 pRetEventv63 = pDataEntry__3;
								 }

								 pDataEntry__3->InitData(
									 &pDataEntry__3->m_EventTypeName,
									 szEventName);
							 }

						 }

					 }

					 SAFEARRAY* pNames = NULL;
					 LONG plLbound_1 = 0;
					 LONG plUbound_1 = 0;
					 LONG plLbound_2 = 0;
					 LONG plUbound_2 = 0;
					 BSTR pv_1 = NULL;
					 CIMTYPE QualifierType = 0;
					 VariantClear(&varClassName);
					 varClassName.vt = VT_I4;
					 varClassName.lVal = 1;

					 //LONG Index = 1;
					 //hr = pClass->GetNames(NULL, WBEM_FLAG_LOCAL_ONLY, NULL, &pNames);
					 hr = pClass_1->GetNames(
						 bstrWmiDataId,
						 WBEM_FLAG_ONLY_IF_IDENTICAL, 
						 &varClassName,
						 &pNames);

					 while( hr == S_OK)
					 {

						 if ( SafeArrayGetLBound(pNames, 1, &plLbound_1) ||
							 SafeArrayGetUBound(pNames, 1, &plUbound_1) || plUbound_1 < 0 )
							 break;

						 //plUbound_1 = pNames->rgsabound->cElements;

						 if ( plLbound_1 <= plUbound_1 )
						 {
							 LONG nIndex = plLbound_1;
							 do
							 {
								 if ( ApiSafeArrayGetElement(pNames, &nIndex, &pv_1) || pClass_1->Get(pv_1, 0, 0, &QualifierType, 0) )
									 break;

								 if ( pQualifierSet )
								 {
									 pQualifierSet->Release();
									 pQualifierSet = 0;
								 }

								 if ( pClass_1->GetPropertyQualifierSet(pv_1, &pQualifierSet) )
									 break;

								 LONG v30 = GetArrayValue(QualifierType, pQualifierSet);
								 LONG ArraySize = QualifierType & 0x2000 ? GetArraySize(pQualifierSet) : 1;

								 SysmonAddProperityList(
													pEventList,
													pv_1,
													v30, 
													ArraySize);
								 ++nIndex;
								
							 }
							 while ( nIndex <= plUbound_1 );
						 }

						 ApiSafeArrayDestroy(pNames);
						 pNames = 0;
						  varClassName.lVal++;// = (Index++);
					 }

					 AddEvent(pEventList);

					 if ( Cnt != 1 )
					 {
						 pDataEntry = pRetEventv63;
						 break;
					 }
				}//1。。。。。。
			}//0.。。。。。。。
		}
	}

	VariantClear(&varClassName);
	VariantClear(&varDisplayName);
	VariantClear(&vargTypeName);
	SysFreeString(bstrClassName);
	SysFreeString(bstrDisplayName);
	SysFreeString(bstrEventType);
	SysFreeString(bstrEventTypeName);
	SysFreeString(bstrWmiDataId);

	AddEvent(pEventList);

	LIST_ENTRY* pEntry = pEventList->m_Entry.Flink;

	InitializeListHead(&pEventList->m_Entry);

	while (pEntry != &pEventList->m_Entry)
	{
		CSysmonEventList* pList = CONTAINING_RECORD(
			pEntry,
			CSysmonEventList,m_Entry);

		pEntry = pEntry->Flink;

		if (pList)
		{
			delete pList;
			pList = NULL;
		}			
	}

	return pDataEntry;
}

CSysmonEventList* CSysmonMofData::AddEvent(
										CSysmonEventList* pEventList)
{
	CSysmonEventList* pRetList = pEventList;
	if (pEventList)
	{
		LIST_ENTRY* pEntry = pEventList->m_Entry.Flink;

		while(pEntry != &pEventList->m_Entry )
		{	
			CSysmonEventList* pList = CONTAINING_RECORD(pEntry,CSysmonEventList,m_Entry);
			if (pList)
			{
				CSysmonDataEntry* pDataEntry = (CSysmonDataEntry*)pList->m_pEvent;
				CSysmonEventList* pEvent = 
									m_SysmonEventList->InsertBack(
												m_SysmonEventList->m_Entry.Blink,
												pDataEntry);

				m_SysmonEventList->m_Entry.Blink = &pEvent->m_Entry;
				pEvent->m_Entry.Blink->Flink = &pEvent->m_Entry;

				m_EventListNums++;
			}

			pEntry = pEntry->Flink;
		}

		pEntry = pEventList->m_Entry.Flink;

		InitializeListHead(&pEventList->m_Entry);
		
		while (pEntry != &pEventList->m_Entry)
		{
			CSysmonEventList* pList = CONTAINING_RECORD(
												pEntry,
												CSysmonEventList,m_Entry);

			pRetList = pList;
			pEntry = pEntry->Flink;

			if (pList)
			{
				delete pList;
				pList = NULL;
			}			
		}

		return pRetList;
	}

	return NULL;
}

LONG CSysmonMofData::GetArrayValue(
							CIMTYPE CimType,
							IWbemQualifierSet * pQualifierSet)
{
	LONG RetValue = 29;
	BSTR bstrQualifier;
	VARIANT varQualifier;
	HRESULT hr;
	BOOL bPointer;
	OLECHAR szFormat[11];
	OLECHAR szStringTermination[31];
	OLECHAR szExtension[31];
	bPointer = 0;

	szFormat[0] = 0;
	szStringTermination[0] = 0;
	szExtension[0] = 0;

	if (pQualifierSet)
	{
		bstrQualifier = SysAllocString(L"Format");
		VariantInit(&varQualifier);
		hr = pQualifierSet->Get(bstrQualifier, 0, &varQualifier, 0);
		SysFreeString(bstrQualifier);

		if ( !hr && varQualifier.lVal )
			StringCchCopy(szFormat, 10, varQualifier.bstrVal);

		bstrQualifier = SysAllocString(L"StringTermination");
		VariantClear(&varQualifier);

		hr = pQualifierSet->Get(bstrQualifier, 0, &varQualifier, 0);
		SysFreeString(bstrQualifier);

		if ( !hr && varQualifier.lVal )
			StringCchCopy(szStringTermination, 30, varQualifier.bstrVal);

		bstrQualifier = SysAllocString(L"Pointer");
		VariantClear(&varQualifier);

		hr = pQualifierSet->Get(bstrQualifier, 0, &varQualifier, 0);
		SysFreeString(bstrQualifier);

		if ( hr == S_OK)
		{
			bPointer = 1;
		}

		bstrQualifier = SysAllocString(L"Extension");
		VariantClear(&varQualifier);

		hr = pQualifierSet->Get(bstrQualifier, 0, &varQualifier, 0);
		SysFreeString(bstrQualifier);

		if ( !hr && varQualifier.lVal )
			StringCchCopy(szExtension, 30, varQualifier.bstrVal);

		VariantClear(&varQualifier);

		 switch ( CimType & (~CIM_FLAG_ARRAY) )
		 {
		 case CIM_SINT16:
			 RetValue = 4;
			 break;
		 case CIM_SINT32:
			 RetValue = 6;
			 break;
		 case CIM_REAL32:
			 RetValue = 11;
			 break;
		 case CIM_REAL64:
			 RetValue = 12;
			 break;
		 case CIM_STRING:
			 {
				 if ( _wcsicmp(szStringTermination, L"NullTerminated") )
				 {
					 if ( _wcsicmp(szStringTermination, L"Counted") )
					 {
						 if ( _wcsicmp(szStringTermination, L"ReverseCounted") )
							 RetValue = _wcsicmp(szStringTermination, L"NotCounted") != 0 ? 13 : 23;
						 else
							 RetValue = 18 - (_wcsicmp(szFormat, L"w") != 0);
					 }
					 else
					 {
						 RetValue = 16 - (_wcsicmp(szFormat, L"w") != 0);
					 }
				 }
				 else
				 {
					 RetValue = 14 - (_wcsicmp(szFormat, L"w") != 0);
				 }
			 }
			 break;
		 case CIM_BOOLEAN:
			 RetValue = 26;
			 break;
		 case CIM_OBJECT:
			 if ( !_wcsicmp(szExtension, L"Port") )
			 {
				 RetValue = 21;
				
			 }
			 else
			 {
				 if ( !_wcsicmp(szExtension, L"IPAddr") )
				 {
					 RetValue = 20;
				 }
				 else if ( _wcsicmp(szExtension, L"Sid") )
				 {
					 if ( _wcsicmp(szExtension, L"Guid") )
					 {
						 if ( !_wcsicmp(szExtension, L"SizeT") )
						 {
							 RetValue = 6;
						 }
						 else
						 {
							 if ( _wcsicmp(szExtension, L"IPAddrV6") )
							 {
								 if ( _wcsicmp(szExtension, L"IPAddrV4") )
								 {
									 if ( !_wcsicmp(szExtension, L"WmiTime") )
										 RetValue = 28;
								 }
								 else
								 {
									 RetValue = 20;
								 }
							 }
							 else
							 {
								 RetValue = 27;
							 }
						 }
						
					 }
					 else
					 {
						 RetValue = 25;
					 }
				 }
				 else
				 {
					 RetValue = 19;
				 }
			 }
			 break;
		 case CIM_SINT8:
			 RetValue = 3;
			 if ( !_wcsicmp(szFormat, L"c") )
				 RetValue = 0;
			 break;
		 case CIM_UINT8:
			 RetValue = 2;
			 break;
		 case CIM_UINT16:
			 RetValue = 5;
			 break;
		 case CIM_UINT32:
			 RetValue = 7;
			 break;
		 case CIM_SINT64:
			 RetValue = 9;
			 break;
		 case CIM_UINT64:
			 RetValue = 10;
			 break;
		 case CIM_CHAR16:
			 RetValue = 1;
			 break;
		 }

		 if (bPointer)
		 {
			 RetValue = 24;
		 }
		 
	}

	return RetValue;
}

LONG CSysmonMofData::GetArraySize(
							IWbemQualifierSet* pQualifierSet)
{
	LONG ArraySize = 1;
	VARIANT varQualifier;
	HRESULT hr;
	if ( pQualifierSet )
	{
		BSTR bstrMax = SysAllocString(L"MAX");
		VariantInit(&varQualifier);

		hr = pQualifierSet->Get(bstrMax, 0, &varQualifier, 0);
		SysFreeString(bstrMax);
		if ( !hr && varQualifier.vt == 3 )
			ArraySize = varQualifier.lVal;

		VariantClear(&varQualifier);
	}

	return ArraySize;
}

CSysmonData::CSysmonData()
	:m_pData(NULL)
	,m_pAlloc(NULL)
	,m_Reference(0) 
{

}

CSysmonData::~CSysmonData()
{

}

BOOL CSysmonData::Compare(BSTR pString)
{
	if (pString && m_pData)
	{
		return _wcsicmp(m_pData,pString) == 0;
	}
	return FALSE;
}

BOOL CSysmonData::CompareN(BSTR pString,ULONG Size)
{
	if (pString && m_pData)
	{
		return _wcsnicmp(m_pData,pString,Size) == 0;
	}
	return FALSE;
}

CSysmonDataEntry::CSysmonDataEntry()
	:m_EventClassName(NULL)
	,m_EventTypeName(NULL)
	,m_ProperitySize(0)
{

}

CSysmonDataEntry::CSysmonDataEntry(
								GUID* Guid,
								USHORT Type,
								USHORT Version,
								USHORT Level)
{
	m_EventClassName = NULL;
	m_EventTypeName = NULL;

	m_EventType = Type;
	m_EventVersion = Version;
	m_EventLevel = Level;
	m_ProperitySize = 0;
	m_ProperityList = new CSysmonEventList(0,0);
	
	_mm_storeu_si128(
				(__m128i *)&m_EventGuid,
				_mm_loadu_si128((const __m128i *)Guid));
}

CSysmonDataEntry::~CSysmonDataEntry()
{

}

LONG CSysmonDataEntry::GetCLassNameLen()
{
	if ( m_EventClassName && m_EventClassName->m_pData)
	{
		return SysStringLen(m_EventClassName->m_pData);
	}

	return 0;
}

CSysmonDataEntry* CSysmonDataEntry::InitData(CSysmonData** pData,BSTR pString)
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

				if ( m_EventClassName->m_pAlloc )
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

	return this;
}


CSysmonEventList::CSysmonEventList()
	:m_pEvent(0)
{
	InitializeListHead(&m_Entry);
}

CSysmonEventList::CSysmonEventList(
							LIST_ENTRY* Fink,
							LIST_ENTRY* Blink)
							:m_pEvent(0)
{
	LIST_ENTRY* pl1;
	LIST_ENTRY* pl2;

	pl1 = Fink;
	m_pEvent = 0;
	if (Fink)
	{
		pl2 = Blink;
	}
	else
	{
		pl1 = &m_Entry;
		pl2 = &m_Entry;
	}

	m_Entry.Flink = pl1;
	m_Entry.Blink = pl2;
}

CSysmonEventList::~CSysmonEventList()
{

}

CSysmonEventList* CSysmonEventList::InsertBack(LIST_ENTRY* Blink,void* pElm)
{
	CSysmonEventList* pList = new CSysmonEventList(
												&m_Entry,
												Blink);
	if (pList)
	{
		pList->m_pEvent = pElm;
	}

	return pList;
}