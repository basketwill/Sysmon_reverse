#ifndef _CSysmonUtil_h
#define _CSysmonUtil_h

typedef unsigned __int64 QWORD;

class CSysmonUtil
{
public:
	CSysmonUtil(void);
	~CSysmonUtil(void);

public:
	static
	BOOL 
	SysmonVersionIsSupport();

	static
	unsigned __int64 CounterTimes(FILETIME t1,__int64 t2);
};

#endif