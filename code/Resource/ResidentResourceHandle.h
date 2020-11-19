#pragma once

#include "Resource/ResourceHandle.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

/*! Cached resource handle.
 * \ingroup Resource
 *
 * Cached resource persist in the resource manager
 * thus are never reloaded unless explicitly flushed.
 */
class T_DLLCLASS ResidentResourceHandle : public ResourceHandle
{
	T_RTTI_CLASS;

public:
	explicit ResidentResourceHandle(const TypeInfo& type, bool persistent);

	const TypeInfo& getProductType() const { return m_resourceType; }

	bool isPersistent() const { return m_persistent; }

private:
	const TypeInfo& m_resourceType;
	bool m_persistent;
};

	}
}

