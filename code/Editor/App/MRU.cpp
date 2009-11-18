#include "Core/Io/FileSystem.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAggregate.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Misc/String.h"
#include "Editor/App/MRU.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

struct IgnoreCasePredicate
{
	std::wstring m_str;

	IgnoreCasePredicate(const std::wstring& str)
	:	m_str(str)
	{
	}

	bool operator () (const std::wstring& str) const
	{
		return compareIgnoreCase(str, m_str) == 0;
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.MRU", MRU, ISerializable)

void MRU::usedFile(const Path& filePath)
{
	// Always handle absolute paths.
	std::wstring fullPath = FileSystem::getInstance().getAbsolutePath(filePath).getPathName();

	// Remove existing entry; we will re-add below as most recent.
	std::vector< std::wstring >::iterator i = std::find_if(m_filePaths.begin(), m_filePaths.end(), IgnoreCasePredicate(fullPath));
	if (i != m_filePaths.end())
		m_filePaths.erase(i);

	m_filePaths.insert(m_filePaths.begin(), fullPath);
	if (m_filePaths.size() > 8)
		m_filePaths.pop_back();
}

bool MRU::getUsedFiles(std::vector< Path >& outFilePaths) const
{
	Path currentPath = FileSystem::getInstance().getAbsolutePath(L"");

	// Generate list of relative paths.
	for (std::vector< std::wstring >::const_iterator i = m_filePaths.begin(); i != m_filePaths.end(); ++i)
	{
		Path relativePath;
		if (FileSystem::getInstance().getRelativePath(
			*i,
			currentPath,
			relativePath
		))
			outFilePaths.push_back(relativePath);
	}

	return true;
}

bool MRU::serialize(ISerializer& s)
{
	return s >> MemberStlVector< std::wstring >(L"filePaths", m_filePaths);
}

	}
}
