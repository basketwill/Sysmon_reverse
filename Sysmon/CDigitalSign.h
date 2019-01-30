#ifndef _CDigitalSign_h
#define _CDigitalSign_h
#include <WinTrust.h>

typedef void* HCATINFO;
typedef void* HCATADMIN;

typedef struct CATALOG_INFO_ {
	DWORD cbStruct;
	WCHAR wszCatalogFile[MAX_PATH];
} CATALOG_INFO;

extern "C"
{
	typedef
	LONG
	( WINAPI *pWinVerifyTrust)(
							HWND hwnd,
							GUID *pgActionID,
							LPVOID pWVTData);

	typedef 
	CRYPT_PROVIDER_SGNR* 
	(WINAPI *pWTHelperGetProvSignerFromChain)(
							CRYPT_PROVIDER_DATA *pProvData,
							DWORD idxSigner,
							BOOL fCounterSigner,
							DWORD idxCounterSigner);

	typedef
	CRYPT_PROVIDER_DATA * (WINAPI *pWTHelperProvDataFromStateData)(
							HANDLE hStateData);

	typedef
	BOOL 
	(WINAPI *pCryptCATAdminReleaseContext)(
		IN VOID* hCatAdmin,
		IN DWORD     dwFlags
		);

	typedef
	BOOL 
	(WINAPI *pCryptCATAdminReleaseCatalogContext)(
		IN VOID* hCatAdmin,
		IN VOID*  hCatInfo,
		IN DWORD     dwFlags
		);

	typedef
	BOOL
	(WINAPI *pCryptCATCatalogInfoFromContext)(
		_In_    VOID*     hCatInfo,
		_Inout_ CATALOG_INFO *psCatInfo,
		_In_    DWORD        dwFlags
		);

	typedef
	HCATINFO 
	(WINAPI *pCryptCATAdminEnumCatalogFromHash)(
		HCATADMIN hCatAdmin,
		BYTE      *pbHash,
		DWORD     cbHash,
		DWORD     dwFlags,
		HCATINFO  *phPrevCatInfo
		);

	typedef
	BOOL
	(WINAPI *pCryptCATAdminCalcHashFromFileHandle)(
		HANDLE hFile,
		DWORD  *pcbHash,
		BYTE   *pbHash,
		DWORD  dwFlags
		);

	typedef
	BOOL
	(WINAPI *pCryptCATAdminAcquireContext)(
		HCATADMIN  *phCatAdmin,
		const GUID *pgSubsystem,
		DWORD      dwFlags
		);

	typedef
	HCATINFO
	(WINAPI *pCryptCATAdminAddCatalog)(
		HCATADMIN hCatAdmin,
		PWSTR     pwszCatalogFile,
		PWSTR     pwszSelectBaseName,
		DWORD     dwFlags
		);

	typedef
	BOOL
	(WINAPI *pCryptCATAdminRemoveCatalog)(
		IN HCATADMIN hCatAdmin,
		IN LPCWSTR   pwszCatalogFile,
		IN DWORD     dwFlags
		);

	typedef
	BOOL 
	(WINAPI *pIsCatalogFile)(
		IN HANDLE hFile,
		WCHAR     *pwszFileName
		);

	typedef
	DWORD
	(WINAPI *pCertNameToStrW)(
		DWORD           dwCertEncodingType,
		PCERT_NAME_BLOB pName,
		DWORD           dwStrType,
		LPSTR           psz,
		DWORD           csz
		);
};



class CDigitalSign
{
public:
	CDigitalSign(void);
	virtual ~CDigitalSign(void);

	bool Init();
private:
	bool m_bInit;
	pWinVerifyTrust							m_WinVerifyTrust;
	pWTHelperGetProvSignerFromChain			m_pWTHelperGetProvSignerFromChain;
	pWTHelperProvDataFromStateData			m_pWTHelperProvDataFromStateData;
	pCryptCATAdminReleaseContext			m_pCryptCATAdminReleaseContext;
	pCryptCATAdminReleaseCatalogContext		m_pCryptCATAdminReleaseCatalogContext;
	pCryptCATCatalogInfoFromContext			m_pCryptCATCatalogInfoFromContext;
	pCryptCATAdminEnumCatalogFromHash		m_pCryptCATAdminEnumCatalogFromHash;
	pCryptCATAdminCalcHashFromFileHandle	m_pCryptCATAdminCalcHashFromFileHandle;
	pCryptCATAdminAcquireContext			m_pCryptCATAdminAcquireContext;
	pCryptCATAdminAddCatalog				m_pCryptCATAdminAddCatalog;
	pCryptCATAdminRemoveCatalog				m_pCryptCATAdminRemoveCatalog;
	pIsCatalogFile							m_pIsCatalogFile;
	pCertNameToStrW							m_pCertNameToStrW;
};

#endif

