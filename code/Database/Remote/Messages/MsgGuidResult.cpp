#include "Database/Remote/Messages/MsgGuidResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgGuidResult", MsgGuidResult, IMessage)

MsgGuidResult::MsgGuidResult(const Guid& value)
:	m_value(value)
{
}

bool MsgGuidResult::serialize(ISerializer& s)
{
	return s >> Member< Guid >(L"value", m_value);
}

	}
}
