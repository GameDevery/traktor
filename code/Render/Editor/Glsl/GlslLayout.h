#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{
	
class GlslResource;

class T_DLLCLASS GlslLayout : public Object
{
	T_RTTI_CLASS;

public:
	void add(GlslResource* resource);

	/*! Get resource at index. */
	GlslResource* get(int32_t index);

	/*! Get typed resource at index. */
	template < typename T >
	T* get(int32_t index) { return mandatory_non_null_type_cast< T* >(get(index)); }

	/*! Get resource by name. */
	const GlslResource* get(const std::wstring& name) const;

	/*! Get resource by name. */
	GlslResource* get(const std::wstring& name);

	/*! Get all resources. */
	const RefArray< GlslResource >& get() const { return m_resources; }

	/*! Get resources by type. */
	template < typename T >
	RefArray< T > get() const
	{
		RefArray< T > resources;
		for (auto resource : m_resources)
		{
			if (auto typed = dynamic_type_cast< T* >(resource))
				resources.push_back(typed);
		}
		return resources;
	}

	/*! Get all resources bound to a specific stage. */
	RefArray< GlslResource > get(uint8_t stageMask) const;

	/*! Get local index of resource. */
	int32_t getLocalIndex(const GlslResource* resource) const;

	/*! Get global index of resource. */
	int32_t getGlobalIndex(const GlslResource* resource) const;

	/*! Number of resources. */
	uint32_t count() const { return (uint32_t)m_resources.size(); }

	uint32_t count(const TypeInfo& resourceType, uint8_t stageMask = ~0) const;

	template < typename T >
	uint32_t count(uint8_t stageMask = ~0) const { return count(type_of< T >(), stageMask); }

private:
	RefArray< GlslResource > m_resources;
};

	}
}
