#ifndef _CPool_h
#define _CPool_h

#include "cpplib.h"
#include "CIntNumber.h"

template<typename T,
		POOL_TYPE PoolType = NonPagedPool,
		USHORT PoolTag = NON_PAGED_POOL_TAG,
		BOOLEAN AutoRelease = false>
class CPool
{
public:
	CPool(void)
	{
		mPool = NULL;
		mPoolSize = 0;
		mReference = 0;
	}

	CPool(const CPool& Pool )
	{
		mPool = Pool.mPool;
		mPoolSize = Pool.mPoolSize;
		mReference = Pool.mReference;
	}

	~CPool(void)
	{
		if (	mPool && 
				AutoRelease == true )
		{
			Free();
		}
	}

	operator T*() const
	{
		return (T*)mPool;
	}

	BOOL
	operator ==( CIntNumber<INT3264>& Pointer )
	{
		return (mPool == (T*)((INT3264)Pointer));
	}

	BOOL
	operator !=( CIntNumber<INT3264>& Pointer )
	{
		return (mPool != (T*) ((INT3264)Pointer));
	}

	const
	T&
	operator []( INT3264 Index) const
	{
		if ( Index < Size() &&
			Index >= 0 )
		{
			return ((T*)*this)[Index];
		}

		return  ((T*)*this)[0];
	}

	T&
	operator []( INT3264 Index )
	{
		if ( Index >= Size() ||
			Index < 0 )
		{
			return ((T*)*this)[0];
		}
		
		return ((T*)*this)[Index];
	}
	
// 	T&
// 	operator=( T& _E )
// 	{
// 		return _E;
// 	}

	CPool<T,PoolType,PoolTag,AutoRelease>&
	operator=( CPool<T,PoolType,PoolTag,AutoRelease>& _X )
	{
		mPool = (T*)_X;
		mPoolSize = _X.Size();
	}
	
	T* Allocate( SIZE_T _S )
	{

		if ( mPool == NULL )
		{
			mPool = static_cast<T*>(ExAllocatePoolWithTag(		
													PoolType,
													_S,
													PoolTag));

			if ( mPool )
			{
				mPoolSize = _S;
			}
		}

		return static_cast<T*>(mPool);
	}

	T* Allocate()
	{

		if ( mPool == NULL )
		{
			mPool = static_cast<T*>(ExAllocatePoolWithTag(		PoolType,
												sizeof(T),
												PoolTag));

			if ( mPool )
			{
				mPoolSize = sizeof(T);
			}
		}

		return mPool;
	}

	void Free()
	{
		if ( mPool )
		{
			ExFreePool(mPool);
			mPool = NULL;
			mPoolSize = 0;
		}
	}

	INT3264 Size()
	{
		return mPoolSize;
	}

private:

	T* mPool;
	ULONG_PTR mPoolSize;
	ULONG_PTR mReference;

};

#endif