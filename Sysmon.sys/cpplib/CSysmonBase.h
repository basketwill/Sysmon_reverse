#ifndef _CSysmonBase_h
#define _CSysmonBase_h

#include <cpplib.h>
#include <fltKernel.h>

class CSysmonBase
{
public:
	CSysmonBase(void);
	~CSysmonBase(void);

public:
	static
	USHORT 
	SysmonIsNamedPipe(
					PCFLT_RELATED_OBJECTS pFltFileObj
					);

	static
	BOOL 
	SysmonFileIsCommonName(
					PFLT_CALLBACK_DATA CallbackData
					);

	static
	bool 
	SysmonQueryFileFileAttributes(
					PFLT_RELATED_OBJECTS pFltObj
					);
};

#endif