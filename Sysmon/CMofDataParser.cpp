#include "StdAfx.h"
#include "CMofDataParser.h"
#include "Sysmon.h"
#include <WinSock2.h>
#include <in6addr.h>

#define SeLengthSid( Sid ) (8 +(4*((SID*)Sid)->SubAuthorityCount))

#pragma comment (lib,"Ws2_32.lib")
CMofDataParser::CMofDataParser(void)
	:m_pServices(NULL)
{
	InitializeListHead(&m_ModDataList);
}

CMofDataParser::~CMofDataParser(void)
{
	
}

CMofDataParser* CMofDataParser::Instance()
{


	if ( !m_pServices)
	{
		if(FAILED(Connect(
			BSTR(_T("root\\wmi")))))
		{
			return NULL;
		}
	}

	return this;
}

HRESULT CMofDataParser::Connect(
	BSTR bstrNamespace
	)
{
	HRESULT hr = S_OK;
	IWbemLocator* pLocator = NULL;

	hr = CoInitialize(0);

	hr = CoCreateInstance(__uuidof(WbemLocator),
		0,
		CLSCTX_INPROC_SERVER,
		__uuidof(IWbemLocator),
		(LPVOID*) &pLocator);

	if (FAILED(hr))
	{
		goto cleanup;
	}

	hr = pLocator->ConnectServer(bstrNamespace,
		NULL,
		NULL,
		NULL,
		0L,
		NULL,
		NULL,
		&m_pServices);

	if (FAILED(hr))
	{
		goto cleanup;
	}

	hr = CoSetProxyBlanket(m_pServices,
		RPC_C_AUTHN_WINNT,
		RPC_C_AUTHZ_NONE,
		NULL,
		RPC_C_AUTHN_LEVEL_PKT, 
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, 
		EOAC_NONE);

	if (FAILED(hr))
	{
		m_pServices->Release();
		m_pServices = NULL;
	}

cleanup:

	if (pLocator)
		pLocator->Release();

	return hr;
}

BOOL CMofDataParser::Parse( 
						PEVENT_TRACE pEvent,
						CMofParseRes& Result )
{
	TCHAR ClassGuid[50] = {0};
	SYSTEMTIME st = {0};
	
	FILETIME ft = {0};
	IWbemClassObject* pEventCategoryClass = NULL;
	CMofParseRes* pEventRes = NULL;
	PROPERTY_LIST* pProperties = NULL;
	DWORD PropertyCount = 0;
	LONG pPropertyIndex[30] = {0};
	PBYTE pEventData = NULL;  
	PBYTE pEndOfEventData = NULL;

	if (!Instance())
	{
		return NULL;
	}

	StringFromGUID2(
				pEvent->Header.Guid,
				ClassGuid, 
				sizeof(ClassGuid));
	
	ft.dwHighDateTime = pEvent->Header.TimeStamp.HighPart;
	ft.dwLowDateTime = pEvent->Header.TimeStamp.LowPart;

	if (pEvent->MofLength <= 0)
	{
		return FALSE;
	}


	for ( LIST_ENTRY* pEntry = m_ModDataList.Flink;
		pEntry != &m_ModDataList;
		pEntry = pEntry->Flink)
	{
		CMofParseRes* pRes = CONTAINING_RECORD(
											pEntry,
											CMofParseRes,
											m_Entry);

		if (pRes)
		{
			if (pRes)
			{
				if (pRes->m_MofHeader.MofVsersion == pEvent->Header.Class.Version &&
					pRes->m_MofHeader.MofType == pEvent->Header.Class.Type)
				{
					if (pEvent->Header.Guid.Data1 == pRes->m_MofHeader.MofGuid.Data1 &&
						pEvent->Header.Guid.Data2 == pRes->m_MofHeader.MofGuid.Data2 &&
						pEvent->Header.Guid.Data3 == pRes->m_MofHeader.MofGuid.Data3 &&
						pEvent->Header.Guid.Data4[0] == pRes->m_MofHeader.MofGuid.Data4[0] &&
						pEvent->Header.Guid.Data4[1] == pRes->m_MofHeader.MofGuid.Data4[1] &&
						pEvent->Header.Guid.Data4[2] == pRes->m_MofHeader.MofGuid.Data4[2] &&
						pEvent->Header.Guid.Data4[3] == pRes->m_MofHeader.MofGuid.Data4[3] &&
						pEvent->Header.Guid.Data4[4] == pRes->m_MofHeader.MofGuid.Data4[4] &&
						pEvent->Header.Guid.Data4[5] == pRes->m_MofHeader.MofGuid.Data4[5] &&
						pEvent->Header.Guid.Data4[6] == pRes->m_MofHeader.MofGuid.Data4[6] &&
						pEvent->Header.Guid.Data4[7] == pRes->m_MofHeader.MofGuid.Data4[7])
					{
						Result = *pRes;
						return TRUE;
					}
				}
			}
		}
	}

	FileTimeToSystemTime(
		&ft,
		&st);
	SystemTimeToTzSpecificLocalTime(
		NULL,
		&st,
		&Result.m_MofHeader.MofLocal);


	pEventCategoryClass = GetEventCategoryClass(
									BSTR(ClassGuid),
									pEvent->Header.Class.Version);
	if (pEventCategoryClass)
	{

		pEventRes = GetEventClassPropertyList(
						pEventCategoryClass,
						pEvent->Header.Guid,
						pEvent->Header.Class.Version,
						pEvent->Header.Class.Level,
						pEvent->Header.Class.Type);

		pEventCategoryClass->Release();
		pEventCategoryClass = NULL;

		if (pEventRes)
		{
			Result = *pEventRes;
// 			if (GetPropertyList(
// 						pEventClass,
// 						&pProperties,
// 						&PropertyCount,
// 						pPropertyIndex ))
// 			{
// 				
// 				pEventData = (PBYTE)(pEvent->MofData);
// 				pEndOfEventData = ((PBYTE)(pEvent->MofData) + pEvent->MofLength);
// 
// 				if ( PropertyCount )
// 				{
// 					Result.m_MofProperty = new MOF_ITEM[PropertyCount];
// 					if (Result.m_MofProperty)
// 					{
// 						ZeroMemory(
// 							Result.m_MofProperty,
// 							sizeof(MOF_ITEM) * 
// 							PropertyCount);
// 					}
// 				}
// 
// 				if ( Result.m_MofProperty )
// 				{
// 					for (LONG i = 0; (DWORD)i < PropertyCount; i++)
// 					{
// 
// 						if ( !(0 <= pPropertyIndex[i] &&
// 							pPropertyIndex[i] <= PropertyCount ))
// 						{
// 							break;
// 						}
// 
// 						GetPropertyName( 
// 								&pProperties[pPropertyIndex[i]],
// 								Result.m_MofProperty[i].Name);
// 
// 						pEventData =  GetPropertyValue(
// 											&pProperties[pPropertyIndex[i]], 
// 											*(Result.GetItem(i)),
// 											pEventData, 
// 											(USHORT)(pEndOfEventData - pEventData));
// 
// 						if (NULL == pEventData)
// 						{
// 							break;
// 						}
// 
// 						Result.m_MofHeader.MofCount = (i + 1);
// 					}
// 				}				
// 
// 				FreePropertyList(
// 							pProperties,
// 							PropertyCount);
// 			}

			return TRUE;
		}
	}

	return FALSE;
}


