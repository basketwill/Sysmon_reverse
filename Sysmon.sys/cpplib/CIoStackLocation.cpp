#include "cpplib.h"
#include "CIoStackLocation.h"


CIoStackLocation::CIoStackLocation(void)
	:mStack(NULL)
{
}

CIoStackLocation::CIoStackLocation(
				const CIoStackLocation& Stack)
{
	mStack = Stack.mStack;
}

CIoStackLocation::CIoStackLocation(const PIO_STACK_LOCATION& Stack)
{
	mStack = Stack;
}

CIoStackLocation::~CIoStackLocation(void)
{
}


CIoStackLocation::operator PIO_STACK_LOCATION()
{
	return mStack;
}

void
CIoStackLocation::operator =(const PIO_STACK_LOCATION Stack)
{
	mStack =Stack;
}

BOOLEAN 
CIoStackLocation::operator ==(const PIO_STACK_LOCATION Stack)
{
	return ( mStack == Stack?true:false); 
}

BOOLEAN CIoStackLocation::operator !=(const PIO_STACK_LOCATION Stack)
{
	return (mStack != Stack?true:false);
}

PVOID CIoStackLocation::operator !()
{
	return ((PVOID)!mStack);
}

CFileObject CIoStackLocation::FileObject()
{
	return mStack->FileObject;
}

UCHAR&	CIoStackLocation::Flags()
{
	return mStack->Flags;
}