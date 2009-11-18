#ifndef traktor_db_DbmReadObject_H
#define traktor_db_DbmReadObject_H

#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Read instance object.
 * \ingroup Database
 */
class DbmReadObject : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmReadObject(uint32_t handle = 0);

	uint32_t getHandle() const { return m_handle; }

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_DbmReadObject_H