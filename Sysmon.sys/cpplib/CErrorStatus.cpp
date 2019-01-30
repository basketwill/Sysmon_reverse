#include "CErrorStatus.h"


CErrorStatus::CErrorStatus(void)
{
	mNtStatus = STATUS_UNSUCCESSFUL;
}

CErrorStatus::~CErrorStatus(void)
{
}

CErrorStatus::CErrorStatus( NTSTATUS Status  )
{
	mNtStatus = Status;
}

CErrorStatus::CErrorStatus(const CErrorStatus& Error )
{
	mNtStatus = Error.mNtStatus;
	mSystemError = Error.mSystemError;
}

void
CErrorStatus::operator =( NTSTATUS Status )
{
	mNtStatus = Status;
}

BOOL 
CErrorStatus::IsNtSuccess()
{
	return NT_SUCCESS(mNtStatus);
}