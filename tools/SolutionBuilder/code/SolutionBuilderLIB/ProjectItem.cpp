#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/MemberRefArray.h>
#include "ProjectItem.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"ProjectItem", ProjectItem, ISerializable)

void ProjectItem::addItem(ProjectItem* item)
{
	m_items.push_back(item);
}

void ProjectItem::removeItem(ProjectItem* item)
{
	m_items.remove(item);
}

const RefArray< ProjectItem >& ProjectItem::getItems() const
{
	return m_items;
}

bool ProjectItem::serialize(ISerializer& s)
{
	return s >> MemberRefArray< ProjectItem >(L"items", m_items);
}
