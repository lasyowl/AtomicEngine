#pragma once

#include "Singleton.h"

enum
{
	FileSysLoad_Relative	= 0x01,
	FileSysLoad_Recursive	= 0x02,
	FileSysLoad_Default		= FileSysLoad_Relative | FileSysLoad_Recursive
};

struct SFileMetaData
{
	std::wstring FullName;
	std::vector<SFileMetaData> Children;
};

class CFileSystem : public TSingleton<CFileSystem>
{
public:
	void LoadDirectory( const std::wstring& InFilePath, int32 FileSysLoadFlag = FileSysLoad_Default );
};