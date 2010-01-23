#ifndef traktor_db_DbmCreateInstance_H
#define traktor_db_DbmCreateInstance_H

#include "Database/Remote/IMessage.h"
#include "Core/Guid.h"

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

/*! \brief Create instance.
 * \ingroup Database
 */
class T_DLLCLASS DbmCreateInstance : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmCreateInstance(uint32_t handle = 0, const std::wstring& name = L"", const Guid& guid = Guid());

	uint32_t getHandle() const { return m_handle; }

	const std::wstring& getName() const { return m_name; }

	const Guid& getGuid() const { return m_guid; }

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_handle;
	std::wstring m_name;
	Guid m_guid;
};

	}
}

#endif	// traktor_db_DbmCreateInstance_H
