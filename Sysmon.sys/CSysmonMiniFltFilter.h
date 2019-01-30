#ifndef _CSysmonMiniFltFilter_h
#define _CSysmonMiniFltFilter_h

#include <cminifltfilter.h>
#include <CListEntry.h>
#include <CppLib.h>
#include <CErrorStatus.h>

class CSysmonMiniFltFilter :
	public CMiniFltFilter
{
public:
	CSysmonMiniFltFilter(void);
	~CSysmonMiniFltFilter(void);

	CErrorStatus Init(PDRIVER_OBJECT DriverObj);

public:
	static
		NTSTATUS 
		NTAPI
		FilterUnloadCallback(
		FLT_FILTER_UNLOAD_FLAGS Flags
		);

	static
		NTSTATUS
		NTAPI
		InstanceSetupCallback(
		PCFLT_RELATED_OBJECTS FltObjects,
		FLT_INSTANCE_SETUP_FLAGS Flags,
		DEVICE_TYPE VolumeDeviceType,
		FLT_FILESYSTEM_TYPE VolumeFilesystemType
		);

	static
		NTSTATUS
		NTAPI
		InstanceQueryTeardownCallback(
		PCFLT_RELATED_OBJECTS FltObjects,
		FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
		);

private:
//	CListEntry	mListFile;
};

#endif