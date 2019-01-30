#ifndef _CDebug_h
#define _CDebug_h
#include "cpplib.h"

class CDebug
{
public:
	typedef enum {kMaxLengthString = 256};
	CDebug(void);
	~CDebug(void);

#ifdef DEBUG

	static
	void DebugOut( const CHAR* formatStr, ... );
#else
	static
	inline 
	void DebugOut( const CHAR* formatStr, ... )
	{

	}

#endif

};

#endif