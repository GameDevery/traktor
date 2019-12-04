#include <sstream>
#include "Compress/Zip/DeflateStreamZip.h"
#include "Compress/Zip/ZipVolume.h"
#include "Core/Io/File.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/WildCompare.h"

namespace traktor
{
	namespace compress
	{
		namespace
		{

#pragma pack(1)
struct EOCD
{
	uint32_t signature;
	uint16_t disk;
	uint16_t diskOfCD;
	uint16_t numberOfCDRecords;
	uint16_t totalNumberOfCDRecords;
	uint32_t cdSize;
	uint32_t cdOffset;
	uint16_t commentLength;
	// uint8_t comment[commentLength]
};

struct CDFH
{
	uint32_t signature;
	uint16_t version;
	uint16_t requiredVersion;
	uint16_t generalFlags;
	uint16_t compression;
	uint16_t lastModificationTime;
	uint16_t lastModificationDate;
	uint32_t crc32;
	uint32_t compressedSize;
	uint32_t uncompressedSize;
	uint16_t fileNameLength;
	uint16_t extraFieldLength;
	uint16_t commentLength;
	uint16_t startDisk;
	uint16_t internalAttributes;
	uint32_t externalAttributes;
	uint32_t lfhOffset;
	// uint8_t fileName[fileNameLength]
	// uint8_t extraField[extraFieldLength]
	// uint8_t comment[commentLength]
};

struct LFH
{
	uint32_t signature;
	uint16_t version;
	uint16_t generalFlags;
	uint16_t compression;
	uint16_t lastModificationTime;
	uint16_t lastModificationDate;
	uint32_t crc32;
	uint32_t compressedSize;
	uint32_t uncompressedSize;
	uint16_t fileNameLength;
	uint16_t extraFieldLength;
	// uint8_t fileName[fileNameLength]
	// uint8_t extraField[extraFieldLength]
};

#pragma pack()

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.compress.ZipVolume", ZipVolume, IVolume)

ZipVolume::ZipVolume(IStream* zipFile)
:	m_zipFile(zipFile)
{
	char fileName[4096];
	char extra[4096];
	char comment[4096];
	EOCD eocd = {};

	// Search for EOCD signature from the end of file.
	uint64_t size = m_zipFile->available();
	if (size < sizeof(EOCD))
	{
		log::error << L"Corrupt ZIP file; no data." << Endl;
		return;
	}

	uint64_t search = size - sizeof(EOCD);
	do
	{
		m_zipFile->seek(IStream::SeekSet, search);
		if (m_zipFile->read(&eocd.signature, 4) != 4)
		{
			log::error << L"Corrupt ZIP file; failed to find EOCD signature, read failed." << Endl;
			return;
		}
		if (eocd.signature == 0x06054b50)
			break;
		search--;
	}
	while (search >= 0);

	if (eocd.signature != 0x06054b50)
	{
		log::error << L"Corrupt ZIP file; failed to find EOCD signature." << Endl;
		return;
	}

	// Read rest of EOCD header.
	m_zipFile->read(&eocd.disk, sizeof(EOCD) - sizeof(uint32_t));

	// Read central directory.
	m_zipFile->seek(IStream::SeekSet, eocd.cdOffset);
	for (uint32_t i = 0; i < eocd.numberOfCDRecords; ++i)
	{
		CDFH cdfh = {};

		if (m_zipFile->read(&cdfh, sizeof(CDFH)) != sizeof(CDFH))
		{
			log::error << L"Corrupt ZIP file; failed to find CDFH." << Endl;
			return;
		}

		if (cdfh.signature != 0x02014b50)
		{
			log::error << L"Corrupt ZIP file; incorrect CDFH signature." << Endl;
			return;
		}

		if (
			cdfh.fileNameLength >= sizeof_array(fileName) ||
			cdfh.extraFieldLength >= sizeof_array(fileName) ||
			cdfh.commentLength >= sizeof_array(fileName)
		)
		{
			log::error << L"Corrupt ZIP file; too long strings in header." << Endl;
			return;
		}

		m_zipFile->read(fileName, cdfh.fileNameLength);
		fileName[cdfh.fileNameLength] = '\0';

		m_zipFile->read(extra, cdfh.extraFieldLength);
		extra[cdfh.extraFieldLength] = '\0';

		m_zipFile->read(comment, cdfh.commentLength);
		comment[cdfh.commentLength] = '\0';

		auto& fi = m_fileInfo[L"/" + mbstows(fileName)];
		fi.offset = cdfh.lfhOffset;
		fi.compressedSize = cdfh.compressedSize;
		fi.uncompressedSize = cdfh.uncompressedSize;
	}
}

std::wstring ZipVolume::getDescription() const
{
	return L"zip";
}

Ref< File > ZipVolume::get(const Path& path)
{
	return nullptr;
}

int ZipVolume::find(const Path& mask, RefArray< File >& out)
{
	std::wstring maskPath = mask.getPathOnly();
	std::wstring systemPath = getSystemPath(maskPath);
	std::wstring fileMask = mask.getFileName();

	if (fileMask == L"*.*")
		fileMask = L"*";

	WildCompare maskCompare(systemPath + fileMask);
	for (auto fi : m_fileInfo)
	{
		if (maskCompare.match(fi.first))
		{
			out.push_back(new File(
				fi.first,
				fi.second.uncompressedSize,
				File::FfNormal
			));
		}
	}

	return (int)out.size();
}

bool ZipVolume::modify(const Path& fileName, uint32_t flags)
{
	return false;
}

Ref< IStream > ZipVolume::open(const Path& fileName, uint32_t mode)
{
	char fileNameTmp[4096];
	char extra[4096];

	auto it = m_fileInfo.find(fileName.getPathNameNoVolume());
	if (it == m_fileInfo.end())
		return nullptr;

	const auto& fi = it->second;

	m_zipFile->seek(IStream::SeekSet, fi.offset);

	LFH lfh = {};

	if (m_zipFile->read(&lfh, sizeof(LFH)) != sizeof(LFH))
	{
		log::error << L"Corrupt ZIP file; failed to find LFH." << Endl;
		return nullptr;
	}

	if (lfh.signature != 0x04034b50)
	{
		log::error << L"Corrupt ZIP file; incorrect LFH signature." << Endl;
		return nullptr;
	}

	if (
		lfh.fileNameLength >= sizeof_array(fileNameTmp) ||
		lfh.extraFieldLength >= sizeof_array(extra)
	)
	{
		log::error << L"Corrupt ZIP file; too long strings in header." << Endl;
		return nullptr;
	}

	m_zipFile->read(fileNameTmp, lfh.fileNameLength);
	fileNameTmp[lfh.fileNameLength] = '\0';

	m_zipFile->read(extra, lfh.extraFieldLength);
	extra[lfh.extraFieldLength] = '\0';

	return new DeflateStreamZip(m_zipFile);
}

bool ZipVolume::exist(const Path& fileName)
{
	return false;
}

bool ZipVolume::remove(const Path& fileName)
{
	return false;
}

bool ZipVolume::move(const Path& fileName, const std::wstring& newName, bool overwrite)
{
	return false;
}

bool ZipVolume::copy(const Path& fileName, const std::wstring& newName, bool overwrite)
{
	return false;
}

bool ZipVolume::makeDirectory(const Path& directory)
{
	return false;
}

bool ZipVolume::removeDirectory(const Path& directory)
{
	return false;
}

bool ZipVolume::renameDirectory(const Path& directory, const std::wstring& newName)
{
	return false;
}

bool ZipVolume::setCurrentDirectory(const Path& directory)
{
	if (directory.isRelative())
		m_currentDirectory = m_currentDirectory + directory;
	else
		m_currentDirectory = directory;
	return true;
}

Path ZipVolume::getCurrentDirectory() const
{
	return m_currentDirectory;
}

std::wstring ZipVolume::getSystemPath(const Path& path) const
{
	std::wstringstream ss;
	if (path.isRelative())
	{
		std::wstring tmp = m_currentDirectory.getPathNameNoVolume();
		ss << tmp << L"/" << path.getPathName();
	}
	else
		ss << path.getPathNameNoVolume();
	return ss.str();
}

	}
}
