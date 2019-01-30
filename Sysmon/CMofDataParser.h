#ifndef _CMofDataParser_h
#define _CMofDataParser_h

#include "Sysmon.h"

#define MAX_CHAR_ARRAY_SIZE 20
#define TCHAR_VALUE 0x1
#define UINT32_ARRAY_VALUE 0x2
#define OTHER_VALUE 0x4
#define CHAR_ARRAY_VALUE 0x8
#include <WbemCli.h>
#include <Wmistr.h>
#include <Evntrace.h>
#include <strsafe.h>

typedef unsigned int uint32;
typedef unsigned long long uint64;

extern "C"
{
	typedef HRESULT (WINAPI * pSafeArrayDestroy)(SAFEARRAY *);
	typedef HRESULT (WINAPI * pSafeArrayGetElement)(SAFEARRAY * , LONG *, void *);
};


typedef struct _MOF_DATA_HEADER
{
	int			MofCount;
	SYSTEMTIME	MofLocal;
	BSTR		MofClassName;
	BSTR		MofTypeName;
	USHORT		MofType;
	USHORT		MofLevel;
	USHORT		MofVsersion;
	GUID		MofGuid;
}MOF_DATA_HEADER;

typedef struct _MOF_CHAR_ARRAY
{
	TCHAR Name[20];
	union
	{
		char*  cValue;
		WCHAR* wValue; 
	};

}MOF_CHAR_ARRAY;

typedef struct _MOF_ITEM
{
	TCHAR	Name[50];
	ULONG	Types;
	UINT32	ArraySize;
	union
	{
		TCHAR*			cValues;
		LARGE_INTEGER	liValue;
		UINT32*			u32ArrayValue;
		MOF_CHAR_ARRAY*	mofcArray;
	};
	
}MOF_ITEM,*PMOF_ITEM;

typedef struct _MOF_PROPERTY
{
	LIST_ENTRY	Entry;
	BSTR		Name;
	ULONG		Types;
	UINT32		ArraySize;

}MOF_PROPERTY;

class CMofParseRes
{
	friend class  CMofDataParser;
public:
	CMofParseRes()
	{
		ZeroMemory(
				this,
				sizeof(*this));

		InitializeListHead(
					&m_MofPropertyList
					);
	}

	~CMofParseRes()
	{
	}

	UINT32 GetCount()
	{
		return m_MofHeader.MofCount;
	}

	SYSTEMTIME GetTimeStmp()
	{
		return m_MofHeader.MofLocal;
	}
	
public:
	LIST_ENTRY			m_Entry;
private:
	MOF_DATA_HEADER		m_MofHeader;
	LIST_ENTRY			m_MofPropertyList;

};

typedef struct _propertyList
{
	BSTR Name;     
	LONG CimType;
	IWbemQualifierSet* pQualifiers;

} PROPERTY_LIST;

class CMofData
{
public:
	CMofData(void)
	{
		m_Mof = NULL;
		memset(
			&m_MofHead,
			0,
			sizeof(MOF_DATA_HEADER));
	}

	CMofData(const CMofData& Mof)
	{
		m_Mof = Mof.m_Mof;
	}

	CMofData(const PMOF_ITEM& Mof)
	{
		m_Mof = Mof;
	}

	~CMofData(void)
	{

	}

	
	void operator =(const PMOF_ITEM Mof)
	{
		m_Mof = Mof;
	}

	BOOL operator ==(const TCHAR* Name )
	{
		if ( m_Mof )
		{
			if (sizeof(TCHAR) >=  2 )
			{
				if ( !wcsicmp(Name,m_Mof->Name))
				{
					return TRUE;
				}
				
			}
			else
			{
				if (!stricmp(
							(char*)Name,
							(char*)m_Mof->Name))
				{
					return TRUE;
				}
			}
			
		}
		return FALSE;
	}

