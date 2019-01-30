#include "CDriverDispatch.h"


CDriverDispatch::CDriverDispatch(void)
	:mDriverDisptach(NULL)
{
}

CDriverDispatch::CDriverDispatch(const PDRIVER_DISPATCH* DriverDisptach)
{
	mDriverDisptach = (PDRIVER_DISPATCH*)DriverDisptach;
}

CDriverDispatch::~CDriverDispatch(void)
{
}

void CDriverDispatch::operator =(const PDRIVER_DISPATCH* DriverDisptach)
{
	mDriverDisptach = (PDRIVER_DISPATCH*)DriverDisptach;
}

BOOL CDriverDispatch::operator !() const
{
	return mDriverDisptach != NULL;
}

PDRIVER_DISPATCH& CDriverDispatch::operator [](int n)
{
	return mDriverDisptach[n];
}