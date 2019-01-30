#include "cpplib.h"
#include "CDebug.h"
#include <stdarg.h>
#include <stdio.h>


CDebug::CDebug(void)
{
}


CDebug::~CDebug(void)
{
}

#ifdef DEBUG

void
CDebug::DebugOut( const CHAR* formatStr, ... )
{
	CHAR	StrBuf[kMaxLengthString];
	va_list	ArgList;
	memset( StrBuf , 0 , kMaxLengthString );
	va_start(ArgList, formatStr);
	_vsnprintf(
			StrBuf,
			kMaxLengthString, 
			formatStr,
			ArgList);
	va_end(ArgList);

	KdPrint(("[AfdFilter]:"));
	KdPrint((StrBuf));
	KdPrint(("\n"));
}

#endif