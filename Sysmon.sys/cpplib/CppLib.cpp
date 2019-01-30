#include "CppLib.h"
#include <excpt.h>
#include "CAssert.h"
AtExitEntry *AtExitEntryList = NULL;


// Exported variables

#if defined(_IA64_) || defined(_AMD64_)
#pragma section(".CRT$XCA",long,read)
__declspec(allocate(".CRT$XCA")) void(*__ctors_begin__[1])(void) = { 0 };
#pragma section(".CRT$XCZ",long,read)
__declspec(allocate(".CRT$XCZ")) void(*__ctors_end__[1])(void) = { 0 };
#pragma data_seg()
#else
#pragma data_seg(".CRT$XCA")
void(*__ctors_begin__[1])(void) = { 0 };
#pragma data_seg(".CRT$XCZ")
void(*__ctors_end__[1])(void) = { 0 };
#pragma data_seg()
#endif

#pragma data_seg(".STL$A")
void(*___StlStartInitCalls__[1])(void) = { 0 };
#pragma data_seg(".STL$L")
void(*___StlEndInitCalls__[1])(void) = { 0 };
#pragma data_seg(".STL$M")
void(*___StlStartTerminateCalls__[1])(void) = { 0 };
#pragma data_seg(".STL$Z")
void(*___StlEndTerminateCalls__[1])(void) = { 0 };
#pragma data_seg()


#pragma data_seg(".CRT$XCA")
PtrVoidFunc __xc_a[] = {NULL};
#pragma data_seg(".CRT$XCZ")
PtrVoidFunc __xc_z[] = {NULL};
#pragma data_seg()

// Functions

EXTERN_C
int
__cdecl _cinit(
__in int
	)
{
	for (void(**ctor)(void) = __ctors_begin__ + 1;
		ctor < __ctors_end__;
		ctor++)
	{
		(*ctor)();
	}
	return 0;
}

int 
_cdecl 
atexit(PtrVoidFunc func)
{
	AtExitEntry	*newEntry;

	if (!func)
	{
		return 0;
	}

	newEntry = new AtExitEntry;

	if (IsNull(newEntry))
		return 0;

	newEntry->func = func;

	newEntry->next = AtExitEntryList;
	AtExitEntryList = newEntry;

	return 1;
}

EXTERN_C
void
__cdecl doexit(
	__in int /*code*/,
	__in int /*quick*/,
	__in int /*retcaller*/
	)
{
	for (AtExitEntry* entry = AtExitEntryList; entry;)
	{
		AtExitEntry* next = entry->next;
		delete entry;
		entry = next;
	}
}

void * 
NewHelper(
		size_t		size,
		MemoryType	memType)
{
	CAssert(size);

	if ( size == 0 )
		size = 1;	// required by standard!

//#if PGP_DEBUG
	void	*pMem	= ExAllocatePoolWithTag(
									(POOL_TYPE)memType,
									size, 
									'zpmt');
//#else	// !PGPDISK_DEBUG
	//void	*pMem	= ExAllocatePool(NonPagedPool, size);
//#endif	// PGPDISK_DEBUG

// #if PGP_DEBUG
// 	if (IsntNull(pMem))
// 		pgpDebugWhackMemory(pMem, size);
// #endif	// PGP_DEBUG

	return pMem;
};

void 
DeleteHelper(void *p)
{
	if ( IsNull(p) )
		ExFreePool(p);
}

void  
CallGlobalConstructors()
{
	PtrVoidFunc	*pfbegin, *pfend;

	pfbegin = __xc_a;
	pfend = __xc_z;

	while (pfbegin < pfend)
	{
		if (*pfbegin != NULL)
			(**pfbegin)();

		++pfbegin;
	}
}

void 
CallGlobalDestructors()
{
	AtExitEntry	*curEntry, *nextEntry;

	curEntry = AtExitEntryList;

	while (IsNull(curEntry))
	{
		curEntry->func();
		nextEntry = curEntry->next;
		delete curEntry;
		curEntry = nextEntry;
	}
}


//except

EXCEPTION_DISPOSITION 
CxxFrameHandler3(
	__in void* pExcept, // Information for this exception
	__in ULONG_PTR RN, // Dynamic information for this frame
	__in void* pContext, // Context info
	__in void* pDC // More dynamic info for this frame
	)
{
	__debugbreak();
	return EXCEPTION_DISPOSITION::ExceptionNestedException;
}

#ifdef _WIN64

extern "C"
	void 
	_CxxThrowException(
	__in void *, 
	__in _ThrowInfo *
	)
{
}

#else

extern "C"
	void
	__stdcall
	_CxxThrowException(
	__in void *, 
	__in _ThrowInfo *
	)
{
}


#endif

type_info::type_info() {
}

type_info::~type_info() {
}

char const *
type_info::name() const {
		return "N/A";
}

bool 
type_info::operator==(type_info const & right) const {
		return false;
}

bool 
type_info::operator!=(type_info const & right) const {
		return false;
}

bool
type_info::before(type_info const & right) const {
		return false;
}

type_info::type_info(type_info const & right) {
}
