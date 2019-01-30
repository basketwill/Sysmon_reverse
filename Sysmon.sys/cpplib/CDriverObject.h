#ifndef _CDriverObject_h
#define _CDriverObject_h
#include "cpplib.h"

class CDriverObject
{
public:
	CDriverObject(void);
	CDriverObject(PDRIVER_OBJECT DriverObject);
	CDriverObject(const CDriverObject& DriverObject);
	~CDriverObject(void);
public:

	operator const PDRIVER_OBJECT() const;
private:
	PDRIVER_OBJECT mDriverObject;
};

#endif