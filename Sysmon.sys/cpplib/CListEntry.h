#ifndef _CListEntry_H
#define _CListEntry_H

#include "cpplib.h"
#include "CEResource.h"

#define LIST_COMMON_TAG 'Aflt'

typedef struct _C_LIST_NODE
{
	LIST_ENTRY Entry;
	void*      Elment;

}C_LIST_NODE;

template<	typename T,
			BOOLEAN IsHead = false,
			unsigned long PoolTag = LIST_COMMON_TAG >

class CListEntry
{
public:

	class IteratorType
	{
		friend class CListEntry;
	public:
		IteratorType()
		{
			mPl = NULL;
			mPt = NULL;
		}

		IteratorType(const IteratorType& _T )
		{
			mPt = _T.mPt;
			mPl = _T.mPl;
		}

		~IteratorType()
		{
		}

	private:
		LIST_ENTRY* mPl;
		CListEntry* mPt;
	};

	class Iterator
	{
		friend class CListEntry;
		friend class IteratorType;
	public:
		Iterator()
		{
			mT.mPl = NULL;
			mT.mPt = NULL;
		}

		Iterator( const Iterator& _I )
		{
			mT.mPl = _I.mT.mPl;
			mT.mPt = _I.mT.mPt;
		}

		~Iterator()
		{
			mT.mPl = NULL;

			if ( mT.mPt )
			{
				mT.mPt->UnLock();
			}
		}

	public:

		void
		operator++()
		{
			if ( mT.mPl )
			{
				mT.mPl = mT.mPl->Flink;
			}
		}

		void operator--()
		{
			if ( mT.mPl )
			{
				mT.mPl->Blink;
			}
		}

		T&
		operator *()
		{
			C_LIST_NODE* Node;
			Node = CONTAINING_RECORD(
							mT.mPl,
							C_LIST_NODE,
							Entry);	
			
			return *((T*)Node->Elment);
		}

		BOOLEAN
		operator !=(Iterator& _I)
		{
			return (this != &_I);
		}

		BOOLEAN
		operator !=( LIST_ENTRY* _X )
		{
			return (mT.mPl != _X);
		}

		void
		operator =( IteratorType& _X )
		{
			mT.mPl = _X.mPl;
			mT.mPt = _X.mPt;
		}

	private:
		IteratorType mT;

	};

	CListEntry()
	{
		InitializeListHead( &mAfdList);

		mIteratorType.mPl = mAfdList.Flink;
		mIteratorType.mPt = this;

		mMaxSize = -1;
		mSize = 0;
	}

	~CListEntry()
	{

	}

	BOOLEAN push_front( T* _X )
	{
		C_LIST_NODE* Entry;

		Entry = (C_LIST_NODE*)
			ExAllocatePoolWithTag(
						NonPagedPool,
						sizeof(C_LIST_NODE),
						PoolTag);

		if ( Entry )
		{
			Lock();

			__try
			{
				Entry->Elment = (void*)_X;
				
				InsertHeadList(	&mAfdList,
						&Entry->Entry);

			}
			__finally
			{
				UnLock();
			}				

			return true;
		}

		return false;		
	}

	BOOLEAN push_back( T* _X )
	{
		C_LIST_NODE* Entry;

		Entry = (C_LIST_NODE*)
			ExAllocatePoolWithTag(
						NonPagedPool,
						sizeof(C_LIST_NODE),
						PoolTag);

		if ( Entry )
		{
			Lock();

			__try
			{
				Entry->Elment = (void*)_X;

				InsertTailList(	&mAfdList,
								&Entry->Entry);

			}
			__finally
			{
				UnLock();
			}				

			return true;
		}

		return false;		
	}

	BOOLEAN Insert( T* _X )
	{
		if ( IsHead == true )
		{
			return push_front(	_X );
		}
		else
		{
			return push_back(_X);
		}

		return false;
	}

	//删除指定的迭代器
	IteratorType& erase(Iterator& _I )
	{
		IteratorType _T;
		C_LIST_NODE* Node;

		__try
		{
			_T.mPt = _I.mT.mPt;

			Node = CONTAINING_RECORD( 
				_I.mT.mPl ,
				C_LIST_NODE,
				Entry);

			if ( Node )
			{			
				RemoveEntryList(_I.mT.mPl);
				ExFreePool(Node);
				Node = NULL;
			}

			_T.mPl = _I.mT.mPl;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{

		}		
	
		return _T;
	}

	BOOLEAN Remove( T* _X )
	{
		LIST_ENTRY* Entry;
		C_LIST_NODE* Node;
		T* Elment;

		Lock();

		__try
		{
			__try
			{
				Entry = &mAfdList;

				if( !IsListEmpty(Entry))
				{
					while( Entry->Flink != &mAfdList )
					{
						Node = CONTAINING_RECORD( 
											Entry ,
											C_LIST_NODE,
											Entry);

						if ( Node )
						{
							if ( Node->Elment == _X )
							{
								RemoveEntryList(Entry);

								ExFreePool(Node);

								Node = NULL;

								break;
							}
						}

						Entry = Entry->Flink;
					}
					
				}
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				return false;
			}
		}		
		__finally
		{
			UnLock();
		}
		
		return true;
	}

	IteratorType& Begin()
	{
		Lock();

		mIteratorType.mPl		= mAfdList.Flink;
		mIteratorType.mPt       = this;

		return mIteratorType;
	}

	LIST_ENTRY* End()
	{
		return &mAfdList;
	}

private:

	void Lock()
	{
		mESourceLock.Lock();
	}

	void UnLock()
	{
		mESourceLock.UnLock();
	}

	IteratorType	mIteratorType;
	LIST_ENTRY		mAfdList;
	ULONG			mMaxSize;
	ULONG			mSize;
	CEResource	mESourceLock;
};

#endif