#pragma once

#include "resource.h"

BOOLEAN
	FORCEINLINE
	IsListEmpty(
	__in const LIST_ENTRY * ListHead
	)
{
	return (BOOLEAN)(ListHead->Flink == ListHead);
}

FORCEINLINE
	VOID
	InitializeListHead(
	__out PLIST_ENTRY ListHead
	)
{
	ListHead->Flink = ListHead->Blink = ListHead;
}

FORCEINLINE
	VOID
	InsertTailList(
	__inout PLIST_ENTRY ListHead,
	__inout __drv_aliasesMem PLIST_ENTRY Entry
	)
{
	PLIST_ENTRY Blink;

	Blink = ListHead->Blink;
	Entry->Flink = ListHead;
	Entry->Blink = Blink;
	Blink->Flink = Entry;
	ListHead->Blink = Entry;
}

FORCEINLINE
	BOOLEAN
	RemoveEntryList(
	__in PLIST_ENTRY Entry
	)
{
	PLIST_ENTRY Blink;
	PLIST_ENTRY Flink;

	Flink = Entry->Flink;
	Blink = Entry->Blink;
	Blink->Flink = Flink;
	Flink->Blink = Blink;
	return (BOOLEAN)(Flink == Blink);
}

FORCEINLINE
	PLIST_ENTRY
	RemoveHeadList(
	__inout PLIST_ENTRY ListHead
	)
{
	PLIST_ENTRY Flink;
	PLIST_ENTRY Entry;

	Entry = ListHead->Flink;
	Flink = Entry->Flink;
	ListHead->Flink = Flink;
	Flink->Blink = ListHead;
	return Entry;
}

DWORD RunSysmonX64();
