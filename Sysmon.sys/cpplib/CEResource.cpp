#include "CEResource.h"


CEResource::CEResource(void)
{
	//mReference = 0;
	ExInitializeResource(&mEResoure);
}

CEResource::~CEResource(void)
{	
	ExDeleteResource(&mEResoure);		
}

void 
CEResource::Lock()
{	
	ExAcquireResourceExclusive(&mEResoure,true);		
}

void 
CEResource::UnLock()
{	
	ExReleaseResource(&mEResoure);	
}