IWbemClassObject* 
CMofDataParser::GetEventCategoryClass(
								BSTR bstrClassGuid,
								int Version)
{
	HRESULT hr = S_OK;
	HRESULT hrQualifier = S_OK;
	IEnumWbemClassObject* pClasses = NULL;
	IEnumWbemClassObject* pClasses_2 = NULL;
	IWbemClassObject* pClass = NULL;
	IWbemClassObject* pClass_2 = NULL;
	IWbemQualifierSet* pQualifiers = NULL;
	ULONG cnt = 0;
	ULONG cnt_2 = 0;
	VARIANT varGuid;
	VARIANT varVersion; 

	VariantInit(&varGuid);
	VariantInit(&varVersion);

	hr = m_pServices->CreateClassEnum(
						BSTR(_T("EventTrace")), 
						WBEM_FLAG_DEEP |
						WBEM_FLAG_FORWARD_ONLY |
						WBEM_FLAG_USE_AMENDED_QUALIFIERS,
						NULL,
						&pClasses);

	if (FAILED(hr))
	{
		goto cleanup;
	}

	__try
	{
		while (S_OK == hr)
		{
			if (pClass)
			{
				pClass->Release();
				pClass = NULL;
			}

			hr = pClasses->Next(WBEM_INFINITE, 1, &pClass, &cnt);

			if (FAILED(hr) || cnt != 1)
			{
				goto cleanup;
			}

			VARIANT varCLassName;
			VariantInit(&varCLassName);
			if (FAILED(pClass->Get(BSTR(_T("__CLASS")),0,&varCLassName,0,0)))
			{
				continue;
			}

			m_pServices->CreateClassEnum(
				varCLassName.bstrVal,
				WBEM_FLAG_DEEP | 
				WBEM_FLAG_UPDATE_ONLY | 
				WBEM_FLAG_USE_AMENDED_QUALIFIERS,
				0,
				&pClasses_2);

			VariantClear(&varCLassName);

			cnt_2 = 1;

			do 
			{
				
				if (pClasses_2)
				{
					if ( pClasses_2->Next(5000, 1, &pClass_2, &cnt_2) != S_OK )
						continue;

					if ( cnt_2 != 1 )
						break;			
				}
				else
				{
					pClass_2 = pClass;
					cnt_2 = 1;
				}

				HRESULT hr2 = pClass_2->Get(BSTR(_T("__CLASS")),0,&varCLassName,0,0);
				VariantClear(&varCLassName);

				if (SUCCEEDED(hr2))
				{
					if ( pQualifiers )
					{
						pQualifiers->Release();
						pQualifiers = 0;
					}

					hrQualifier = pClass_2->GetQualifierSet(&pQualifiers);

					if (pQualifiers)
					{
						hrQualifier = pQualifiers->Get(L"Guid", 0, &varGuid, NULL);

						if (SUCCEEDED(hrQualifier))
						{
							if (_wcsicmp(varGuid.bstrVal, bstrClassGuid) == 0)
							{

								hrQualifier = pQualifiers->Get(L"EventVersion", 0, &varVersion, NULL);

								if (SUCCEEDED(hrQualifier))
								{
									VariantChangeType(&varVersion, &varVersion, 0, VT_I2);
									SHORT Ver = varVersion.iVal;
									VariantClear(&varVersion);
									if (Version == Ver)
									{
										__leave; //found class
									}

									VariantClear(&varVersion);
								}
								else if (WBEM_E_NOT_FOUND == hrQualifier) 
								{
									__leave; //found class
								}
							}

							VariantClear(&varGuid);
						}

						pQualifiers->Release();
						pQualifiers = NULL;
					}
				}

				if (pClass_2)
				{
					pClass_2->Release();
					pClass_2 = NULL;
				}

			} while (cnt_2 == 1);

			if (pClasses_2)
			{
				pClasses_2->Release();
				pClasses_2 = NULL;
			}		
		} 
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{

	}

cleanup:

	if (pClasses_2)
	{
		pClasses_2->Release();
		pClasses_2 = NULL;
	}

	if (pClasses)
	{
		pClasses->Release();
		pClasses = NULL;
	}

	if (pQualifiers)
	{
		pQualifiers->Release();
		pQualifiers = NULL;
	}

	if (pClass)
	{
		pClass->Release();
		pClass = NULL;
	}

	VariantClear(&varVersion);
	VariantClear(&varGuid);

	return pClass_2;
}

CMofParseRes* 
CMofDataParser::GetEventClassPropertyList(
						IWbemClassObject* pEventCategoryClass,
						GUID& Guid,
						USHORT Version,
						USHORT Level,
						USHORT EventType)
{
	HRESULT hr = S_OK;
	HRESULT hrQualifier = S_OK;
	IEnumWbemClassObject* pClasses = NULL;
	IWbemClassObject* pClass = NULL;
	IWbemQualifierSet* pQualifiers = NULL;
	BOOL FoundEventClass = FALSE;
	CMofParseRes* pMofParseRes = NULL;
	OLECHAR szClassName[100] = {0};
	ULONG cnt = 0;
	VARIANT varClassName;
	VARIANT varEventType;
	VARIANT	varDisplayName;
	
	LIST_ENTRY pList;

	InitializeListHead(&pList);
	VariantInit(&varClassName);
	VariantInit(&varEventType);
	VariantInit(&varDisplayName);

	hr = pEventCategoryClass->Get(_T("__CLASS"), 0, &varClassName, NULL, NULL);

	if (FAILED(hr))
	{
		goto cleanup;
	}

	StringCchCopy(
				szClassName,
				100,
				varClassName.bstrVal);

	if (pQualifiers)
	{
		pQualifiers->Release();
		pQualifiers = NULL;
	}

	pEventCategoryClass->GetQualifierSet(&pQualifiers);

	if (pQualifiers && !pQualifiers->Get(
		BSTR(_T("DisplayName")),
		0,
		&varDisplayName,
		0) && varDisplayName.lVal )
	{
		StringCchCopy(
				szClassName,
				MAX_PATH,
				varDisplayName.bstrVal);
	}

	hr = m_pServices->CreateClassEnum(varClassName.bstrVal, 
		WBEM_FLAG_DEEP | 
		WBEM_FLAG_UPDATE_ONLY | 
		WBEM_FLAG_USE_AMENDED_QUALIFIERS,
		NULL, 
		&pClasses);

	if (FAILED(hr))
	{		
		goto cleanup;
	}

	while (S_OK == hr)
	{
		hr = pClasses->Next(WBEM_INFINITE, 1, &pClass, &cnt);

		if (hr != S_OK)
		{
			goto cleanup;
		}

		if (cnt != 1)
		{
			break;
		}

		if (pQualifiers)
		{
			pQualifiers->Release();
			pQualifiers = NULL;
		}

		hrQualifier = pClass->GetQualifierSet(&pQualifiers);

		if (FAILED(hrQualifier))
		{			
			pClass->Release();
			pClass = NULL;
			goto cleanup;
		}

		hrQualifier = pQualifiers->Get(L"EventType", 0, &varEventType, NULL);

		if (FAILED(hrQualifier))
		{
			
			pClass->Release();
			pClass = NULL;
			goto cleanup;
		}

		// If multiple events provide the same data, the EventType qualifier
		// will contain an array of types. Loop through the array and find a match.

		if (varEventType.vt & VT_ARRAY)
		{
			HRESULT hrSafe = S_OK;
			
			SAFEARRAY* pEventTypes = varEventType.parray;
			VARIANT vargTypeName;
			VariantInit(&vargTypeName);
			if ( pQualifiers->Get(
				BSTR(_T("EventTypeName")),
				0,
				&vargTypeName, 
				0) == S_OK &&
				vargTypeName.vt & VT_ARRAY )
			{
				SAFEARRAY* pEventNames = vargTypeName.parray;
				BSTR* ppvData = NULL;
				

				if (pEventNames && pEventTypes &&
					pEventNames->rgsabound->cElements &&
					pEventTypes->rgsabound->cElements &&
					pEventTypes->rgsabound->cElements ==
					pEventNames->rgsabound->cElements)
				{
					SafeArrayAccessData(
									pEventNames,
									(void**)&ppvData);

					for (LONG i = 0; (ULONG)i < pEventTypes->rgsabound->cElements; i++)
					{
						int ClassEventType = 0;
						hrSafe = SafeArrayGetElement(pEventTypes, &i, &ClassEventType);

						CMofParseRes* pMofParseRes2 = (CMofParseRes*)malloc(sizeof(CMofParseRes));

						if (pMofParseRes2)
						{
							ZeroMemory(pMofParseRes2,sizeof(CMofParseRes));

							InitializeListHead(&pMofParseRes2->m_MofPropertyList);
							pMofParseRes2->m_MofHeader.MofClassName = SysAllocString(szClassName);
							memcpy(
								&pMofParseRes2->m_MofHeader.MofGuid,
								&Guid,			 
								sizeof(GUID));

							pMofParseRes2->m_MofHeader.MofType = ClassEventType;
							pMofParseRes2->m_MofHeader.MofLevel = Level;
							pMofParseRes2->m_MofHeader.MofVsersion = Version;
							pMofParseRes2->m_MofHeader.MofTypeName = SysAllocString(ppvData[i]);
							InsertTailList(&pList,&pMofParseRes2->m_Entry);

							if (ClassEventType == EventType)
							{
								pMofParseRes = pMofParseRes2;
							}
						}						
					}
				}

			}

			VariantClear(&vargTypeName);
		}
		else
		{
			VARIANT vargTypeName;
			VariantInit(&vargTypeName);
			if ( pQualifiers->Get(
				BSTR(_T("EventTypeName")),
				0,
				&vargTypeName, 
				0) == S_OK )
			{
				VariantChangeType(&varEventType, &varEventType, 0, VT_I2);
				USHORT vType = varEventType.iVal;

				CMofParseRes* pMofParseRes2 = (CMofParseRes*)malloc(sizeof(CMofParseRes));

				if (pMofParseRes2)
				{
					ZeroMemory(pMofParseRes2,sizeof(CMofParseRes));
					InitializeListHead(&pMofParseRes2->m_MofPropertyList);
					pMofParseRes2->m_MofHeader.MofClassName = SysAllocString(szClassName);
					memcpy(
						&pMofParseRes2->m_MofHeader.MofGuid,
						&Guid,			 
						sizeof(GUID));

					pMofParseRes2->m_MofHeader.MofType = vType;
					pMofParseRes2->m_MofHeader.MofLevel = Level;
					pMofParseRes2->m_MofHeader.MofVsersion = Version;
					pMofParseRes2->m_MofHeader.MofTypeName = SysAllocString(vargTypeName.bstrVal);

					InsertTailList(&pList,&pMofParseRes2->m_Entry);

					if (vType == EventType)
					{
						//FoundEventClass = TRUE;
						pMofParseRes = pMofParseRes2;
					}
				}
			}
			VariantClear(&vargTypeName);

			
		}
		VariantClear(&varEventType);

		SAFEARRAY* pNames = NULL;
		VARIANT QualifierVal;
		VariantInit(&QualifierVal);
		QualifierVal.vt = VT_I4;
		QualifierVal.lVal = 1;
		LONG rgIndices = 0;	

		while(TRUE)
		{
			hr = pClass->GetNames(
				(BSTR)_T("WmiDataId"),
				WBEM_FLAG_ONLY_IF_IDENTICAL,
				&QualifierVal,
				&pNames);

			if (hr != S_OK)
			{
				break;
			}

			if (!pNames->rgsabound->cElements)
			{
				break;
			}

			rgIndices = 0;
			do 
			{
				BSTR pvElement = NULL;
				CIMTYPE QualifierType = 0;
				if ( SafeArrayGetElement(pNames, &rgIndices, &pvElement) ||
					pClass->Get(pvElement, 0, 0, &QualifierType, 0))
				{
					break;
				}

				if ( pQualifiers )
				{
					pQualifiers->Release();
					pQualifiers = 0;
				}

				if(pClass->GetPropertyQualifierSet(pvElement,&pQualifiers))
				{
					break;
				}

				LONG dwLen = GetArrayValue(QualifierType,pQualifiers);
				LONG ArraySize = QualifierType & VT_ARRAY ? GetArraySize(pQualifiers) : 1;

				AddProperityList(
								&pList,
								pvElement,
								dwLen,
								ArraySize);
				rgIndices++;

			} while(rgIndices < pNames->rgsabound->cElements);

			QualifierVal.lVal++;
			SafeArrayDestroy(pNames);
			pNames = 0;
		}		
		
		pClass->Release();
		pClass = NULL;

		AddGlobalEvent(&pList);
	}

cleanup:

	if (pClass)
	{
		pClass->Release();
		pClass = NULL;
	}

	if (pClasses)
	{
		pClasses->Release();
		pClasses = NULL;
	}

	if (pQualifiers)
	{
		pQualifiers->Release();
		pQualifiers = NULL;
	}
	
	AddGlobalEvent(&pList);

	VariantClear(&varClassName);
	VariantClear(&varEventType);
	VariantClear(&varDisplayName);

	return pMofParseRes;
}


BOOL CMofDataParser::GetPropertyList(
						IWbemClassObject* pClass, 
						PROPERTY_LIST** ppProperties,
						DWORD* pPropertyCount,
						LONG* PropertyIndex)
{
	HRESULT hr = S_OK;
	SAFEARRAY* pNames = NULL;
	LONG j = 0;
	VARIANT var;

	// Retrieve the property names.

	hr = pClass->GetNames(
						NULL,
						WBEM_FLAG_LOCAL_ONLY,
						NULL,
						&pNames);
	if (pNames)
	{
		*pPropertyCount = pNames->rgsabound->cElements;

		
		*ppProperties = new PROPERTY_LIST[*pPropertyCount];	

		if ( NULL == *ppProperties)
		{
			hr = E_OUTOFMEMORY;
			goto cleanup;
		}

		ZeroMemory(
			*ppProperties,
			sizeof(PROPERTY_LIST) * (*pPropertyCount)
					);

		for (LONG i = 0; (ULONG)i < *pPropertyCount; i++)
		{
			PROPERTY_LIST* pCurProperList = &(*ppProperties)[i];
			
			__try
			{
				hr = SafeArrayGetElement(
					pNames, 
					&i,
					&(pCurProperList->Name));

				if (FAILED(hr))
				{
					goto cleanup;
				} 

				//Save the qualifiers. Used latter to help determine how to read the event data.

				hr = pClass->GetPropertyQualifierSet(
					pCurProperList->Name,
					&(pCurProperList->pQualifiers));
				if (FAILED(hr))
				{
					goto cleanup;
				} 

				hr = pCurProperList->pQualifiers->Get(
					L"WmiDataId",
					0,
					&var,
					NULL);
				if (SUCCEEDED(hr))
				{
					j = var.intVal - 1;
					VariantClear(&var);
					*(PropertyIndex + j ) = i;

				}
				else
				{
					goto cleanup;
				}

				hr = pClass->Get(
					pCurProperList->Name,
					0,
					NULL,
					&(pCurProperList->CimType),
					NULL);

				if (FAILED(hr))
				{
					goto cleanup;
				} 
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{

			}
		}
	}

cleanup:

	if (pNames)
	{
		SafeArrayDestroy(pNames);
	}

	if (FAILED(hr))
	{
		if (*ppProperties)
		{
			FreePropertyList(
						*ppProperties,
						*pPropertyCount);
		}

		return FALSE;
	}

	return TRUE;
}


void CMofDataParser::FreePropertyList(
				PROPERTY_LIST* pProperties,
				DWORD Count)
{
	if(pProperties)
	{
		for (DWORD i=0; i < Count; i++)
		{
			SysFreeString((pProperties+i)->Name);

			if ((pProperties+i)->pQualifiers)
			{
				(pProperties+i)->pQualifiers->Release();
				(pProperties+i)->pQualifiers = NULL;
			}
		}

		delete[] pProperties;
		pProperties = NULL;
	}

}

void CMofDataParser::GetPropertyName(
						PROPERTY_LIST* pProperty,
						TCHAR* PropertyName)
{
	HRESULT hr;
	VARIANT varDisplayName;
	VariantInit(&varDisplayName);
	__try
	{
		hr = pProperty->pQualifiers->Get(
										L"DisplayName",
										0, 
										&varDisplayName,
										NULL);

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		hr = -1;
	}
	

	
	if (SUCCEEDED(hr))
	{
		PropertyName = SysAllocString(varDisplayName.bstrVal);
	}
	else
	{
		PropertyName = SysAllocString(pProperty->Name);
	}
	
	VariantClear(&varDisplayName);
}


PBYTE CMofDataParser::GetPropertyValue(
								PROPERTY_LIST*	pProperty,
								MOF_ITEM&		MofItem,
								PBYTE			pEventData,
								USHORT			RemainingBytes)
{
	HRESULT hr;
	VARIANT varQualifier;
	ULONG ArraySize = 1;
	BOOL PrintAsChar = FALSE;
	BOOL PrintAsHex = FALSE;
	BOOL PrintAsIPAddress = FALSE; 
	BOOL PrintAsPort = FALSE; 
	BOOL IsWideString = FALSE;
	BOOL IsNullTerminated = FALSE;
	USHORT StringLength = 0;
	MofItem.ArraySize = 0;
	MofItem.cValues = 0;
	MofItem.liValue.QuadPart = 0;

	if (SUCCEEDED(hr = pProperty->pQualifiers->Get(
												L"Pointer",
												0, 
												NULL,
												NULL)) ||
		SUCCEEDED(hr = pProperty->pQualifiers->Get(
												L"PointerType",
												0,
												NULL,
												NULL)))
	{
		if (m_PointerSize == 4) 
		{
			ULONG temp = 0;

			CopyMemory(
					&temp,
					pEventData,
					sizeof(ULONG));

			MofItem.Types = OTHER_VALUE;
			MofItem.liValue.LowPart = temp; 
		}
		else
		{
			ULONGLONG temp = 0;

			CopyMemory(
					&temp,
					pEventData,
					sizeof(ULONGLONG));

			
			MofItem.Types = OTHER_VALUE;
			MofItem.liValue.QuadPart = temp;
		}

		pEventData += m_PointerSize;

		return pEventData;
	}
	else
	{
		
		if (pProperty->CimType & CIM_FLAG_ARRAY)
		{
			hr = pProperty->pQualifiers->Get(
											L"MAX",
											0,
											&varQualifier,
											NULL);
			if (SUCCEEDED(hr))
			{
				ArraySize = varQualifier.intVal;
				VariantClear(&varQualifier);
			}
			else
			{
				//wprintf(L"Failed to retrieve the MAX qualifier. Terminating.\n");
				return NULL;
			}
		}

		// The CimType is the data type of the property.

		switch(pProperty->CimType & (~CIM_FLAG_ARRAY))
		{
		case CIM_SINT32:
			{
				LONG temp = 0;
				MofItem.u32ArrayValue = new uint32[ArraySize];

				if ( MofItem.u32ArrayValue)
				{
					for ( ULONG i = 0 ; i < ArraySize ;  i++)
					{
						CopyMemory(
							&temp,
							pEventData,
							sizeof(LONG));

						MofItem.u32ArrayValue[i] = temp;
						
						pEventData += sizeof(LONG);
					}

					MofItem.Types = UINT32_ARRAY_VALUE;
				}
				else
				{
					return NULL;
				}

				
				return pEventData;
			}

		case CIM_UINT32:
			{
				ULONG temp = 0;

				hr = pProperty->pQualifiers->Get(
												L"Extension",
												0,
												&varQualifier,
												NULL);
				if (SUCCEEDED(hr))
				{
					
					if (_wcsicmp(L"IPAddr", varQualifier.bstrVal) == 0)
					{
						PrintAsIPAddress = TRUE;
					}

					VariantClear(&varQualifier);
				}
				else
				{
					hr = pProperty->pQualifiers->Get(
													L"Format",
													0,
													NULL,
													NULL);
					if (SUCCEEDED(hr))
					{
						PrintAsHex = TRUE;
					}
				}

				MofItem.u32ArrayValue = new uint32[ArraySize];

				if ( MofItem.u32ArrayValue)
				{
					for (ULONG i = 0; i < ArraySize; i++)
					{
						CopyMemory(&temp, pEventData, sizeof(ULONG));

						if (PrintAsIPAddress)
						{
// 							wprintf(L"%03d.%03d.%03d.%03d\n", (temp >>  0) & 0xff,
// 								(temp >>  8) & 0xff,
// 								(temp >>  16) & 0xff,
// 								(temp >>  24) & 0xff);

							MofItem.u32ArrayValue[i] = temp;
						}
						else if (PrintAsHex)
						{
							//wprintf(L"0x%x\n", temp);
							MofItem.u32ArrayValue[i] = temp;
						}
						else
						{
							//wprintf(L"%lu\n", temp);
							MofItem.u32ArrayValue[i] = temp;
						}

						pEventData += sizeof(ULONG);
					}


					MofItem.Types = UINT32_ARRAY_VALUE;
				}
				else
				{
					return NULL;
				}

				return pEventData;
			}

		case CIM_SINT64:
			{
				LONGLONG temp = 0;

				for (ULONG i=0; i < ArraySize; i++)
				{
					CopyMemory(
							&temp,
							pEventData,
							sizeof(LONGLONG));
					//wprintf(L"%I64d\n", temp);
					MofItem.liValue.QuadPart = temp;
					MofItem.Types = OTHER_VALUE;
					pEventData += sizeof(LONGLONG);
				}

				return pEventData;
			}

		case CIM_UINT64:
			{
				ULONGLONG temp = 0;

				for (ULONG i=0; i < ArraySize; i++)
				{
					CopyMemory(&temp, pEventData, sizeof(ULONGLONG));
					//wprintf(L"%I64u\n", temp);
					MofItem.liValue.QuadPart = temp;
					MofItem.Types = OTHER_VALUE;

					pEventData += sizeof(ULONGLONG);
				}

				return pEventData;
			}

		case CIM_STRING:
			{
				USHORT temp = 0;

				if( _wcsicmp(
						_T("ApplicationId"),
						MofItem.Name))
				{
					hr = pProperty->pQualifiers->Get(
						L"Format",
						0, 
						NULL,
						NULL);

					if (SUCCEEDED(hr))
					{
						IsWideString = TRUE;
					}

					hr = pProperty->pQualifiers->Get(
						L"StringTermination",
						0,
						&varQualifier, 
						NULL);

					if (FAILED(hr) || (_wcsicmp(
						varQualifier.bstrVal,
						L"NullTerminated") == 0))
					{
						IsNullTerminated = TRUE;
					}
					else if (_wcsicmp(
						varQualifier.bstrVal,
						L"Counted") == 0)
					{
						// First two bytes of the string contain its length.

						CopyMemory(
							&StringLength, 
							pEventData,
							sizeof(USHORT));

						pEventData += sizeof(USHORT);
					}
					else if (_wcsicmp(
						varQualifier.bstrVal,
						L"ReverseCounted") == 0)
					{

						CopyMemory(
							&temp,
							pEventData,
							sizeof(USHORT));

						StringLength = MAKEWORD(
							HIBYTE(temp), 
							LOBYTE(temp));

						pEventData += sizeof(USHORT);
					}
					else if (_wcsicmp(
						varQualifier.bstrVal,
						L"NotCounted") == 0)
					{

						StringLength = RemainingBytes;
					}

					if ( ArraySize)
					{
						MofItem.mofcArray = new MOF_CHAR_ARRAY[ArraySize];

						if ( MofItem.mofcArray)
						{
							MofItem.Types = CHAR_ARRAY_VALUE;
							ZeroMemory(
								MofItem.mofcArray,
								sizeof(MOF_CHAR_ARRAY)*ArraySize);

							MofItem.ArraySize = ArraySize;
						}
						else
						{
							return NULL;
						}
					}

					VariantClear(&varQualifier);

					for (ULONG i = 0; i < ArraySize; i++)
					{
						if (IsWideString)
						{
							if (IsNullTerminated)
							{
								StringLength = (USHORT)(wcslen((WCHAR*)pEventData));
								StringLength += sizeof(WCHAR);

								if ( StringLength && MofItem.mofcArray )
								{
									MofItem.mofcArray[i].Name[0] = i;
									MofItem.mofcArray[i].wValue = new WCHAR[StringLength];

									if (MofItem.mofcArray[i].wValue)
									{
										StringCchCopy(
											MofItem.mofcArray[i].wValue,
											StringLength,
											(STRSAFE_LPCWSTR)pEventData);
									}
								}				

							}
							else
							{
								LONG StringSize = (StringLength) * sizeof(WCHAR); 
								//WCHAR* pwsz = (WCHAR*)malloc(StringSize + sizeof(WCHAR)); // +2 for NULL

								if (MofItem.mofcArray)
								{
									MofItem.mofcArray[i].Name[0] = i;
									MofItem.mofcArray[i].wValue = new WCHAR[StringLength+1];

									if (MofItem.mofcArray[i].wValue)
									{
										CopyMemory(
											MofItem.mofcArray[i].wValue,
											(WCHAR*)pEventData,
											StringSize);

										*(MofItem.mofcArray[i].wValue+StringSize) = '\0';

									}
								}

							}

							StringLength *= sizeof(WCHAR);
						}
						else  // It is an ANSI string
						{
							if (IsNullTerminated)
							{
								StringLength = (USHORT)strlen((char*)pEventData) + 1;
								//printf("%s\n", (char*)pEventData);
								if ( StringLength && MofItem.mofcArray )
								{
									MofItem.mofcArray[i].Name[0] = i;
									MofItem.mofcArray[i].cValue = new char[StringLength];

									if (MofItem.mofcArray[i].cValue)
									{
										StringCchCopyA(
											MofItem.mofcArray[i].cValue,
											StringLength,
											(STRSAFE_LPCSTR)pEventData);
									}
								}
							}
							else
							{
								//char* psz = (char*)malloc(StringLength+1);  // +1 for NULL
								if (MofItem.mofcArray)
								{
									MofItem.mofcArray[i].Name[0] = i;
									MofItem.mofcArray[i].cValue = new char[StringLength+1];

									if (MofItem.mofcArray[i].cValue)
									{
										CopyMemory(
											MofItem.mofcArray[i].cValue,
											(char*)pEventData,
											StringLength);

										*(MofItem.mofcArray[i].cValue + StringLength) = '\0';

									}
								}

							}
						}

						pEventData += StringLength;
						StringLength = 0;
					}
				}
				else
				{
					MofItem.Types = OTHER_VALUE;
					MofItem.liValue.QuadPart = 0;

					CopyMemory( 
							&MofItem.liValue.LowPart,
							pEventData,
							4);
					 
					(UCHAR*)pEventData += 4;
				}

				
				return pEventData;
			} 

		case CIM_BOOLEAN:
			{
				BOOL temp = FALSE;

				for (ULONG i = 0; i < ArraySize; i++)
				{
					CopyMemory(&temp, pEventData, sizeof(BOOL));
					wprintf(L"%s\n", (temp) ? L"TRUE" : L"FALSE");
					pEventData += sizeof(BOOL);
				}

				return pEventData;
			}

		case CIM_SINT8:
		case CIM_UINT8:
			{
				hr = pProperty->pQualifiers->Get(
												L"Extension",
												0,
												&varQualifier, 
												NULL);
				if (SUCCEEDED(hr))
				{
					
					if (_wcsicmp(
							L"Guid",
							varQualifier.bstrVal) == 0)
					{
						WCHAR szGuid[50] =  {0};
						GUID Guid;

						CopyMemory(
								&Guid,
								(GUID*)pEventData,
								sizeof(GUID));

						StringFromGUID2(	
									Guid,
									szGuid,
									sizeof(szGuid)-1);
						MofItem.Types = TCHAR_VALUE;
						MofItem.cValues = new TCHAR[50];

						if (MofItem.cValues)
						{
							StringCchCopy(
										MofItem.cValues,
										50,
										szGuid
										);
						}
						
					}

					VariantClear(&varQualifier);
					pEventData += sizeof(GUID);
				}
				else 
				{
					hr = pProperty->pQualifiers->Get(
													L"Format",
													0,
													NULL,
													NULL);
					if (SUCCEEDED(hr))
					{
						PrintAsChar = TRUE;  // ANSI character
					}

					for (ULONG i = 0; i < ArraySize; i++)
					{
						if (PrintAsChar)
							wprintf(L"%c", *((char*)pEventData)); 
						else
							wprintf(L"%hd", *((BYTE*)pEventData));

						pEventData += sizeof(UINT8);
					}
				}

				wprintf(L"\n");

				return pEventData;
			}

		case CIM_CHAR16:
			{
				WCHAR temp;

				for (ULONG i = 0; i < ArraySize; i++)
				{
					CopyMemory(&temp, pEventData, sizeof(WCHAR));
					wprintf(L"%c", temp);
					pEventData += sizeof(WCHAR);
				}

				wprintf(L"\n");

				return pEventData;
			}

		case CIM_SINT16:
			{
				SHORT temp = 0;

				for (ULONG i = 0; i < ArraySize; i++)
				{
					CopyMemory(&temp, pEventData, sizeof(SHORT));
					wprintf(L"%hd\n", temp);
					pEventData += sizeof(SHORT);
				}

				return pEventData;
			}

		case CIM_UINT16:
			{
				USHORT temp = 0;

				// If the data is a port number, call the ntohs Windows Socket 2 function
				// to convert the data from TCP/IP network byte order to host byte order.
				// This is here to support legacy event classes; the Port extension 
				// should only be used on properties whose CIM type is object.

				hr = pProperty->pQualifiers->Get(
												L"Extension",
												0, 
												&varQualifier,
												NULL);
				if (SUCCEEDED(hr))
				{
					if (_wcsicmp(
								L"Port",
								varQualifier.bstrVal) == 0)
					{
						PrintAsPort = TRUE;
					}

					VariantClear(&varQualifier);
				}

				if (ArraySize)
				{
					MofItem.u32ArrayValue = new uint32[ArraySize];
					if (MofItem.u32ArrayValue)
					{
						MofItem.Types = UINT32_ARRAY_VALUE;
						ZeroMemory(
									MofItem.u32ArrayValue,
									sizeof(uint32)*ArraySize
									);
					}
				}

				for (ULONG i = 0; i < ArraySize; i++)
				{
					CopyMemory(
							&temp,
							pEventData,
							sizeof(USHORT));

					if (MofItem.u32ArrayValue)
					{
						if (PrintAsPort)
						{
							MofItem.u32ArrayValue[i] = ntohs(temp);
						}
						else
						{
							MofItem.u32ArrayValue[i] = temp;
						}
					}					

					pEventData += sizeof(USHORT);
				}

				return pEventData;
			}

		case CIM_OBJECT:
			{
				// An object data type has to include the Extension qualifier.

				hr = pProperty->pQualifiers->Get(
												L"Extension",
												0, 
												&varQualifier, 
												NULL);
				if (SUCCEEDED(hr))
				{
					if (_wcsicmp(
								L"SizeT",
								varQualifier.bstrVal) == 0)
					{
						VariantClear(&varQualifier);

						// You do not need to know the data type of the property, you just 
						// retrieve either 4 bytes or 8 bytes depending on the pointer's size.

						for (ULONG i = 0; i < ArraySize; i++)
						{
							if (m_PointerSize == 4) 
							{
								ULONG temp = 0;

								CopyMemory(
										&temp, 
										pEventData,
										sizeof(ULONG));
								wprintf(L"0x%x\n", temp);
							}
							else
							{
								ULONGLONG temp = 0;

								CopyMemory(&temp, pEventData, sizeof(ULONGLONG));
								wprintf(L"0x%x\n", temp);
							}

							pEventData += m_PointerSize;
						}

						return pEventData;
					}
					if (_wcsicmp(
								L"Port", 
								varQualifier.bstrVal) == 0)
					{
						USHORT temp = 0;

						VariantClear(&varQualifier);

						for (ULONG i = 0; i < ArraySize; i++)
						{
							CopyMemory(
									&temp,
									pEventData, 
									sizeof(USHORT));
							wprintf(L"%hu\n", ntohs(temp));
							pEventData += sizeof(USHORT);
						}

						return pEventData;
					}
					else if (_wcsicmp(L"IPAddr", varQualifier.bstrVal) == 0 ||
						_wcsicmp(L"IPAddrV4", varQualifier.bstrVal) == 0)
					{
						ULONG temp = 0;

						VariantClear(&varQualifier);

						for (ULONG i = 0; i < ArraySize; i++)
						{
							CopyMemory(&temp, pEventData, sizeof(ULONG));

							wprintf(L"%d.%d.%d.%d\n", (temp >>  0) & 0xff,
								(temp >>  8) & 0xff,
								(temp >>  16) & 0xff,
								(temp >>  24) & 0xff);

							pEventData += sizeof(ULONG);
						}

						return pEventData;
					}
					else if (_wcsicmp(
									L"IPAddrV6",
									varQualifier.bstrVal) == 0)
					{
						typedef LPTSTR (WINAPI *PIPV6ADDRTOSTRING)(
							const IN6_ADDR *Addr,
							LPTSTR S
							);

						WCHAR IPv6AddressAsString[46] = {0};
						IN6_ADDR IPv6Address;
						PIPV6ADDRTOSTRING fnRtlIpv6AddressToString;

						VariantClear(&varQualifier);

						fnRtlIpv6AddressToString = (PIPV6ADDRTOSTRING)
												GetProcAddress(
															GetModuleHandle(L"ntdll"),
															"RtlIpv6AddressToStringW");

						if (NULL == fnRtlIpv6AddressToString)
						{
							wprintf(L"GetProcAddress failed with %lu.\n", GetLastError());
							return NULL;
						}

						for (ULONG i = 0; i < ArraySize; i++)
						{
							CopyMemory(
									&IPv6Address,
									pEventData,
									sizeof(IN6_ADDR));

							fnRtlIpv6AddressToString(
												&IPv6Address,
												IPv6AddressAsString);

							wprintf(L"%s\n", IPv6AddressAsString);

							pEventData += sizeof(IN6_ADDR);
						}

						return pEventData;
					}
					else if (_wcsicmp(L"Guid", varQualifier.bstrVal) == 0)
					{
						WCHAR szGuid[50] = {0};
						GUID Guid;

						VariantClear(&varQualifier);

						for (ULONG i = 0; i < ArraySize; i++)
						{
							CopyMemory(&Guid, (GUID*)pEventData, sizeof(GUID));

							StringFromGUID2(Guid, szGuid, sizeof(szGuid)-1);
							wprintf(L"%s\n", szGuid);

							pEventData += sizeof(GUID);
						}

						return pEventData;
					}
					else if (_wcsicmp(L"Sid", varQualifier.bstrVal) == 0)
					{
						// Get the user's security identifier and print the 
						// user's name and domain.

						SID* psid;
						DWORD cchUserSize = 0;
						DWORD cchDomainSize = 0;
						WCHAR* pUser = NULL;
						WCHAR* pDomain = NULL;
						SID_NAME_USE eNameUse;
						DWORD status = 0;
						ULONG temp = 0;
						USHORT CopyLength = 0;
						BYTE buffer[SECURITY_MAX_SID_SIZE];

						VariantClear(&varQualifier);

						if ( ArraySize == 1)
						{
							MofItem.mofcArray = new MOF_CHAR_ARRAY[2];
							if (MofItem.mofcArray)
							{
								MofItem.Types = CHAR_ARRAY_VALUE;
								ZeroMemory(
										MofItem.mofcArray,
										sizeof(MOF_CHAR_ARRAY)*2);

								MofItem.ArraySize = 2;
							}
						}

						for (ULONG i = 0; i < ArraySize; i++)
						{
							CopyMemory(
									&temp,
									pEventData, 
									sizeof(ULONG));

							if (temp > 0)
							{

								USHORT BytesToSid = m_PointerSize * 2;

								pEventData += BytesToSid;

								if (RemainingBytes - BytesToSid > SECURITY_MAX_SID_SIZE)
								{
									CopyLength = SECURITY_MAX_SID_SIZE;
								}
								else
								{
									CopyLength = RemainingBytes - BytesToSid;
								}

								CopyMemory(
										&buffer,
										pEventData,
										CopyLength);

								psid = (SID*)&buffer;

								LookupAccountSid(
												NULL,
												psid,
												pUser,
												&cchUserSize,
												pDomain, 
												&cchDomainSize,
												&eNameUse);

								status = GetLastError();
								if (ERROR_INSUFFICIENT_BUFFER == status)
								{
									pUser = (WCHAR*)malloc(cchUserSize * sizeof(WCHAR));
									pDomain = (WCHAR*)malloc(cchDomainSize * sizeof(WCHAR));

									if (pUser && pDomain)
									{
										if (LookupAccountSid(
															NULL,
															psid,
															pUser,
															&cchUserSize,
															pDomain,
															&cchDomainSize,
															&eNameUse))
										{

											if ( i == 0)
											{

												if ( MofItem.mofcArray )
												{
													MofItem.mofcArray[0].wValue = new WCHAR[cchDomainSize];

													if (MofItem.mofcArray[0].wValue)
													{
														StringCchCopy(
															MofItem.mofcArray[0].Name,
															50,
															_T("Domain")
																	);

														StringCchCopy(
															MofItem.mofcArray[0].wValue,
															cchDomainSize,
															pDomain
															);		
													}

													MofItem.mofcArray[1].wValue = new WCHAR[cchUserSize];

													if (MofItem.mofcArray[1].wValue)
													{
														StringCchCopy(
															MofItem.mofcArray[1].Name,
															50,
															_T("User")
															);

														StringCchCopy(
															MofItem.mofcArray[1].wValue,
															cchUserSize,
															pUser
															);

													}
												}
											}
											
										}
										else
										{
											wprintf(L"Second LookupAccountSid failed with, %d\n", GetLastError());
										}
									}
									else
									{
										wprintf(L"Allocation error.\n");
									}

									if (pUser)
									{
										free(pUser);
										pUser = NULL;
									}

									if (pDomain)
									{
										free(pDomain);
										pDomain = NULL;
									}

									cchUserSize = 0;
									cchDomainSize = 0;
								}
								else if (ERROR_NONE_MAPPED == status)
								{
									wprintf(L"Unable to locate account for the specified SID\n");
								}
								else
								{
									wprintf(L"First LookupAccountSid failed with, %d\n", status);
								}

								pEventData += SeLengthSid(psid);
							}
							else  // There is no SID
							{
								pEventData += sizeof(ULONG);
							}
						}

						return pEventData;
					}
					else
					{
						wprintf(L"Extension, %s, not supported.\n", varQualifier.bstrVal);
						VariantClear(&varQualifier);
						return NULL;
					}
				}
				else
				{
					wprintf(L"Object data type is missing Extension qualifier.\n");
					return NULL;
				}
			}

		default: 
			{
				wprintf(L"Unknown CIM type\n");
				return NULL;
			}

		} // switch
	}
}

LONG CMofDataParser::GetArraySize(
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

LONG CMofDataParser::GetArrayValue(
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


void CMofDataParser::AddProperityList(
							LIST_ENTRY* plist,
							BSTR pvData,
							LONG ValueLen,
							LONG ArraySize)
{
	LIST_ENTRY* pEntry = plist->Flink;
	
	while(pEntry != plist)
	{		
		CMofParseRes* pRes = CONTAINING_RECORD(
											pEntry,
											CMofParseRes,
											m_Entry);

		if (pRes)
		{
			MOF_PROPERTY*  pProperty = (MOF_PROPERTY*)
									malloc(sizeof(MOF_PROPERTY));

			if (pProperty)
			{
				__try
				{
					ZeroMemory(pProperty,sizeof(MOF_PROPERTY));
					pProperty->Entry.Flink = pProperty->Entry.Blink = 0;
					int nLen = 0;
					if (pvData && (nLen = lstrlen(pvData)))
					{
						pProperty->ArraySize = ArraySize;
						pProperty->Types = ValueLen;

						pProperty->Name = (BSTR)malloc((nLen+1)* sizeof(OLECHAR));

						if (pProperty->Name)
						{
							__try
							{
								StringCchCopy(
									pProperty->Name,
									nLen + 1,
									pvData);

								InsertTailList(
									&pRes->m_MofPropertyList,
									&pProperty->Entry);
							}
							__except(EXCEPTION_EXECUTE_HANDLER)
							{
								if (pProperty->Name)
								{
									free(pProperty->Name);
									pProperty->Name = NULL;
								}
							}
							
						}
						else
						{
							free(pProperty);
							pProperty = NULL;
						}
					}					


				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					if (pProperty)
					{
						free(pProperty);
						pProperty = NULL;
					}
				}
			}
		}

		__try
		{
			pEntry = pEntry->Flink;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			break;
		}
		
	}
}

void CMofDataParser::AddGlobalEvent(
							LIST_ENTRY* plist
							)
{

	while(!IsListEmpty(plist))
	{
		LIST_ENTRY* pEntry = plist->Flink;
		__try
		{
			CMofParseRes* pRes = CONTAINING_RECORD(
				pEntry,
				CMofParseRes,
				m_Entry);

			if (pRes)
			{
				RemoveEntryList(&pRes->m_Entry);
				InsertTailList(&m_ModDataList,&pRes->m_Entry);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			break;
		}		
	}

	InitializeListHead(plist);
}