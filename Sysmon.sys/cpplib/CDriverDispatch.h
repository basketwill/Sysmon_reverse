#ifndef _CDriverDispatch_h
#define _CDriverDispatch_h
#include <cpplib.h>

class CDriverDispatch
{
public:
	CDriverDispatch(void);
	CDriverDispatch(const PDRIVER_DISPATCH* DriverDisptach);
	virtual ~CDriverDispatch(void);


	void operator =(const PDRIVER_DISPATCH* DriverDisptach);
	BOOL operator !() const;
	PDRIVER_DISPATCH& operator [](int n);
private:
	PDRIVER_DISPATCH* mDriverDisptach;
};

#endif