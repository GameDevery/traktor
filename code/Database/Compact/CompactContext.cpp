#include "Database/Compact/CompactContext.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.CompactContext", CompactContext, Object)

CompactContext::CompactContext(BlockFile* blockFile, CompactRegistry* registry)
:	m_blockFile(blockFile)
,	m_registry(registry)
{
}

	}
}
