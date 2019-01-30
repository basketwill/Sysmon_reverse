#ifndef _CEResource_h
#define _CEResource_h

#include "cpplib.h"

class CEResource
{
public:
	CEResource(void);
	~CEResource(void);

	void Lock();
	void UnLock();

private:
	ERESOURCE			mEResoure;
	//volatile	LONG	mReference;
};

#endif