#include "Core/Misc/Split.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberEnum.h"
#include "Ui/Custom/PropertyList/ApplyReflector.h"
#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"
#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/CheckPropertyItem.h"
#include "Ui/Custom/PropertyList/ColorPropertyItem.h"
#include "Ui/Custom/PropertyList/FilePropertyItem.h"
#include "Ui/Custom/PropertyList/ListPropertyItem.h"
#include "Ui/Custom/PropertyList/NullPropertyItem.h"
#include "Ui/Custom/PropertyList/NumericPropertyItem.h"
#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "Ui/Custom/PropertyList/QuaternionPropertyItem.h"
#include "Ui/Custom/PropertyList/StaticPropertyItem.h"
#include "Ui/Custom/PropertyList/TextPropertyItem.h"
#include "Ui/Custom/PropertyList/VectorPropertyItem.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ApplyReflector", ApplyReflector, Serializer)

ApplyReflector::ApplyReflector(AutoPropertyList* propertyList)
:	m_propertyList(propertyList)
{
	m_propertyList->getPropertyItems(m_propertyItems, PropertyList::GfDescendants);
	m_propertyItemIterator = m_propertyItems.begin();
}

Serializer::Direction ApplyReflector::getDirection() const
{
	return Serializer::SdRead;
}

bool ApplyReflector::operator >> (const Member< bool >& m)
{
	CheckPropertyItem* propertyItem = checked_type_cast< CheckPropertyItem*, false >(*m_propertyItemIterator++);
	m = propertyItem->isChecked();
	return true;
}

bool ApplyReflector::operator >> (const Member< int8_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< int8_t >(propertyItem->getValue());
	return true;
}

bool ApplyReflector::operator >> (const Member< uint8_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< uint8_t >(propertyItem->getValue());
	return true;
}

bool ApplyReflector::operator >> (const Member< int16_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< int16_t >(propertyItem->getValue());
	return true;
}

bool ApplyReflector::operator >> (const Member< uint16_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< uint16_t >(propertyItem->getValue());
	return true;
}

bool ApplyReflector::operator >> (const Member< int32_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< int32_t >(propertyItem->getValue());
	return true;
}

bool ApplyReflector::operator >> (const Member< uint32_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< uint32_t >(propertyItem->getValue());
	return true;
}

bool ApplyReflector::operator >> (const Member< int64_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< int64_t >(propertyItem->getValue());
	return true;
}

bool ApplyReflector::operator >> (const Member< uint64_t >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< uint64_t >(propertyItem->getValue());
	return true;
}

bool ApplyReflector::operator >> (const Member< float >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< float >(propertyItem->getValue());
	return true;
}

bool ApplyReflector::operator >> (const Member< double >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = static_cast< double >(propertyItem->getValue());
	return true;
}

bool ApplyReflector::operator >> (const Member< std::string >& m)
{
	TextPropertyItem* propertyItem = checked_type_cast< TextPropertyItem*, false >(*m_propertyItemIterator++);
	m = wstombs(propertyItem->getValue());
	return true;
}

bool ApplyReflector::operator >> (const Member< std::wstring >& m)
{
	TextPropertyItem* propertyItem = checked_type_cast< TextPropertyItem*, false >(*m_propertyItemIterator++);
	m = propertyItem->getValue();
	return true;
}

bool ApplyReflector::operator >> (const Member< Guid >& m)
{
	BrowsePropertyItem* propertyItem = checked_type_cast< BrowsePropertyItem*, false >(*m_propertyItemIterator++);
	m = propertyItem->getValue();
	return true;
}

bool ApplyReflector::operator >> (const Member< Path >& m)
{
	FilePropertyItem* propertyItem = checked_type_cast< FilePropertyItem*, false >(*m_propertyItemIterator++);
	m = propertyItem->getPath();
	return true;
}

bool ApplyReflector::operator >> (const Member< Color4ub >& m)
{
	ColorPropertyItem* propertyItem = checked_type_cast< ColorPropertyItem*, false >(*m_propertyItemIterator++);
	m = propertyItem->getValue();
	return false;
}

bool ApplyReflector::operator >> (const Member< Scalar >& m)
{
	NumericPropertyItem* propertyItem = checked_type_cast< NumericPropertyItem*, false >(*m_propertyItemIterator++);
	m = Scalar(static_cast< float >(propertyItem->getValue()));
	return true;
}

bool ApplyReflector::operator >> (const Member< Vector2 >& m)
{
	VectorPropertyItem* propertyItem = checked_type_cast< VectorPropertyItem*, false >(*m_propertyItemIterator++);
	const VectorPropertyItem::vector_t& value = propertyItem->getValue();
	m = Vector2(value[0], value[1]);
	return true;
}

bool ApplyReflector::operator >> (const Member< Vector4 >& m)
{
	VectorPropertyItem* propertyItem = checked_type_cast< VectorPropertyItem*, false >(*m_propertyItemIterator++);
	const VectorPropertyItem::vector_t& value = propertyItem->getValue();
	m = Vector4(value[0], value[1], value[2], value[3]);
	return true;
}

bool ApplyReflector::operator >> (const Member< Matrix33 >& m)
{
	return true;
}

bool ApplyReflector::operator >> (const Member< Matrix44 >& m)
{
	return true;
}

bool ApplyReflector::operator >> (const Member< Quaternion >& m)
{
	QuaternionPropertyItem* propertyItem = checked_type_cast< QuaternionPropertyItem*, false >(*m_propertyItemIterator++);
	m = propertyItem->getValue();
	return true;
}

bool ApplyReflector::operator >> (const Member< ISerializable >& m)
{
	T_ASSERT (is_a< ObjectPropertyItem >(*m_propertyItemIterator));
	m_propertyItemIterator++;
	return m->serialize(*this);
}

bool ApplyReflector::operator >> (const Member< ISerializable* >& m)
{
	ObjectPropertyItem* propertyItem = checked_type_cast< ObjectPropertyItem*, false >(*m_propertyItemIterator++);
	Ref< ISerializable > object = checked_type_cast< ISerializable* >(propertyItem->getObject());
	if (object)
	{
		int version = type_of(object).getVersion();
		if (!serialize(object, version, 0))
			return false;
	}
	m = object;
	return true;
}

bool ApplyReflector::operator >> (const Member< void* >& m)
{
	return false;
}

bool ApplyReflector::operator >> (const MemberArray& m)
{
	ArrayPropertyItem* propertyItem = checked_type_cast< ArrayPropertyItem*, false >(*m_propertyItemIterator++);
	m.reserve(0, 0);
	while (m_propertyItemIterator != m_propertyItems.end() && (*m_propertyItemIterator)->getParentItem() == propertyItem)
	{
		if (!is_a< NullPropertyItem >(*m_propertyItemIterator))
			m.read(*this);
		else
		{
			m.insert();
			m_propertyItemIterator++;
		}
	}
	return true;
}

bool ApplyReflector::operator >> (const MemberComplex& m)
{
	if (m.getCompound())
		m_propertyItemIterator++;
	return m.serialize(*this);
}

bool ApplyReflector::operator >> (const MemberEnumBase& m)
{
	ListPropertyItem* propertyItem = checked_type_cast< ListPropertyItem*, false >(*m_propertyItemIterator++);
	m.set(propertyItem->getSelectedItem());
	return true;
}

		}
	}
}
