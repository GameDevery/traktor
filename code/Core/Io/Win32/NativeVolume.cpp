#include <iostream>
#include <sstream>
#if !defined(_XBOX) && !defined(WINCE)
#	include <direct.h>
#endif
#include "Core/Platform.h"
#include "Core/Io/Win32/NativeVolume.h"
#include "Core/Io/Win32/NativeStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace
	{

DateTime createDateTime(const FILETIME& ft)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft, &st);
	return DateTime(
		st.wYear,
		uint8_t(st.wMonth),
		st.wDay,
		uint8_t(st.wHour),
		uint8_t(st.wMinute),
		uint8_t(st.wSecond)
	);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeVolume", NativeVolume, IVolume)

NativeVolume::NativeVolume(const Path& currentDirectory)
:	m_currentDirectory(currentDirectory)
{
}

std::wstring NativeVolume::getDescription() const
{
#if defined(_XBOX) || defined(WINCE)

	return m_currentDirectory;

#else

	wchar_t volumeName[256] = { L"" };
	if (!GetVolumeInformation(
		m_currentDirectory.getVolume().c_str(),
		volumeName,
		sizeof_array(volumeName),
		NULL,
		NULL,
		NULL,
		NULL,
		0
	))
		return L"";

	return volumeName;

#endif
}

Ref< File > NativeVolume::get(const Path& path)
{
	RefArray< File > files;
	if (find(path, files) > 0)
		return files[0];
	else
		return 0;
}

