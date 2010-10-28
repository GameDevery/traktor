#include "Online/AttachmentResult.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.AttachmentResult", AttachmentResult, Result)

void AttachmentResult::succeed(ISerializable* attachment)
{
	m_attachment = attachment;
	Result::succeed();
}

ISerializable* AttachmentResult::get() const
{
	return m_attachment;
}

	}
}
