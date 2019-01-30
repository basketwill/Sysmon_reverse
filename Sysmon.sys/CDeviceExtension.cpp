#include "CDeviceExtension.h"

/**********************************************************************************/
#pragma LOCKEDCODE

CDeviceExtension::CDeviceExtension(void)
{
}


/**********************************************************************************/
#pragma LOCKEDCODE

CDeviceExtension::~CDeviceExtension(void)
{
}

/**********************************************************************************/
#pragma PAGEDCODE

CDeviceExtension::operator PIO_CSQ()
{
	return &mCsq;
}

/**********************************************************************************/
#pragma PAGEDCODE

CDeviceExtension::operator PRKEVENT()
{
	return &mFileEvent;
}


/**********************************************************************************/
#pragma INITCODE

void CDeviceExtension::Init()
{
	KeInitializeEvent(
			&mFileEvent,
			SynchronizationEvent,
			0);
}