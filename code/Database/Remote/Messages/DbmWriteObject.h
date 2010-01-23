#ifndef traktor_db_DbmWriteObject_H
#define traktor_db_DbmWriteObject_H

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

/*! \brief Write instance object.
 * \ingroup Database
 */
class T_DLLCLASS DbmWriteObject : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmWriteObject(uint32_t handle = 0, const std::wstring& primaryTypeName = L"");

	uint32_t getHandle() const { return m_handle; }

	const std::wstring& getPrimaryTypeName() const { return m_primaryTypeName; }

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_handle;
	std::wstring m_primaryTypeName;
};

	}
}

#endif	// traktor_db_DbmWriteObject_H