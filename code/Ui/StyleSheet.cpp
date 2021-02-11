#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

struct Group
{
	std::wstring type;
	std::wstring element;
	Color4ub color;

	void serialize(ISerializer& s)
	{
		s >> Member< std::wstring >(L"type", type);
		s >> Member< std::wstring >(L"element", element);
		s >> Member< Color4ub >(L"color", color);
	}
};

struct Value
{
	std::wstring name;
	std::wstring value;

	void serialize(ISerializer& s)
	{
		s >> Member< std::wstring >(L"name", name);
		s >> Member< std::wstring >(L"value", value);
	}
};

class MemberEntity : public MemberComplex
{
public:
	MemberEntity(const wchar_t* const name, StyleSheet::Entity& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< std::wstring >(L"typeName", m_ref.typeName);
		s >> MemberSmallMap< std::wstring, Color4ub >(L"colors", m_ref.colors);
	}

private:
	StyleSheet::Entity& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ui.StyleSheet", 1, StyleSheet, ISerializable)

StyleSheet::Entity* StyleSheet::findEntity(const std::wstring& typeName)
{
	auto it = std::find_if(m_entities.begin(), m_entities.end(), [&](const Entity& entity) {
		return entity.typeName == typeName;
	});
	return it != m_entities.end() ? &(*it) : nullptr;
}

StyleSheet::Entity* StyleSheet::addEntity(const std::wstring& typeName)
{
	auto& entity = m_entities.push_back();
	entity.typeName = typeName;
	return &entity;
}

void StyleSheet::setColor(const std::wstring& typeName, const std::wstring& element, const Color4ub& color)
{
	auto it = std::find_if(m_entities.begin(), m_entities.end(), [&](const Entity& entity) {
		return entity.typeName == typeName;
	});
	if (it != m_entities.end())
		it->colors[element] = color;
	else
	{
		auto& entity = m_entities.push_back();
		entity.typeName = typeName;
		entity.colors[element] = color;
	}
}

Color4ub StyleSheet::getColor(const std::wstring& typeName, const std::wstring& element) const
{
	auto it = std::find_if(m_entities.begin(), m_entities.end(), [&](const Entity& entity) {
		return entity.typeName == typeName;
	});
	if (it == m_entities.end())
		return Color4ub(255, 255, 255);

	auto it2 = it->colors.find(element);
	if (it2 == it->colors.end())
		return Color4ub(255, 255, 255);

	return it2->second;
}

Color4ub StyleSheet::getColor(const Object* widget, const std::wstring& element) const
{
	const TypeInfo* widgetType = &type_of(widget);
	while (widgetType != nullptr)
	{
		auto it = std::find_if(m_entities.begin(), m_entities.end(), [&](const Entity& entity) {
			return entity.typeName == widgetType->getName();
		});
		if (it != m_entities.end())
		{
			auto it2 = it->colors.find(element);
			if (it2 != it->colors.end())
				return it2->second;
		}
		widgetType = widgetType->getSuper();
	}
	return Color4ub(255, 255, 255);
}

void StyleSheet::setValue(const std::wstring& name, const std::wstring& value)
{
	m_values[name] = value;
}

std::wstring StyleSheet::getValue(const std::wstring& name) const
{
	auto it = m_values.find(name);
	return it != m_values.end() ? it->second : L"";
}

Ref< StyleSheet > StyleSheet::merge(const StyleSheet* right) const
{
	Ref< StyleSheet > ss = new StyleSheet();

	ss->m_entities = m_entities;
	for (const auto& entity : right->m_entities)
	{
		for (auto it : entity.colors)
			ss->setColor(entity.typeName, it.first, it.second);
	}

	ss->m_values = m_values;
	for (auto it : right->m_values)
		ss->setValue(it.first, it.second);

	return ss;
}

void StyleSheet::serialize(ISerializer& s)
{
	if (s.getVersion< StyleSheet >() >= 1)
	{
		s >> MemberAlignedVector< Entity, MemberEntity >(L"entities", m_entities);
		s >> MemberSmallMap< std::wstring, std::wstring >(L"values", m_values);
	}
	else
	{
		AlignedVector< Group > groups;
		AlignedVector< Value > values;

		s >> MemberAlignedVector< Group, MemberComposite< Group > >(L"groups", groups);
		s >> MemberAlignedVector< Value, MemberComposite< Value > >(L"values", values);

		for (const auto& group : groups)
			setColor(group.type, group.element, group.color);

		for (const auto& value : values)
			setValue(value.name, value.value);
	}
}

Ref< StyleSheet > StyleSheet::createDefault()
{
	Ref< StyleSheet > ss = new StyleSheet();

	ss->setColor(L"traktor.ui.Widget", L"color", Color4ub(30, 30, 30));
	ss->setColor(L"traktor.ui.Widget", L"color-disabled", Color4ub(60, 60, 60));
	ss->setColor(L"traktor.ui.Widget", L"background-color", Color4ub(239, 239, 242));
	ss->setColor(L"traktor.ui.Widget", L"background-color-disabled", Color4ub(230, 230, 235));
	ss->setColor(L"traktor.ui.Widget", L"border-color", Color4ub(160, 160, 160));

	ss->setColor(L"traktor.ui.ListBox", L"background-color", Color4ub(255, 255, 255));

	ss->setColor(L"traktor.ui.Edit", L"background-color", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.Edit", L"background-color-disabled", Color4ub(239, 239, 242));
	ss->setColor(L"traktor.ui.Edit", L"border-color", Color4ub(180, 180, 180));
	ss->setColor(L"traktor.ui.Edit", L"border-color-disabled", Color4ub(180, 180, 180));

	ss->setColor(L"traktor.ui.Dock", L"caption-background-color", Color4ub(0, 122, 204));
	ss->setColor(L"traktor.ui.Dock", L"caption-color-focus", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.Dock", L"caption-color-no-focus", Color4ub(30, 30, 30));
	ss->setColor(L"traktor.ui.Dock", L"splitter-color", Color4ub(204, 200, 219));

	ss->setColor(L"traktor.ui.Tab", L"tab-background-color", Color4ub(0, 122, 204));
	ss->setColor(L"traktor.ui.Tab", L"tab-background-color-hover", Color4ub(28, 151, 234));
	ss->setColor(L"traktor.ui.Tab", L"tab-line-color", Color4ub(0, 122, 204));
	ss->setColor(L"traktor.ui.Tab", L"tab-color-active", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.Tab", L"tab-color-inactive", Color4ub(30, 30, 30));

	ss->setColor(L"traktor.ui.BackgroundWorkerDialog", L"border-color", Color4ub(0, 122, 204));

	ss->setColor(L"traktor.ui.ToolBar", L"item-background-color-pushed", Color4ub(0, 122, 204));
	ss->setColor(L"traktor.ui.ToolBar", L"item-background-color-hover", Color4ub(254, 254, 254));
	ss->setColor(L"traktor.ui.ToolBar", L"item-color-toggled", Color4ub(51, 153, 255));
	ss->setColor(L"traktor.ui.ToolBar", L"item-color-seperator", Color4ub(160, 160, 160));
	ss->setColor(L"traktor.ui.ToolBar", L"item-background-color-dropdown", Color4ub(239, 239, 242));
	ss->setColor(L"traktor.ui.ToolBar", L"item-background-color-dropdown-hover", Color4ub(254, 254, 254));
	ss->setColor(L"traktor.ui.ToolBar", L"item-color-dropdown-hover", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.ToolBar", L"item-background-color-dropdown-button", Color4ub(243, 243, 246));
	ss->setColor(L"traktor.ui.ToolBar", L"item-color-dropdown-arrow", Color4ub(20, 20, 20));

	ss->setColor(L"traktor.ui.TreeView", L"background-color", Color4ub(246, 246, 246));
	ss->setColor(L"traktor.ui.TreeViewItem", L"color-selected", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.TreeViewItem", L"background-color-selected", Color4ub(51, 153, 255));

	ss->setColor(L"traktor.ui.GridView", L"background-color", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.GridView", L"header-background-color", Color4ub(239, 239, 242));
	ss->setColor(L"traktor.ui.GridView", L"item-color-selected", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.GridView", L"item-background-color-selected", Color4ub(51, 153, 255));
	ss->setColor(L"traktor.ui.GridView", L"line-color", Color4ub(239, 239, 242));

	ss->setColor(L"traktor.ui.ScrollBar", L"background-color", Color4ub(219, 219, 222));
	ss->setColor(L"traktor.ui.ScrollBar", L"color-arrow", Color4ub(120, 120, 120));
	ss->setColor(L"traktor.ui.ScrollBar", L"color-slider", Color4ub(189, 189, 192));

	ss->setColor(L"traktor.ui.ListBox", L"background-color", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.ListBox", L"item-color-selected", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.ListBox", L"item-background-color-selected", Color4ub(51, 153, 255));

	ss->setColor(L"traktor.ui.LogList", L"background-color", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.LogList", L"background-color-info", Color4ub(255, 255, 255, 0));
	ss->setColor(L"traktor.ui.LogList", L"background-color-warning", Color4ub(255, 210, 87));
	ss->setColor(L"traktor.ui.LogList", L"background-color-error", Color4ub(255, 45, 45));
	ss->setColor(L"traktor.ui.LogList", L"color-info", Color4ub(0, 0, 0));
	ss->setColor(L"traktor.ui.LogList", L"color-warning", Color4ub(0, 0, 0));
	ss->setColor(L"traktor.ui.LogList", L"color-error", Color4ub(0, 0, 0));

	return ss;
}

	}
}
