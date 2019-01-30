#ifndef _CIntNumber_h
#define _CIntNumber_h

#include "cpplib.h"

template<typename T>
class CIntNumber
{
public:
	CIntNumber(void)
	{
		mNumber = 0;
	}

	CIntNumber(T _X )
	{
		mNumber = _X;
	}

	CIntNumber(CIntNumber& _X )
	{
		mNumber = (T)_X;
	}

	~CIntNumber(void)
	{
		mNumber = 0;
	}

	operator T() const
	{
		return mNumber;
	}

	void
	operator ++()
	{
		mNumber++;
	}

	void
	operator =(T& _X)
	{
		mNumber = _X;
	}
	
	void
	operator =(CIntNumber& _X)
	{
		mNumber = (T)_X;
	}

 	BOOL
 	operator ==(T& _X)
 	{
 		return (mNumber == _X);
 	}

	BOOL
	operator ==(CIntNumber& _X)
	{
		return (mNumber == (T)_X);
	}

	BOOL
	operator <(T& _X)
	{
		return (BOOL)(mNumber < _X);
	}

	BOOL
		operator <=(T& _X)
	{
		return (mNumber <= _X);
	}

	BOOL
		operator >(T& _X)
	{
		return (mNumber > _X);
	}

	BOOL
		operator >=(T& _X)
	{
		return (mNumber >= _X);
	}

	T mNumber;
};

#endif