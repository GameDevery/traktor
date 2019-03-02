#pragma once

#include <string>
#include "Database/Remote/IMessage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief Write instance data.
 * \ingroup Database
 */
class T_DLLCLASS DbmWriteData : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmWriteData(uint32_t handle = 0, const std::wstring& name = L"");

	uint32_t getHandle() const { return m_handle; }

	const std::wstring& getName() const { return m_name; }

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_handle;
	std::wstring m_name;
};

	}
}

