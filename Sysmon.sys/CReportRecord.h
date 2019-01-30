#ifndef _CReportRecord_h
#define _CReportRecord_h

#include <cpplib.h>
#include <CListEntry.h>


class CSysmonRecord
{
public:
	CSysmonRecord();
	~CSysmonRecord();

private:
	USHORT mType;
};

class CReportSysmonRecord
{
public:
	CReportSysmonRecord(void);
	~CReportSysmonRecord(void);

private:
	CListEntry<CSysmonRecord*> mReportRecord;
};

#endif