#include "stdafx.h"
#include "FileSystem.h"

#include <filesystem>

namespace stdfs = std::filesystem;

void CFileSystem::LoadDirectory( const std::wstring& InFilePath, int32 FileSysLoadFlag )
{
	stdfs::path RootPath = stdfs::current_path();
	for ( stdfs::directory_iterator Iter( RootPath ); Iter != stdfs::end( Iter ); ++Iter )
	{
		const stdfs::directory_entry& Entry = *Iter;
		//std::cout << Entry.path() << std::endl;
	}
}