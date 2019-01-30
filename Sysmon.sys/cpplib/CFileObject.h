#ifndef _CFileObject_h
#define _CFileObject_h

class CFileObject
{
public:
	CFileObject(void);
	CFileObject(const CFileObject& File);
	CFileObject(const PFILE_OBJECT& File);
	~CFileObject(void);

	operator PFILE_OBJECT();

	PVOID operator !();

	PUNICODE_STRING FileName();

	PFILE_OBJECT&    RelatedFileObject();

private:
	PFILE_OBJECT mFileObj;
};

#endif