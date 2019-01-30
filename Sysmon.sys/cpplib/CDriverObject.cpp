
#include "CDriverObject.h"


CDriverObject::CDriverObject(void)
{
}

CDriverObject::CDriverObject(PDRIVER_OBJECT DriverObject)
{
	mDriverObject = DriverObject;
}

CDriverObject::CDriverObject(const CDriverObject& DriverObject)
{
	mDriverObject = DriverObject.mDriverObject;
}

CDriverObject::~CDriverObject(void)
{
}


CDriverObject::operator const PDRIVER_OBJECT() const
{
	return mDriverObject;
}