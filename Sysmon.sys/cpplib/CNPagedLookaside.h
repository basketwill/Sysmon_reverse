#ifndef _CNPagedLookaside_h
#define _CNPagedLookaside_h

#include "cpplib.h"

template< typename T, USHORT PoolTag = NPAGE_LIST_POOL_TAG>
class CNPagedLookaside
{
public:
	CNPagedLookaside(void)
	{
		ExInitializeNPagedLookasideList(
			&mNPagedLookasside, 
			0,
			0,
			0,
			sizeof(T),
			PoolTag,
			0);
	}

	~CNPagedLookaside(void)
	{
		ExDeleteNPagedLookasideList(&mNPagedLookasside);
	}

	T* Allocate()
	{
		return static_cast<T*>(ExAllocateFromNPagedLookasideList(&mNPagedLookasside));
	}

	void Free( T* pT )
	{
		ExFreeToNPagedLookasideList(&mNPagedLookasside, pT);
	}

private:
	NPAGED_LOOKASIDE_LIST mNPagedLookasside;
};

#endif