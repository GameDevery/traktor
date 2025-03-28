/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Render/Editor/Glsl/GlslResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class T_DLLCLASS GlslLayout : public Object
{
	T_RTTI_CLASS;

public:
	void addStatic(GlslResource* resource, int32_t binding);

	void addBindless(GlslResource* resource);

	void add(GlslResource* resource);

	/*! Get resource at index. */
	GlslResource* getByIndex(int32_t index);

	/*! Get typed resource at index. */
	template < typename T >
	T* getByIndex(int32_t index) { return mandatory_non_null_type_cast< T* >(getByIndex(index)); }

	/*! Get resource by binding. */
	GlslResource* getByBinding(int32_t binding);

	/*! Get typed resource by binding. */
	template < typename T >
	T* getByBinding(int32_t binding) { return mandatory_non_null_type_cast< T* >(getByBinding(binding)); }

	/*! Get resource by name. */
	const GlslResource* getByName(const std::wstring& name) const;

	/*! Get resource by name. */
	template < typename T >
	T* getByName(const std::wstring& name) { return mandatory_non_null_type_cast<T*>(getByName(name)); }

	/*! Get resource by name. */
	GlslResource* getByName(const std::wstring& name);

	/*! Get all resources. */
	RefArray< GlslResource > getBySet(GlslResource::Set set);

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
	RefArray< GlslResource > getByStage(uint8_t stageMask) const;

	/*! Number of resources. */
	uint32_t count() const { return (uint32_t)m_resources.size(); }

	uint32_t count(const TypeInfo& resourceType, uint8_t stageMask = ~0) const;

	template < typename T >
	uint32_t count(uint8_t stageMask = ~0) const { return count(type_of< T >(), stageMask); }

private:
	RefArray< GlslResource > m_staticResources;
	RefArray< GlslResource > m_dynamicResources;
	RefArray< GlslResource > m_bindlessResources;
	RefArray< GlslResource > m_resources;

	void updateResourceLayout();
};

}
