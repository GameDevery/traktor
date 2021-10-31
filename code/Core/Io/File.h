#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Date/DateTime.h"
#include "Core/Io/Path.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! System file descriptor.
 * \ingroup Core
 */
class T_DLLCLASS File : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! File flags. */
	enum Flags
	{
		FfInvalid = 0,
		FfNormal = 1,
		FfReadOnly = 2,
		FfHidden = 4,
		FfArchive = 8,
		FfDirectory	= 16,
		FfExecutable = 32
	};

	/*! File open modes. */
	enum Mode
	{
		FmRead = 1,
		FmWrite = 2,
		FmAppend = 4,
		FmMapped = 8
	};

	File() = default;

	explicit File(
		const Path& path,
		uint64_t size,
		uint32_t flags,
		const DateTime& creationTime,
		const DateTime& lastAccessTime,
		const DateTime& lastWriteTime
	);

	explicit File(
		const Path& path,
		uint64_t size,
		uint32_t flags
	);

	const Path& getPath() const;

	uint64_t getSize() const;

	uint32_t getFlags() const;

	bool isNormal() const;

	bool isReadOnly() const;

	bool isHidden() const;

	bool isArchive() const;

	bool isDirectory() const;

	bool isExecutable() const;

	const DateTime& getCreationTime() const;

	const DateTime& getLastAccessTime() const;

	const DateTime& getLastWriteTime() const;

	virtual void serialize(ISerializer& s) override final;

protected:
	Path m_path;
	uint64_t m_size = 0;
	uint32_t m_flags = FfInvalid;
	DateTime m_creationTime;
	DateTime m_lastAccessTime;
	DateTime m_lastWriteTime;
};

}