	UINT32 getDataInt32( UINT32 DataType ,int Index = 0)
	{
		if ( m_Mof )
		{
			if(m_Mof->Types == DataType
				 )
			{

				if (DataType == UINT32_ARRAY_VALUE)
				{
					if (m_Mof->u32ArrayValue)
					{
						return m_Mof->u32ArrayValue[Index];
					}		
				}
				else if ( DataType == CHAR_ARRAY_VALUE )
				{
					if ( m_Mof->mofcArray )
					{
						return (UINT32)&m_Mof->mofcArray[Index];
					}		
				}
				
						
			}
			
		}

		return 0;
	}

	UINT64 getDataInt64( UINT32 DataType ,int Index = 0 )
	{
		if ( m_Mof )
		{
			if(m_Mof->Types == DataType
				)
			{

				if (DataType == OTHER_VALUE)
				{					
					return m_Mof->liValue.QuadPart;						
				}
			}

		}

		return 0;
	}

	TCHAR* getWideString( UINT32 DataType ,int Index = 0 )
	{
		if ( m_Mof )
		{
			if(m_Mof->Types == DataType
				)
			{

				if (DataType == CHAR_ARRAY_VALUE )
				{
					if (m_Mof->mofcArray)
					{
						return m_Mof->mofcArray[Index].wValue;
					}		
				}
				else if ( DataType == TCHAR_VALUE )
				{
					if ( m_Mof->cValues )
					{
						return (TCHAR*)m_Mof->cValues;
					}		
				}
			}
		}

		return NULL;
	}

	char* getAnsiString( UINT32 DataType ,int Index = 0 )
	{
		if ( m_Mof )
		{
			if(m_Mof->Types == DataType
				)
			{

				if (DataType == CHAR_ARRAY_VALUE )
				{
					if (m_Mof->mofcArray)
					{
						return m_Mof->mofcArray[Index].cValue;
					}		
				}
				else if ( DataType == TCHAR_VALUE )
				{
					if ( m_Mof->cValues )
					{
						return (char*)m_Mof->cValues;
					}		
				}
			}
		}

		return NULL;
	}

	UINT32 GetProcessId()
	{
		return getDataInt32( UINT32_ARRAY_VALUE );
	}

	UINT64 GetUniqueProcessKey()
	{
		return getDataInt64( OTHER_VALUE );
	}

	char* GetProcessImageFileName()
	{
		char* ImageFileName = NULL;

		if (! (ImageFileName = getAnsiString( TCHAR_VALUE , 0 )) )
		{
			ImageFileName = getAnsiString(CHAR_ARRAY_VALUE,0);
		}

		return ImageFileName;
	}


	/************************************************************************/
	/* FileObject
	*/
	/************************************************************************/

	UINT64 GetFileObject()
	{
		return getDataInt64(OTHER_VALUE,0);
	}

	TCHAR* GetFileOpenPath()
	{
		TCHAR* OpenPath = NULL;

		if ( !(OpenPath = getWideString(
								TCHAR_VALUE,
								0)))
		{
			OpenPath = getWideString(
								CHAR_ARRAY_VALUE,
								0);
		}

		return OpenPath;
	}

private:
	MOF_DATA_HEADER m_MofHead;
	PMOF_ITEM		m_Mof;
	LIST_ENTRY		m_Entry;
};



class CMofDataParser
{
public:
	CMofDataParser(void);
	

	virtual ~CMofDataParser(void);
private:
	HRESULT Connect(
		BSTR bstrNamespace
		);
public:
	
	void SetPointSize(ULONG PointSize )
	{
		m_PointerSize = 8;
	}

	BOOL Parse(
			PEVENT_TRACE pEvent,
			CMofParseRes& Result );

private:
	IWbemServices*				m_pServices;
	ULONG						m_PointerSize;
	LIST_ENTRY					m_ModDataList;
protected:
	void GetPropertyName(
			PROPERTY_LIST* pProperty,
			TCHAR* PropertyName
			);

	PBYTE GetPropertyValue(
			PROPERTY_LIST*	pProperty,
			MOF_ITEM&		MofItem,
			PBYTE			pEventData,
			USHORT			RemainingBytes);

