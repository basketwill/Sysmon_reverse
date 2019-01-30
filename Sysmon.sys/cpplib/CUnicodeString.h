#ifndef _CUnicodeString_h
#define _CUnicodeString_h


#include <CppLib.h>


class CUnicodeString
{
public:
	CUnicodeString(void);
	CUnicodeString(const CUnicodeString& String);
	CUnicodeString(const PUNICODE_STRING& pString);
	virtual ~CUnicodeString(void);

	PUNICODE_STRING operator &();

	operator PUNICODE_STRING();

	void operator =(const WCHAR* pString);
	void operator =(WCHAR* pString);
private:
	UNICODE_STRING mString;
};

#endif