/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Ui/HierarchicalState.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ui.HierarchicalState", 1, HierarchicalState, ISerializable)

void HierarchicalState::setScrollPosition(int32_t scrollPosition)
{
	m_scrollPosition = scrollPosition;
}

int32_t HierarchicalState::getScrollPosition() const
{
	return m_scrollPosition;
}

void HierarchicalState::setValue(uint32_t id, int32_t value)
{
	m_values[id] = value;
}

int32_t HierarchicalState::getValue(uint32_t id, int32_t defaultValue) const
{
	auto it = m_values.find(id);
	return it != m_values.end() ? it->second : defaultValue;
}

void HierarchicalState::addState(const std::wstring& path, bool expanded, bool selected)
{
	if (expanded || selected)
		m_states[path] = std::make_pair(expanded, selected);
	else
	{
		auto it = m_states.find(path);
		if (it != m_states.end())
			m_states.erase(it);
	}
}

bool HierarchicalState::getExpanded(const std::wstring& path) const
{
	auto it = m_states.find(path);
	return it != m_states.end() ? it->second.first : false;
}

bool HierarchicalState::getSelected(const std::wstring& path) const
{
	auto it = m_states.find(path);
	return it != m_states.end() ? it->second.second : false;
}

Ref< HierarchicalState > HierarchicalState::merge(const HierarchicalState* state) const
{
	Ref< HierarchicalState > merged = new HierarchicalState();
	merged->m_states = m_states;

	for (const auto& it : state->m_states)
		merged->addState(it.first, it.second.first, it.second.second);

	return merged;
}

void HierarchicalState::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"scrollPosition", m_scrollPosition);

	if (s.getVersion< HierarchicalState >() >= 1)
		s >> MemberSmallMap < uint32_t, int32_t >(L"values", m_values);

	s >> MemberSmallMap<
		std::wstring,
		std::pair< bool, bool >,
		Member< std::wstring >,
		MemberStlPair< bool, bool >
	>(L"states", m_states);
}

}
