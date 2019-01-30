#ifndef _CIoStackLocation_h
#define _CIoStackLocation_h

#include "CFileObject.h"

class CIoStackLocation
{
public:
	CIoStackLocation(void);
	CIoStackLocation(const CIoStackLocation& Stack);
	CIoStackLocation(const PIO_STACK_LOCATION& Stack);
	~CIoStackLocation(void);

	operator PIO_STACK_LOCATION();

	void operator =(const PIO_STACK_LOCATION Stack);
	BOOLEAN operator ==(const PIO_STACK_LOCATION Stack);
	BOOLEAN operator !=(const PIO_STACK_LOCATION Stack);
	PVOID operator !();

	CFileObject FileObject();
	UCHAR&		Flags();

private:
	PIO_STACK_LOCATION mStack;
};

#endif