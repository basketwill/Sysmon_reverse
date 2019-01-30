#include "StdAfx.h"
#include "CSysmonDriverOpt.h"


CSysmonDriverOpt::CSysmonDriverOpt(void)
{
}


CSysmonDriverOpt::~CSysmonDriverOpt(void)
{
}


//======================================== 动态加载/卸载sys驱动 ======================================
// SYS文件跟程序放在同个目录下
// 如果产生的SYS名为HelloDDK.sys,那么安装驱动InstallDriver("HelloDDK",".\\HelloDDK.sys","370030"/*Altitude*/);
// 启动驱动服务 StartDriver("HelloDDK");
// 停止驱动服务 StopDriver("HelloDDK");
// 卸载SYS也是类似的调用过程， DeleteDriver("HelloDDK");
//====================================================================================================

BOOL CSysmonDriverOpt::InstallDriver(
							LPCTSTR lpszDriverName,
							LPCTSTR lpszDriverPath,
							LPCTSTR lpszAltitude)
{
	TCHAR   szTempStr[MAX_PATH];
	HKEY    hKey;
	DWORD    dwData;
	TCHAR    szDriverImagePath[MAX_PATH];    

	if( NULL == lpszDriverName || NULL == lpszDriverPath )
	{
		return FALSE;
	}

	//得到完整的驱动路径
	GetFullPathName(
			lpszDriverPath,
			MAX_PATH,
			szDriverImagePath,
			NULL);

	SC_HANDLE hServiceMgr = NULL;
	SC_HANDLE hService = NULL;

	//打开服务控制管理器
	hServiceMgr = OpenSCManager( 
							NULL,
							NULL, 
							SC_MANAGER_ALL_ACCESS );

	if( hServiceMgr == NULL ) 
	{
		CloseServiceHandle(hServiceMgr);
		return FALSE;        
	}

	//创建驱动所对应的服务
	hService = CreateService( hServiceMgr,
		lpszDriverName,             // 驱动程序的在注册表中的名字
		lpszDriverName,             // 注册表驱动程序的DisplayName 值
		SERVICE_ALL_ACCESS,         // 加载驱动程序的访问权限
		SERVICE_FILE_SYSTEM_DRIVER, // 表示加载的服务是文件系统驱动程序
		SERVICE_DEMAND_START,       // 注册表驱动程序的Start 值
		SERVICE_ERROR_IGNORE,       // 注册表驱动程序的ErrorControl 值
		szDriverImagePath,          // 注册表驱动程序的ImagePath 值
		_T("FSFilter Activity Monitor"),// 注册表驱动程序的Group 值
		NULL, 
		_T("FltMgr"),                   // 注册表驱动程序的DependOnService 值
		NULL, 
		NULL);

	if( hService == NULL ) 
	{        
		if( GetLastError() == ERROR_SERVICE_EXISTS ) 
		{
			//服务创建失败，是由于服务已经创立过
			CloseServiceHandle(hService);       // 服务句柄
			CloseServiceHandle(hServiceMgr);    // SCM句柄
			return TRUE; 
		}
		else 
		{
			CloseServiceHandle(hService);       // 服务句柄
			CloseServiceHandle(hServiceMgr);    // SCM句柄
			return FALSE;
		}
	}
	CloseServiceHandle(hService);       // 服务句柄
	CloseServiceHandle(hServiceMgr);    // SCM句柄

	//-------------------------------------------------------------------------------------------------------
	// SYSTEM\\CurrentControlSet\\Services\\DriverName\\Instances子健下的键值项 
	//-------------------------------------------------------------------------------------------------------
	lstrcpy(szTempStr,_T("SYSTEM\\CurrentControlSet\\Services\\"));
	lstrcat(szTempStr,lpszDriverName);
	lstrcat(szTempStr,_T("\\Instances"));
	if(RegCreateKeyEx(
				HKEY_LOCAL_MACHINE,
				szTempStr,
				0,
				_T(""),
				TRUE,
				KEY_ALL_ACCESS,
				NULL,
				&hKey,
				(LPDWORD)&dwData)!=ERROR_SUCCESS)
	{
		return FALSE;
	}

	// 注册表驱动程序的DefaultInstance 值 
	lstrcpy(szTempStr,lpszDriverName);
	lstrcat(szTempStr,_T("Instance"));
	if(RegSetValueEx(
					hKey,
					_T("DefaultInstance"),
					0,
					REG_SZ,
					(CONST BYTE*)
					szTempStr,
					(DWORD)lstrlen(szTempStr)*sizeof(TCHAR))!=ERROR_SUCCESS)
	{
		return FALSE;
	}

	RegFlushKey(hKey);//刷新注册表
	RegCloseKey(hKey);
	//-------------------------------------------------------------------------------------------------------

	//-------------------------------------------------------------------------------------------------------
	// SYSTEM\\CurrentControlSet\\Services\\DriverName\\Instances\\DriverName Instance子健下的键值项 
	//-------------------------------------------------------------------------------------------------------
	lstrcpy(szTempStr,_T("SYSTEM\\CurrentControlSet\\Services\\"));
	lstrcat(szTempStr,lpszDriverName);
	lstrcat(szTempStr,_T("\\Instances\\"));
	lstrcat(szTempStr,lpszDriverName);
	lstrcat(szTempStr,_T("Instance"));

	if(RegCreateKeyEx(
			HKEY_LOCAL_MACHINE,
			szTempStr,
			0,
			_T(""),
			TRUE,
			KEY_ALL_ACCESS,
			NULL,
			&hKey,
			(LPDWORD)&dwData)!=ERROR_SUCCESS)
	{
		return FALSE;
	}

	// 注册表驱动程序的Altitude 值
	lstrcpy(szTempStr,lpszAltitude);
	if(RegSetValueEx(
				hKey,
				_T("Altitude"),
				0,
				REG_SZ,
				(CONST BYTE*)szTempStr,
				(DWORD)
				lstrlen(szTempStr)*sizeof(TCHAR))!=ERROR_SUCCESS)
	{
		return FALSE;
	}

	// 注册表驱动程序的Flags 值
	dwData = 0x0;
	if(RegSetValueEx(
					hKey,
					_T("Flags"),
					0,
					REG_DWORD,
					(CONST BYTE*)&dwData,
					sizeof(DWORD))!=ERROR_SUCCESS)
	{
		return FALSE;
	}

	RegFlushKey(hKey);//刷新注册表
	RegCloseKey(hKey);
	//-------------------------------------------------------------------------------------------------------

	return TRUE;
}

