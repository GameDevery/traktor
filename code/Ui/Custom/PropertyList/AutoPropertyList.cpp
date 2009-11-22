#include "Ui/Custom/PropertyList/AutoPropertyList.h"
#include "Ui/Custom/PropertyList/PropertyItem.h"
#include "Ui/Custom/PropertyList/InspectReflector.h"
#include "Ui/Custom/PropertyList/ApplyReflector.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

void removeAllChildren(PropertyList* list, PropertyItem* item)
{
	RefArray< PropertyItem > children = item->getChildItems();
	for (RefArray< PropertyItem >::iterator i = children.begin(); i != children.end(); ++i)
	{
		removeAllChildren(list, *i);
		list->removePropertyItem(item, *i);
	}
	T_ASSERT (item->getChildItems().empty());
}

			}
		
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.AutoPropertyList", AutoPropertyList, PropertyList)

bool AutoPropertyList::bind(ISerializable* object, ISerializable* outer)
{
	removeAllPropertyItems();
	if (!(m_object = object))
		return true;

	m_outer = outer;

	int32_t version = type_of(m_object).getVersion();

	InspectReflector reflector(this);
	if (!reflector.serialize(m_object, version, m_outer))
		return false;

	update();
	return true;
}

bool AutoPropertyList::refresh()
{
	removeAllPropertyItems();
	if (!m_object)
		return true;

	int32_t version = type_of(m_object).getVersion();

	InspectReflector reflector(this);
	if (!reflector.serialize(m_object, version, m_outer))
		return false;

	update();
	return true;
}

bool AutoPropertyList::refresh(PropertyItem* parent, ISerializable* object)
{
	removeAllChildren(this, parent);
	parent->collapse();

	if (object)
	{
		int32_t version = type_of(object).getVersion();

		InspectReflector reflector(this, parent);
		if (!reflector.serialize(object, version, m_outer))
			return false;
	}

	update();
	return true;
}

bool AutoPropertyList::apply()
{
	if (!m_object)
		return false;

	int32_t version = type_of(m_object).getVersion();

	ApplyReflector reflector(this);
	return reflector.serialize(
		m_object,
		version,
		m_outer
	);
}

bool AutoPropertyList::addObject(PropertyItem* parent, ISerializable* object)
{
	T_ASSERT_M (m_object, L"No object currently bound to property list");

	parent->collapse();

	InspectReflector reflector(this, parent);

	if (!(reflector >> Member< ISerializable* >(L"item", object)))
		return false;

	update();
	return true;
}

		}
	}
}
