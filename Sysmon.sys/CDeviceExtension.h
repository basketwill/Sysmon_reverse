#ifndef _CDeviceExtension_h
#define _CDeviceExtension_h
#include <CppLib.h>

class CDeviceExtension
{
public:
	CDeviceExtension(void);
	virtual ~CDeviceExtension(void);
	operator PIO_CSQ();
	operator PRKEVENT();

	void Init();
private:
	IO_CSQ mCsq;
	KEVENT mFileEvent;
};

#endif