	IWbemClassObject* GetEventCategoryClass(
								BSTR bstrClassGuid,
								int Version);

	CMofParseRes* GetEventClassPropertyList(
								IWbemClassObject* pEventCategoryClass,
								GUID& Guid,
								USHORT Version,
								USHORT Level,
								USHORT EventType);

	void FreePropertyList(
					PROPERTY_LIST* pProperties,
					DWORD Count);

	BOOL GetPropertyList(
					IWbemClassObject* pClass, 
					PROPERTY_LIST** ppProperties,
					DWORD* pPropertyCount,
					LONG* PropertyIndex);

	LONG GetArraySize(
							IWbemQualifierSet* pQualifierSet);

	LONG GetArrayValue(
					CIMTYPE CimType,
					IWbemQualifierSet * pQualifierSet);

	void AddProperityList(
					LIST_ENTRY* Entry,
					BSTR pvData,
					LONG ValueLen,
					LONG ArraySize);

	void AddGlobalEvent(
					LIST_ENTRY* Entry);

public:
	CMofDataParser* Instance();
};

class FileCreateNameX86
{

public:
	FileCreateNameX86(void)
	{

	}

	~FileCreateNameX86()
	{

	}

	UINT32 FileObject;
	TCHAR FileName[1];
};

class CPropertyRes;


class CPropertyRes
{
	friend class CPropertyData;
public:
	CPropertyRes(void)
	{
		ZeroMemory(this,sizeof(*this));
	}

	CPropertyRes(const CPropertyRes& Res)
	{
		ZeroMemory(this,sizeof(*this));

		if (Res.mData_ && Res.mLen_)
		{
			mData_ = new BYTE[Res.mLen_ + 1];

			if ( mData_ )
			{
				CopyMemory(
					mData_,
					Res.mData_,
					Res.mLen_);

				mLen_ = Res.mLen_;
			}

			CopyMemory(
				mName,
				Res.mName,
				50);
		}	
		
	}

	~CPropertyRes(void)
	{
		if (mData_)
		{
			delete[] (char*)mData_;
			mData_ = NULL;
		}

		mLen_ = NULL;

		ZeroMemory(this,sizeof(*this));
	}

	void Init()
	{
		mData_ = NULL;
		mLen_ = 0;
	}

	TCHAR* GetFileName()
	{
		return (TCHAR*)mData_;
	}

	char* GetProcessName()
	{
		return (char*)mData_;
	}

	TCHAR* GetQueryDomainName()
	{
		return (TCHAR*)mData_;
	}

	TCHAR* GetQueryResult()
	{
		return (TCHAR*)mData_;
	}

	ULONGLONG GetFileObject()
	{
		if ( mData_ )
		{
			return *((ULONGLONG*)mData_);
		}

		return 0;
	}

	ULONG GetProcessId()
	{
		if ( mData_ )
		{
			return *((ULONG*)mData_);
		}

		return -1;
	}

	WORD GetPort()
	{
		if ( mData_ )
		{
			return *((WORD*)mData_);
		}

		return -1;
	}

	ULONG GetQueryStatus()
	{
		if ( mData_ )
		{
			return *((ULONG*)mData_);
		}

		return -1;
	}

	ULONG GetIpAddress()
	{
		if ( mData_ )
		{
			return *((ULONG*)mData_);
		}

		return -1;
	}

	ULONG GetIoSize()
	{
		if ( mData_ )
		{
			return *((ULONG*)mData_);
		}

		return 0;
	}

	BOOL operator == ( TCHAR* StrName)
	{
		if (!wcsicmp(StrName,mName))
		{
			return TRUE;
		}

		return FALSE;
	}

private:
	TCHAR mName[50];
	PBYTE mData_;
	int   mLen_;

};

#define PARSE_TYPE_FILE 1
#define PARSE_TYPE_OTHER 2

static CPropertyRes Res;

#endif