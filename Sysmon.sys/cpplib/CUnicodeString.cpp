#include "CUnicodeString.h"

/**********************************************************************************/
#pragma LOCKEDCODE

CUnicodeString::CUnicodeString(void)
{
	mString.Buffer = NULL;
	mString.Length = 0;
	mString.MaximumLength = 0;
}

/**********************************************************************************/
#pragma LOCKEDCODE

CUnicodeString::CUnicodeString(const CUnicodeString& String)
{
	mString.Buffer = String.mString.Buffer;
	mString.Length = String.mString.Length;
	mString.MaximumLength = String.mString.MaximumLength;
}

/**********************************************************************************/
#pragma LOCKEDCODE

CUnicodeString::CUnicodeString(const PUNICODE_STRING& pString)
{

	if (pString)
	{
		__try
		{
			mString.Buffer = pString->Buffer;
			mString.Length = pString->Length;
			mString.MaximumLength = pString->MaximumLength;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{

		}
		
	}
	
}

/**********************************************************************************/
#pragma LOCKEDCODE

CUnicodeString::~CUnicodeString(void)
{
}

/**********************************************************************************/
#pragma LOCKEDCODE

PUNICODE_STRING CUnicodeString::operator &()
{
	return &mString;
}

/**********************************************************************************/
#pragma LOCKEDCODE

CUnicodeString::operator PUNICODE_STRING()
{
	return &mString;
}

/**********************************************************************************/
#pragma LOCKEDCODE

void CUnicodeString::operator =(const WCHAR* pString)
{
	if (pString)
	{
		RtlInitUnicodeString(&mString,pString);
	}
}

/**********************************************************************************/
#pragma LOCKEDCODE

void CUnicodeString::operator =(WCHAR* pString)
{
	if (pString)
	{
		RtlInitUnicodeString(&mString,pString);
	}
}