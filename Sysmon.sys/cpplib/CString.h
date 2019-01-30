#ifndef _CStringT_h
#define _CStringT_h
#include <CppLib.h>

template<typename T, int COUNT>
class CStringT
{
public:
	CStringT(void)
	{

	}

	virtual ~CStringT(void)
	{

	}

	operator T*()
	{
		return pT;
	}

protected:
	T pT[COUNT];
};


class CStringW: public CStringT<WCHAR*,1>
{
public:
	CStringW()
	{
		memset(this,0,sizeof(*this));
	}

	~CStringW()
	{

	}
private:
	USHORT mLength;
	USHORT mMaxLength;
};

template<int COUNT>
class CStackStringW : public CStringT<WCHAR,COUNT>
{
public:
	CStackStringW()
	{

	}

	~CStackStringW()
	{

	}

	WCHAR& operator [](int n)
	{
		return pT[n];
	}

	operator WCHAR*()
	{
		return &pT[0];
	}

	WCHAR* operator &()
	{
		return &pT[0];
	}

};



#endif
