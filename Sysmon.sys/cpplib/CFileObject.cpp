#include "cpplib.h"
#include "CFileObject.h"


CFileObject::CFileObject(void)
	:mFileObj(NULL)
{
}

CFileObject::CFileObject(const CFileObject& File)
{
	mFileObj = mFileObj;
}

CFileObject::CFileObject(const PFILE_OBJECT& File)
{
	mFileObj = File;
}

CFileObject::~CFileObject(void)
{
}


CFileObject::operator PFILE_OBJECT()
{
	return mFileObj;
}

PVOID CFileObject::operator !()
{
	return (PVOID)!mFileObj;
}

PUNICODE_STRING CFileObject::FileName()
{
	return &mFileObj->FileName;
}

PFILE_OBJECT&    CFileObject::RelatedFileObject()
{
	return mFileObj->RelatedFileObject;
}