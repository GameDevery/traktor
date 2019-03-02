#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "SolutionBuilder/ProjectItem.h"

namespace traktor
{
	namespace sb
	{

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

void ProjectItem::serialize(ISerializer& s)
{
	s >> MemberRefArray< ProjectItem >(L"items", m_items);
}

	}
}
