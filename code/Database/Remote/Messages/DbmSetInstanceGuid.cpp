#include "Database/Remote/Messages/DbmSetInstanceGuid.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmSetInstanceGuid", DbmSetInstanceGuid, IMessage)

DbmSetInstanceGuid::DbmSetInstanceGuid(uint32_t handle, const Guid& guid)
:	m_handle(handle)
,	m_guid(guid)
{
}

bool DbmSetInstanceGuid::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< Guid >(L"guid", m_guid);
	return true;
}

	}
}
