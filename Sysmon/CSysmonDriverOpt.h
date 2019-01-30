#ifndef _CSysmonDriverOpt_h
#define _CSysmonDriverOpt_h

class CSysmonDriverOpt
{
public:
	CSysmonDriverOpt(void);
	virtual ~CSysmonDriverOpt(void);
public:
	BOOL InstallDriver(
					LPCTSTR lpszDriverName,
					LPCTSTR lpszDriverPath,
					LPCTSTR lpszAltitude); 

	BOOL StartDriver(LPCTSTR lpszDriverName);

	BOOL StopDriver(LPCTSTR lpszDriverName);

	BOOL DeleteDriver(LPCTSTR lpszDriverName);
};

#endif