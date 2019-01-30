#ifndef _CPP_LIB_h
#define _CPP_LIB_h
#include <ntifs.h>
#include <strsafe.h>

typedef char CHAR;
typedef unsigned char UCHAR;
typedef unsigned char BOOLEAN;
typedef wchar_t WCHAR;
typedef unsigned char boolean;
typedef unsigned __int32 UINT32;
typedef unsigned int ULONG32, *PULONG32;
typedef unsigned int DWORD32, *PDWORD32;
typedef int BOOL;
typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;
typedef signed __int64      INT64, *PINT64;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef unsigned __int64    UINT64, *PUINT64;

typedef __int64  INT64;
typedef __int3264 ADDRPOINT;
typedef __int3264 INT3264;
typedef unsigned int UINT;
typedef unsigned char BYTE;
typedef unsigned long ULONG;
typedef short SHORT;
typedef unsigned short USHORT;


#define IsNull(a) (a == NULL)

#define PAGEDCODE	code_seg("PAGE")
#define LOCKEDCODE	code_seg()
#define INITCODE	code_seg("INIT")


#ifndef __cplusplus
extern "C"{
#endif

	// Types

typedef void (_cdecl *PtrVoidFunc)(void);

struct AtExitEntry
{
	PtrVoidFunc func;
	AtExitEntry *next;
};

// Static variables

int 
_cdecl 
atexit(PtrVoidFunc func);

#ifndef __cplusplus
};
#endif

typedef enum MemoryType
{
	kDefaultMemType = NonPagedPool,
};

void * 
NewHelper(
	size_t		size,
	MemoryType	memType);

void 
DeleteHelper(void *p);

inline 
void*
_cdecl 
operator new(size_t size)
{
	return NewHelper(size,kDefaultMemType);
}

inline 
void * 
_cdecl 
operator new[](size_t size)
{
	return NewHelper(size,kDefaultMemType);
}

inline 
void * 
_cdecl 
	operator new(size_t size, MemoryType memType)
{
	return NewHelper(size, memType);
}

inline 
	void * 
	_cdecl 
	operator new[](size_t size, MemoryType memType)
{
	return NewHelper(size, memType);
}

// Placement new.

inline 
void * 
_cdecl 
operator new(size_t size, void *pMem)
{
	(void) size;
	return pMem;
}

inline 
	void * 
	_cdecl 
	operator new[](size_t size, void *pMem)
{
	(void) size;
	return pMem;
}


// Operator delete functions

inline 
	void 
	_cdecl 
	operator delete(void *p)
{ 
	DeleteHelper(p);
}

inline 
	void 
	_cdecl 
	operator delete[](void *p)
{ 
	DeleteHelper(p);
}

inline 
	void 
	_cdecl 
	operator delete(void *p, MemoryType memType)
{ 
	(void) memType;
	DeleteHelper(p);
}

inline 
	void 
	_cdecl 
	operator delete[](void *p, MemoryType memType)
{ 
	(void) memType;
	DeleteHelper(p);
}


// Placement delete.

inline 
void 
_cdecl 
operator delete(void *pMem1, void *pMem2)
{
	(void) pMem1;
	(void) pMem2;

	return;
}

inline 
void 
_cdecl 
operator delete[](void *pMem1, void *pMem2)
{
	(void) pMem1;
	(void) pMem2;
	return;
}

void  
	CallGlobalConstructors();
void 
	CallGlobalDestructors();

extern "C"
EXCEPTION_DISPOSITION 
CxxFrameHandler3(
	__in void* pExcept, // Information for this exception
	__in ULONG_PTR RN, // Dynamic information for this frame
	__in void* pContext, // Context info
	__in void* pDC // More dynamic info for this frame
	);


class type_info {
public:
	type_info();
	virtual ~type_info();

	char const * name() const;

	bool operator==(type_info const & right) const;
	bool operator!=(type_info const & right) const;
	bool before(type_info const & right) const;

private:
	type_info(type_info const & right);
	type_info & operator=(type_info const & right);
};

#endif