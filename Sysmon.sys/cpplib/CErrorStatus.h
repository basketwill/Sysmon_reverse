#ifndef _CErrorStatus_H
#define _CErrorStatus_H

#include "cpplib.h"

class CErrorStatus
{
public:
	CErrorStatus(void);
	~CErrorStatus(void);
	CErrorStatus( NTSTATUS Status  );
	CErrorStatus(const CErrorStatus& Error );

	void operator =( NTSTATUS Status );

	operator NTSTATUS()
	{
		return mNtStatus;
	}

	BOOL IsNtSuccess();

private:
	NTSTATUS mNtStatus;
	ULONG    mSystemError;
};

#endif