BOOL CSysmonDriverOpt::StartDriver(LPCTSTR lpszDriverName)
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;

	if( NULL == lpszDriverName)
	{
		return FALSE;
	}

	schManager = OpenSCManager(
							NULL,
							NULL,
							SC_MANAGER_ALL_ACCESS);

	if(NULL == schManager)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}

	schService = OpenService(
					schManager,
					lpszDriverName,
					SERVICE_ALL_ACCESS);

	if(NULL == schService)
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}

	if(!StartService(
			schService,
			0,
			NULL))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);

		DWORD dwError = GetLastError();
		if( dwError == ERROR_SERVICE_ALREADY_RUNNING ) 
		{             
			return TRUE;
		}

		TCHAR szError[50];
		wsprintf(szError,_T("错误：%d"),dwError);
		MessageBox(NULL,szError,szError,MB_OK);

		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}

BOOL CSysmonDriverOpt::StopDriver(LPCTSTR lpszDriverName)
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;
	bool            bStopped=false;

	schManager = OpenSCManager(
							NULL,
							NULL,
							SC_MANAGER_ALL_ACCESS);
	if(NULL == schManager)
	{
		return FALSE;
	}

	schService = OpenService(
						schManager,
						lpszDriverName,
						SERVICE_ALL_ACCESS);
	if(NULL == schService)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}

	if(!ControlService(
					schService,
					SERVICE_CONTROL_STOP,
					&svcStatus) &&
					(svcStatus.dwCurrentState != SERVICE_STOPPED))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}

BOOL CSysmonDriverOpt::DeleteDriver(LPCTSTR lpszDriverName)
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;

	schManager = OpenSCManager(
							NULL,
							NULL,
							SC_MANAGER_ALL_ACCESS);
	if(NULL == schManager)
	{
		return FALSE;
	}

	schService = OpenService(
						schManager,
						lpszDriverName,
						SERVICE_ALL_ACCESS);

	if(NULL == schService)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}

	ControlService(
				schService,
				SERVICE_CONTROL_STOP,
				&svcStatus);

	if(!DeleteService(schService))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}