int NativeVolume::find(const Path& mask, RefArray< File >& out)
{
	WIN32_FIND_DATA ffd;
	HANDLE hfd;

	hfd = FindFirstFile(
		wstots(getSystemPath(mask)).c_str(),
		&ffd
	);
	if (hfd != INVALID_HANDLE_VALUE)
	{
		std::wstring path = mask.getPathOnly();
		if (!path.empty())
			path += L'/';
		do
		{
			int flags =
				(((ffd.dwFileAttributes  & FILE_ATTRIBUTE_NORMAL   ) == FILE_ATTRIBUTE_NORMAL   ) ? File::FfNormal    : 0) |
				(((ffd.dwFileAttributes  & FILE_ATTRIBUTE_READONLY ) == FILE_ATTRIBUTE_READONLY ) ? File::FfReadOnly  : 0) |
				(((ffd.dwFileAttributes  & FILE_ATTRIBUTE_HIDDEN   ) == FILE_ATTRIBUTE_HIDDEN   ) ? File::FfHidden    : 0) |
				(((ffd.dwFileAttributes  & FILE_ATTRIBUTE_ARCHIVE  ) == FILE_ATTRIBUTE_ARCHIVE  ) ? File::FfArchive   : 0) |
				(((ffd.dwFileAttributes  & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) ? File::FfDirectory : 0);
			
			out.push_back(new File(
				Path(path + tstows(ffd.cFileName)),
				ffd.nFileSizeLow,
				flags,
				createDateTime(ffd.ftCreationTime),
				createDateTime(ffd.ftLastAccessTime),
				createDateTime(ffd.ftLastWriteTime)
			));
		}
		while (FindNextFile(hfd, &ffd));
		FindClose(hfd);
	}

	return int(out.size());
}

bool NativeVolume::modify(const Path& fileName, uint32_t flags)
{
	DWORD dwAttr = 0;

	if (flags & File::FfNormal)
		dwAttr |= FILE_ATTRIBUTE_NORMAL;
	if (flags & File::FfReadOnly)
		dwAttr |= FILE_ATTRIBUTE_READONLY;
	if (flags & File::FfHidden)
		dwAttr |= FILE_ATTRIBUTE_HIDDEN;
	if (flags & File::FfArchive)
		dwAttr |= FILE_ATTRIBUTE_ARCHIVE;

	std::wstring systemPath = getSystemPath(fileName);
	BOOL result = SetFileAttributes(
		wstots(systemPath).c_str(),
		dwAttr
	);

	return bool(result != FALSE);
}

Ref< IStream > NativeVolume::open(const Path& fileName, uint32_t mode)
{
	DWORD desiredAccess = 0, creationDisposition = 0;

	if (mode & File::FmRead)
	{
		desiredAccess |= GENERIC_READ;
		creationDisposition = OPEN_EXISTING;
	}
	if (mode & (File::FmWrite | File::FmAppend))
	{
		desiredAccess |= GENERIC_WRITE;
		creationDisposition = (mode & (File::FmRead | File::FmAppend)) ? OPEN_ALWAYS : CREATE_ALWAYS;
	}

	std::wstring systemPath = getSystemPath(fileName);

	HANDLE hFile = CreateFile(
		wstots(systemPath).c_str(),
		desiredAccess,
		FILE_SHARE_READ,
		NULL,
		creationDisposition,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	return (hFile != INVALID_HANDLE_VALUE) ? new NativeStream(hFile, mode) : 0;
}

bool NativeVolume::exist(const Path& fileName)
{
	WIN32_FIND_DATA ffd;
	HANDLE ffh;
	
	std::wstring systemPath = getSystemPath(fileName);

	if ((ffh = FindFirstFile(wstots(systemPath).c_str(), &ffd)) == INVALID_HANDLE_VALUE)
		return false;

	FindClose(ffh);
	return true;
}

bool NativeVolume::remove(const Path& fileName)
{
	std::wstring systemPath = getSystemPath(fileName);
	return bool(DeleteFile(wstots(systemPath).c_str()) == TRUE);
}

bool NativeVolume::rename(const Path& fileName, const std::wstring& newName)
{
	std::wstring sourceName = getSystemPath(fileName);
	std::wstring destinationName = getSystemPath(fileName.getPathOnly() + L"/" + newName);
	return bool(MoveFile(wstots(sourceName).c_str(), wstots(destinationName).c_str()) == TRUE);
}

bool NativeVolume::makeDirectory(const Path& directory)
{
	std::wstring systemPath = getSystemPath(directory);

	if (!CreateDirectory(wstots(systemPath).c_str(), NULL))
		return bool(GetLastError() == ERROR_ALREADY_EXISTS);

	return true;
}

bool NativeVolume::removeDirectory(const Path& directory)
{
	std::wstring systemPath = getSystemPath(directory);

	if (!RemoveDirectory(wstots(systemPath).c_str()))
		return false;

	return true;
}

bool NativeVolume::renameDirectory(const Path& directory, const std::wstring& newName)
{
	std::wstring sourceName = getSystemPath(directory);
	std::wstring destinationName = getSystemPath(directory.getPathOnly() + L"/" + newName);
	return bool(MoveFile(wstots(sourceName).c_str(), wstots(destinationName).c_str()) == TRUE);
}

bool NativeVolume::setCurrentDirectory(const Path& directory)
{
	if (directory.isRelative())
	{
		m_currentDirectory = m_currentDirectory + directory;
	}
	else
	{
		if (m_currentDirectory.getVolume() != directory.getVolume())
			return false;

		m_currentDirectory = directory;
	}
	return true;
}

Path NativeVolume::getCurrentDirectory() const
{
	return m_currentDirectory;
}

void NativeVolume::mountVolumes(FileSystem& fileSystem)
{
#if !defined(_XBOX) && !defined(WINCE)

	wchar_t mountPoint[] = { L"A" };
	wchar_t driveFormat[] = { L"A:" };

	wchar_t currentDirectory[_MAX_PATH];
	if (!GetCurrentDirectory(sizeof_array(currentDirectory), currentDirectory))
		return;

	for (uint32_t drives = _getdrives(), drive = 0; drives != 0; drives >>= 1, ++drive)
	{
		if ((drives & 1) == 0)
			continue;

		mountPoint[0] =
		driveFormat[0] = L'A' + drive;

		Ref< IVolume > volume = new NativeVolume(driveFormat);

		fileSystem.mount(mountPoint, volume);
		
		if ((L'A' + drive) == toupper(currentDirectory[0]))
		{
			volume->setCurrentDirectory(currentDirectory);
			fileSystem.setCurrentVolume(volume);
		}
	}

#elif defined(_XBOX)

	Ref< IVolume > volume = new NativeVolume(L"D:");
	fileSystem.mount(L"D", volume);
	fileSystem.setCurrentVolume(volume);

#elif defined(WINCE)

	wchar_t moduleName[MAX_PATH];
	GetModuleFileName(NULL, moduleName, MAX_PATH);

	Path originalDirectory(moduleName);
	log::info << L"Using \"" << originalDirectory.getPathOnly() << L"\" as original directory" << Endl;

	Ref< IVolume > volume = new NativeVolume(originalDirectory.getPathOnly());
	fileSystem.mount(L"C", volume);
	fileSystem.setCurrentVolume(volume);

#endif
}

std::wstring NativeVolume::getSystemPath(const Path& path) const
{
	std::wstringstream ss;

	if (path.hasVolume())
	{
		T_ASSERT (path.getVolume() == m_currentDirectory.getVolume());
		ss << path.getPathName();
	}
	else
	{
		if (path.isRelative())
			ss << m_currentDirectory.getPathName() << L"/" << path.getPathName();
		else
			ss << m_currentDirectory.getVolume() << L":" << path.getPathName();
	}

	return replaceAll(ss.str(), L'/', L'\\');
}

}
