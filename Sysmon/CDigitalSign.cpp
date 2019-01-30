#include "StdAfx.h"
#include "CDigitalSign.h"


CDigitalSign::CDigitalSign(void)
	:m_bInit(false)
	,m_WinVerifyTrust(NULL)
	,m_pWTHelperGetProvSignerFromChain(NULL)
	,m_pWTHelperProvDataFromStateData(NULL)
	,m_pCryptCATAdminReleaseContext(NULL)
	,m_pCryptCATAdminReleaseCatalogContext(NULL)
	,m_pCryptCATCatalogInfoFromContext(NULL)
	,m_pCryptCATAdminEnumCatalogFromHash(NULL)
	,m_pCryptCATAdminCalcHashFromFileHandle(NULL)
	,m_pCryptCATAdminAcquireContext(NULL)
	,m_pCryptCATAdminAddCatalog(NULL)
	,m_pCryptCATAdminRemoveCatalog(NULL)
	,m_pIsCatalogFile(NULL)
	,m_pCertNameToStrW(NULL)
{
}


CDigitalSign::~CDigitalSign(void)
{
}


bool CDigitalSign::Init()
{
	if (!m_bInit)
	{
		m_bInit = true;

		HMODULE hWinTrust = LoadLibraryW(L"Wintrust.dll");

		if ( !hWinTrust )
			return false;
		
		m_WinVerifyTrust = (pWinVerifyTrust)GetProcAddress(hWinTrust, "WinVerifyTrust");
		m_pWTHelperGetProvSignerFromChain = (pWTHelperGetProvSignerFromChain)
							GetProcAddress(hWinTrust, "WTHelperGetProvSignerFromChain");

		m_pWTHelperProvDataFromStateData = (pWTHelperProvDataFromStateData)
							GetProcAddress(hWinTrust, "WTHelperProvDataFromStateData");
		
		m_pCryptCATAdminReleaseContext = (pCryptCATAdminReleaseContext)
							GetProcAddress(hWinTrust, "CryptCATAdminReleaseContext");

		m_pCryptCATAdminReleaseCatalogContext = (pCryptCATAdminReleaseCatalogContext)
							GetProcAddress(hWinTrust, "CryptCATAdminReleaseCatalogContext");

		m_pCryptCATCatalogInfoFromContext = (pCryptCATCatalogInfoFromContext)
							GetProcAddress(hWinTrust, "CryptCATCatalogInfoFromContext");

		m_pCryptCATAdminEnumCatalogFromHash = (pCryptCATAdminEnumCatalogFromHash)
							GetProcAddress(hWinTrust, "CryptCATAdminEnumCatalogFromHash");

		m_pCryptCATAdminCalcHashFromFileHandle = (pCryptCATAdminCalcHashFromFileHandle)
							GetProcAddress(hWinTrust,"CryptCATAdminCalcHashFromFileHandle");

		m_pCryptCATAdminAcquireContext = (pCryptCATAdminAcquireContext)
							GetProcAddress(hWinTrust,"CryptCATAdminAcquireContext");

		m_pCryptCATAdminAddCatalog = (pCryptCATAdminAddCatalog)
							GetProcAddress(hWinTrust, "CryptCATAdminAddCatalog");

		m_pCryptCATAdminRemoveCatalog = (pCryptCATAdminRemoveCatalog)
							GetProcAddress(hWinTrust, "CryptCATAdminRemoveCatalog");

		m_pIsCatalogFile = (pIsCatalogFile)GetProcAddress(hWinTrust, "IsCatalogFile");

		hWinTrust = LoadLibraryW(L"crypt32.dll");
		m_pCertNameToStrW = (pCertNameToStrW)GetProcAddress(hWinTrust, "CertNameToStrW");
	}
	
	return m_pCryptCATAdminAcquireContext != 0;
}